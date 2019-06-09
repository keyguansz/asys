adb shell atrace  -t=5000 -o /sdcard/atrace.txt sched gfx view wm
adb pull /sdcard/atrace.txt
pause