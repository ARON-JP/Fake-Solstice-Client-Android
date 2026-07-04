# Keep JNI bridge method names (called from native by signature is fine, but the
# class/loadLibrary must survive).
-keep class com.solstice.fakeclient.NativeBridge { *; }
