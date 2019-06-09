adb root
adb remount
adb push Z:\android-9.0.0_r35\out\target\product\marlin\system\lib\libgui.so /system/lib/
adb push Z:\android-9.0.0_r35\out\target\product\marlin\system\lib64\libgui.so /system/lib64/
adb reboot