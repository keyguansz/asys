# 内存分析实战
## 1.测试环境部署
// TODO 如何配置好的adb环境，调试SC端用有线，非调试端用无线？
Android是root环境，选中自己要调试的进程,
有线：USB线或者网线环境，5s内就可以抓好
无线：热点/WiFi方式抓取很慢，几乎用不了
##  2.抓取hprof
### 命令抓取
[可以通过命令抓取](https://gist.github.com/logcat/8aeca0ee81af6fb0dc10bb0d58940007)
``` shell
// TODO 优化这段代码
// heap_dump.sh
heap_dump_location='/data/local/tmp/tmp.hprof'
dump_heap() {
  adb shell rm $heap_dump_location
  pid=`adb shell ps | grep 'com.example.packagename' | grep -v 'packagename\.' | cut -c10-15`
  adb shell am dumpheap $pid $heap_dump_location
  echo "Heap dump started, we have no idea when it's done, so take a look at logs, and when is done use pull_heap_dump"
}

pull_heap_dump() {
  adb pull $heap_dump_location ~/Desktop/$1
}
```
### UI
// TODO AS下比命令行抓取的数据更全？为啥？
// TODO
AS-> android Monitor-> Monitors-> Memory: dump java heap
##  3.其他操作
利用AS自带工具，直接拖hprof格式的文件到AS中就能开始分析了。
但是这样并不全，还是利用mat工具吧
- 格式转化
 AS的hprof格式转化为 MemoryAnalyzer的hprof
``` java
C:\Users\key.guan>C:\Users\key.guan\AppData\Local\Android\sdk\platform-tools\hprof-conv.exe C:\Users\key.guan\Desktop\dg.hprof new.hprof
ERROR: expecting HPROF file format 1.0.3
// RM 此处是因为压缩解压后数据有异常，就会提示HPROF异常提示
C:\Users\key.guan>C:\Users\key.guan\AppData\Local\Android\sdk\platform-tools\hprof-conv.exe C:\Users\key.guan\djigo.hprof new.hprof
ERROR: read 2595 of 4148 bytes
```
- MemoryAnalyzer分析hprof
- 抓取过程中ui卡死

## 抓取指标
- FPS：[1](https://github.com/wasabeef/Takt),[2这个可以直接在代码里面用](https://github.com/friendlyrobotnyc/TinyDancer)
##　信息备忘
adb connect 10.81.11.87（xandy）
// TODO 多媒体这部分，回到，每帧渲染关系，时间，解决闪屏问题
// 分析一下Launcher，upgrade，setup的性能问题
