```java
destApkDirectory = "lib/armeabi/";
if ((m_configType.equals("apk")) || (hasCaptiveRuntime()))
{//发行模式 || 带运行时
  destApkDirectory = "lib/armeabi-v7a/";
}
```
```bat
@REM 循环安装本目录下的APK文件
FOR %%i IN (*.apk) DO (
ECHO 正在安装：%%i
adb install -r %%i
)

```
