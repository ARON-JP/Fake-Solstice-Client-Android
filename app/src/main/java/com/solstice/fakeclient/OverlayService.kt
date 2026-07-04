package com.solstice.fakeclient

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Context
import android.content.Intent
import android.content.pm.ServiceInfo
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.PixelFormat
import android.os.Build
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.view.Choreographer
import android.view.Gravity
import android.view.InputDevice
import android.view.KeyEvent
import android.view.MotionEvent
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import android.view.WindowManager
import android.widget.TextView
import kotlin.math.hypot

/**
 * Hosts the floating round icon and the native ClickGUI as system overlays
 * (draw-over-other-apps). The icon is a tiny Kotlin-drawn window; the ClickGUI is
 * a fullscreen EGL/GLES SurfaceView rendered by libfakeclient.so. This mirrors
 * the Windows build's "separate always-interactive overlay window" model.
 */
class OverlayService : Service() {

    private lateinit var wm: WindowManager
    private val main = Handler(Looper.getMainLooper())

    private lateinit var icon: View
    private lateinit var iconParams: WindowManager.LayoutParams

    private var gui: GuiSurfaceView? = null
    private var exitBtn: View? = null

    private var nativeReady = false
    private var rendering = false

    private val overlayType =
        WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY  // API 26+

    // ----------------------------------------------------------------- lifecycle
    override fun onBind(intent: Intent?): IBinder? = null

    override fun onCreate() {
        super.onCreate()
        wm = getSystemService(Context.WINDOW_SERVICE) as WindowManager
        startAsForeground()

        NativeBridge.nativeInit(assets)
        nativeReady = true

        addIcon()
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int = START_STICKY

    override fun onDestroy() {
        stopRendering()
        removeExitButton()
        gui?.let { runCatching { wm.removeView(it) } }
        gui = null
        runCatching { wm.removeView(icon) }
        if (nativeReady) {
            NativeBridge.nativeShutdown()
            nativeReady = false
        }
        super.onDestroy()
    }

    // ----------------------------------------------------------------- foreground
    private fun startAsForeground() {
        val channelId = "fakeclient_overlay"
        val nm = getSystemService(NotificationManager::class.java)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val ch = NotificationChannel(
                channelId, getString(R.string.notif_channel),
                NotificationManager.IMPORTANCE_MIN
            )
            nm.createNotificationChannel(ch)
        }
        val notif: Notification = Notification.Builder(this, channelId)
            .setContentTitle("FakeClient")
            .setContentText("オーバーレイ実行中（アイコン長押しで終了）")
            .setSmallIcon(android.R.drawable.ic_menu_view)
            .setOngoing(true)
            .build()

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE) {
            startForeground(1, notif, ServiceInfo.FOREGROUND_SERVICE_TYPE_SPECIAL_USE)
        } else {
            startForeground(1, notif)
        }
    }

    // ----------------------------------------------------------------- icon
    private fun addIcon() {
        icon = FloatingIconView(this)
        iconParams = WindowManager.LayoutParams(
            WindowManager.LayoutParams.WRAP_CONTENT,
            WindowManager.LayoutParams.WRAP_CONTENT,
            overlayType,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE or
                WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
            PixelFormat.TRANSLUCENT
        ).apply {
            gravity = Gravity.TOP or Gravity.START
            x = 60
            y = 300
        }
        attachIconTouch()
        wm.addView(icon, iconParams)
    }

    private fun attachIconTouch() {
        val slop = (8 * resources.displayMetrics.density)
        var startX = 0f; var startY = 0f
        var startWinX = 0; var startWinY = 0
        var downTime = 0L
        var dragging = false
        var longPressFired = false

        val longPress = Runnable {
            longPressFired = true
            toggleExitButton()
        }

        icon.setOnTouchListener { _, e ->
            when (e.actionMasked) {
                MotionEvent.ACTION_DOWN -> {
                    startX = e.rawX; startY = e.rawY
                    startWinX = iconParams.x; startWinY = iconParams.y
                    downTime = System.currentTimeMillis()
                    dragging = false; longPressFired = false
                    main.postDelayed(longPress, 500)
                    true
                }
                MotionEvent.ACTION_MOVE -> {
                    val dx = e.rawX - startX; val dy = e.rawY - startY
                    if (!dragging && hypot(dx, dy) > slop) {
                        dragging = true
                        main.removeCallbacks(longPress)
                    }
                    if (dragging) {
                        iconParams.x = (startWinX + dx).toInt()
                        iconParams.y = (startWinY + dy).toInt()
                        runCatching { wm.updateViewLayout(icon, iconParams) }
                    }
                    true
                }
                MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                    main.removeCallbacks(longPress)
                    val dt = System.currentTimeMillis() - downTime
                    if (!dragging && !longPressFired && dt < 500) {
                        toggleGui()      // tap = open/close ClickGUI
                    }
                    true
                }
                else -> false
            }
        }
    }

    // ----------------------------------------------------------------- ClickGUI
    private fun toggleGui() {
        if (gui == null) showGui() else hideGui()
    }

    private fun showGui() {
        if (gui != null) return
        val v = GuiSurfaceView(this)
        val p = WindowManager.LayoutParams(
            WindowManager.LayoutParams.MATCH_PARENT,
            WindowManager.LayoutParams.MATCH_PARENT,
            overlayType,
            // focusable (no NOT_FOCUSABLE) so a hardware keyboard reaches the GUI.
            WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN or
                WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS or
                WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED,
            PixelFormat.TRANSLUCENT
        ).apply { gravity = Gravity.TOP or Gravity.START }

        gui = v
        wm.addView(v, p)
        v.requestFocus()
        NativeBridge.nativeToggleGui()       // enable ClickGui module
        // Keep the icon tappable above the GUI surface.
        runCatching { wm.removeView(icon); wm.addView(icon, iconParams) }
    }

    private fun hideGui() {
        val v = gui ?: return
        NativeBridge.nativeToggleGui()       // disable ClickGui module
        stopRendering()
        runCatching { wm.removeView(v) }
        gui = null
    }

    // ----------------------------------------------------------------- exit button
    private fun toggleExitButton() {
        if (exitBtn != null) { removeExitButton(); return }
        val btn = TextView(this).apply {
            text = "  ✕ 終了  "
            setTextColor(Color.WHITE)
            textSize = 16f
            setBackgroundColor(Color.parseColor("#CC202434"))
            setPadding(28, 20, 28, 20)
            setOnClickListener { exitOverlay() }
        }
        val p = WindowManager.LayoutParams(
            WindowManager.LayoutParams.WRAP_CONTENT,
            WindowManager.LayoutParams.WRAP_CONTENT,
            overlayType,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE or
                WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
            PixelFormat.TRANSLUCENT
        ).apply {
            gravity = Gravity.TOP or Gravity.START
            x = iconParams.x
            y = iconParams.y + 170
        }
        exitBtn = btn
        wm.addView(btn, p)
        // auto-dismiss after a few seconds if untouched
        main.postDelayed({ removeExitButton() }, 4000)
    }

    private fun removeExitButton() {
        exitBtn?.let { runCatching { wm.removeView(it) } }
        exitBtn = null
    }

    private fun exitOverlay() {
        stopSelf()
    }

    // ----------------------------------------------------------------- render loop
    private val frameCallback = object : Choreographer.FrameCallback {
        override fun doFrame(frameTimeNanos: Long) {
            if (!rendering) return
            NativeBridge.nativeRender()
            Choreographer.getInstance().postFrameCallback(this)
        }
    }

    private fun startRendering() {
        if (rendering) return
        rendering = true
        Choreographer.getInstance().postFrameCallback(frameCallback)
    }

    private fun stopRendering() {
        rendering = false
        Choreographer.getInstance().removeFrameCallback(frameCallback)
    }

    // ================================================================= views
    /** The floating round icon (ring + center dot), drawn directly to a Canvas. */
    private inner class FloatingIconView(ctx: Context) : View(ctx) {
        private val d = resources.displayMetrics.density
        private val sizePx = (64 * d).toInt()
        private val ring = Paint(Paint.ANTI_ALIAS_FLAG).apply {
            style = Paint.Style.STROKE; strokeWidth = 5 * d; color = Color.parseColor("#5B8CFF")
        }
        private val fill = Paint(Paint.ANTI_ALIAS_FLAG).apply {
            style = Paint.Style.FILL; color = Color.parseColor("#CC11131A")
        }
        private val dot = Paint(Paint.ANTI_ALIAS_FLAG).apply {
            style = Paint.Style.FILL; color = Color.parseColor("#5B8CFF")
        }

        override fun onMeasure(w: Int, h: Int) = setMeasuredDimension(sizePx, sizePx)

        override fun onDraw(c: Canvas) {
            val cx = width / 2f; val cy = height / 2f
            val r = width / 2f - 4 * d
            c.drawCircle(cx, cy, r, fill)
            c.drawCircle(cx, cy, r, ring)
            c.drawCircle(cx, cy, 6 * d, dot)
        }
    }

    /** Fullscreen SurfaceView that hosts the native ClickGUI and forwards input. */
    private inner class GuiSurfaceView(ctx: Context) : SurfaceView(ctx), SurfaceHolder.Callback {
        init {
            holder.addCallback(this)
            holder.setFormat(PixelFormat.TRANSLUCENT)
            setZOrderOnTop(true)               // composite over the app below
            isFocusable = true
            isFocusableInTouchMode = true
        }

        override fun surfaceCreated(h: SurfaceHolder) {
            NativeBridge.nativeSurfaceCreated(h.surface)
        }

        override fun surfaceChanged(h: SurfaceHolder, format: Int, width: Int, height: Int) {
            NativeBridge.nativeSurfaceChanged(width, height)
            startRendering()
        }

        override fun surfaceDestroyed(h: SurfaceHolder) {
            stopRendering()
            NativeBridge.nativeSurfaceDestroyed()
        }

        override fun onTouchEvent(e: MotionEvent): Boolean {
            // Hardware mouse arrives as a pointer too; let the mouse path below
            // own real mouse buttons and treat finger touches as gestures.
            if (e.isFromSource(InputDevice.SOURCE_MOUSE)) return onMouse(e)
            NativeBridge.nativeTouch(e.actionMasked, e.x, e.y, e.pointerCount)
            return true
        }

        override fun onGenericMotionEvent(e: MotionEvent): Boolean {
            if (e.isFromSource(InputDevice.SOURCE_MOUSE)) return onMouse(e)
            return super.onGenericMotionEvent(e)
        }

        private var lastButtons = 0
        private fun onMouse(e: MotionEvent): Boolean {
            NativeBridge.nativeMouseMove(e.x, e.y)
            when (e.actionMasked) {
                MotionEvent.ACTION_SCROLL -> {
                    val v = e.getAxisValue(MotionEvent.AXIS_VSCROLL)
                    val hh = e.getAxisValue(MotionEvent.AXIS_HSCROLL)
                    NativeBridge.nativeScroll(hh, v)
                }
                MotionEvent.ACTION_BUTTON_PRESS, MotionEvent.ACTION_DOWN,
                MotionEvent.ACTION_BUTTON_RELEASE, MotionEvent.ACTION_UP,
                MotionEvent.ACTION_MOVE -> {
                    val b = e.buttonState
                    syncButton(b, lastButtons, MotionEvent.BUTTON_PRIMARY, 0, e.x, e.y)
                    syncButton(b, lastButtons, MotionEvent.BUTTON_SECONDARY, 1, e.x, e.y)
                    syncButton(b, lastButtons, MotionEvent.BUTTON_TERTIARY, 2, e.x, e.y)
                    lastButtons = b
                }
            }
            return true
        }

        private fun syncButton(now: Int, prev: Int, mask: Int, imguiBtn: Int, x: Float, y: Float) {
            val isDown = now and mask != 0
            val wasDown = prev and mask != 0
            if (isDown != wasDown) NativeBridge.nativeMouseButton(imguiBtn, isDown, x, y)
        }

        override fun dispatchKeyEvent(e: KeyEvent): Boolean {
            if (e.keyCode == KeyEvent.KEYCODE_BACK) {
                if (e.action == KeyEvent.ACTION_UP) hideGui()   // back closes the GUI
                return true
            }
            val vk = KeyMap.toVk(e.keyCode)
            if (vk != 0) {
                when (e.action) {
                    KeyEvent.ACTION_DOWN -> NativeBridge.nativeKey(vk, true)
                    KeyEvent.ACTION_UP -> NativeBridge.nativeKey(vk, false)
                }
                return true
            }
            return super.dispatchKeyEvent(e)
        }
    }
}
