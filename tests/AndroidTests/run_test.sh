framework=/system/framework
bpath=$framework/core.jar:$framework/ext.jar:$framework/framework.jar:$framework/android.test.runner.jar
adb shell exec dalvikvm  -Xbootclasspath:$bpath -cp /system/app/AndroidTests.apk:/data/app/com.android.unit_tests.apk \
      com.android.internal.util.WithFramework junit.textui.TestRunner $*
