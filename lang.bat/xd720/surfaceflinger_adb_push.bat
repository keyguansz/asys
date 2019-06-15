adb root
adb remount
adb push Z:\android-9.0.0_r35\out\target\product\marlin\system\lib\libsurfaceflinger.so /system/lib/
adb push Z:\android-9.0.0_r35\out\target\product\marlin\system\lib64\libsurfaceflinger.so /system/lib64/

adb push Z:\android-9.0.0_r35\out\target\product\marlin\system\bin\surfaceflinger /system/bin/
adb reboot