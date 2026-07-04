package com.solstice.fakeclient

import android.app.Activity
import android.content.Intent
import android.graphics.Color
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.provider.Settings
import android.view.Gravity
import android.view.ViewGroup
import android.widget.Button
import android.widget.LinearLayout
import android.widget.TextView

/**
 * Launcher screen. It only exists to (1) obtain the "draw over other apps"
 * permission and (2) start/stop the floating overlay. Once the overlay is
 * running everything happens through the floating icon, so this activity can be
 * closed.
 */
class MainActivity : Activity() {

    private lateinit var status: TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val root = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setBackgroundColor(Color.parseColor("#11131A"))
            setPadding(48, 96, 48, 48)
            gravity = Gravity.CENTER_HORIZONTAL
        }

        val title = TextView(this).apply {
            text = "FakeClient (Android overlay)"
            textSize = 22f
            setTextColor(Color.parseColor("#5B8CFF"))
        }
        status = TextView(this).apply {
            textSize = 14f
            setTextColor(Color.LTGRAY)
            setPadding(0, 32, 0, 32)
        }

        val grantBtn = Button(this).apply {
            text = "オーバーレイ権限を許可"
            setOnClickListener { requestOverlayPermission() }
        }
        val startBtn = Button(this).apply {
            text = "オーバーレイを開始"
            setOnClickListener { startOverlay() }
        }
        val stopBtn = Button(this).apply {
            text = "オーバーレイを停止"
            setOnClickListener { stopOverlay() }
        }
        val help = TextView(this).apply {
            text = "起動後、画面に丸いアイコンが出ます。\n" +
                   "・タップ: ClickGUIを開閉\n" +
                   "・ドラッグ: アイコン移動\n" +
                   "・長押し: 終了ボタン表示\n" +
                   "ClickGUI内: タップ=左クリック, 長押し=右クリック(設定展開),\n" +
                   "2本指タップ=中クリック(キーバインド開始)。\n" +
                   "USBキーボード/マウス接続時はWindows版と同様に操作できます。"
            textSize = 12f
            setTextColor(Color.GRAY)
            setPadding(0, 32, 0, 0)
        }

        val lp = LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT
        ).apply { topMargin = 16 }

        root.addView(title)
        root.addView(status)
        root.addView(grantBtn, lp)
        root.addView(startBtn, lp)
        root.addView(stopBtn, lp)
        root.addView(help)
        setContentView(root)
    }

    override fun onResume() {
        super.onResume()
        updateStatus()
    }

    private fun hasOverlayPermission(): Boolean =
        Build.VERSION.SDK_INT < Build.VERSION_CODES.M || Settings.canDrawOverlays(this)

    private fun updateStatus() {
        status.text = if (hasOverlayPermission())
            "権限OK。オーバーレイを開始できます。"
        else
            "「他のアプリの上に表示」権限が必要です。"
    }

    private fun requestOverlayPermission() {
        if (hasOverlayPermission()) { updateStatus(); return }
        val intent = Intent(
            Settings.ACTION_MANAGE_OVERLAY_PERMISSION,
            Uri.parse("package:$packageName")
        )
        startActivity(intent)
    }

    private fun startOverlay() {
        if (!hasOverlayPermission()) {
            requestOverlayPermission()
            return
        }
        val intent = Intent(this, OverlayService::class.java)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
            startForegroundService(intent)
        else
            startService(intent)
        moveTaskToBack(true)   // get out of the way; the overlay takes over
    }

    private fun stopOverlay() {
        stopService(Intent(this, OverlayService::class.java))
    }
}
