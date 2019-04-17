adb root
adb remount
echo "any key pull"
pause;
adb pull /system/etc/hosts
echo "any key exit"
pause;