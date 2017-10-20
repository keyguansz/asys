# 1.	Adb
Android Debug Bridge
## 1 1 PC如何连接Android手机设备
本质上是一个跨端通信问题，因此有串口通信(USB线)，网线，无线
### USB线
   用一个USB线连接PC（主设备）和Android（从设备），PC端会自动装驱动
   // TODO PC为何知道要去装驱动，这个驱动是从哪里获取，如果是网络，那么PC怎么知道去获取哪个链接对应的驱动
### 铜线网线
网络adb调试-有线
基本思想：Pc主机（P端）和android手机（A端）在同一局域网下。
#### 配置详解
1.	硬件环境：用usb转网口线，usb口接Android设备，网口接PC主机
2.	软件配置（A端）：adbd_tcp.sh &
3.	shell查询A端的busybox ifconfig，获得IP_A(192.168.1.210)和maskA(192.168.1.255)
4.	P端配置：本地连接->属性->Internet 协议版本4->配置Ip在同一有局域网（如：192.168.1.211）
5.	Pc发起连接：adb connect 192.168.1.210
#### 命令如下：
```java
// cmd 执行说明
C:\Users\key.guan>adb root
C:\Users\key.guan>adb shell
root@zs600b:/ # adbd_tcp.sh &
root@zs600b:/ # busybox ifconfig
  eth0    Link encap:Ethernet  HWaddr 00:E0:4C:68:02:27
          inet addr:192.168.1.210  Bcast:192.168.1.255  Mask:255.255.255.0
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:832 errors:0 dropped:0 overruns:0 frame:0
          TX packets:5 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
	        RX bytes:60146 (58.7 KiB)  TX bytes:1198 (1.1 KiB)
root@zs600b:/ # exit
拔掉usb线
C:\Users\key.guan>adb connect 192.168.1.210
connected to 192.168.1.210:5555
```
#### adbd_tcp完成Android配置
``` sh
#!/system/bin/sh
net_card=`busybox ifconfig | grep eth0`
net_config_adbd_restart()
{
	ifconfig eth0 192.168.1.210 && \
	ifconfig eth0 up && \
	setprop service.adb.tcp.port 5555 && \
	stop adbd && \
	start adbd
}

if [ $net_card=="" ]; then
	echo "Net card(eth0) has not found!!"
	exit 1
else
	net_config_adbd_restart
fi

```
#### QA
- Ifconfig不可以用
只能用busybox：（busybox1.11 ifconfig也可，单独的ifconfig被阉割了不能被系统识别）
- adbd_tcp.sh没有在后台执行
A端系统adb异常，插拔U线和网线都无法adb shell了：adbd_tcp.sh &漏了&,在执行sh的stop adbd的时候，退出控制台，导致没有执行到adbd start，系统的adbd就一直处于关闭状态
。只好重启
``` java
C:\Users\key.guan>adb connect 192.168.1.210
unable to connect to 192.168.1.210:5555: cannot connect to 192.168.1.210:5555:
由于目标计算机积极拒绝，无法连接。 (10061)
C:\Users\key.guan>adb shell
error: no devices/emulators found
```
- 网卡没有插好
``` java
root@zs600b:/ # adbd_tcp.sh
Net card(eth0) has not found!!
```
- 不是root权限
``` java
shell@zs600b:/ $ adbd_tcp.sh
/system/bin/adbd_tcp.sh[19]: [: Link: unexpected operator/operand
error: SIOCSIFADDR (Operation not permitted)
```
- 网络IP有误/配置不对
``` java
C:\Users\key.guan>adb connect 192.168.1.211
unable to connect to 192.168.1.211:5555: cannot connect to 192.168.1.211:5555:
由于目标计算机积极拒绝，无法连接。 (10061)
```
### 无线
从原理上来讲，处在同一个无线局域网内就可以了，因此有两种连接方式
-  PC无无线网卡：连接网线端一台无线路由器（热点为hotspot），Android连接hotspot
-  PC有无线网卡：一般笔记本都有/自己买一个随身wifi，Android开启热点，PC连接之
该方法优点简单无需过多硬件，缺点就是容易受外界无线干扰，网速慢。下面是23:00 工作环境实测效果
```java
// 无线热点方式 20cm speed = 50kb/s; 1cm 0.2MB/s
C:\Users\key.guan>adb push C:\Users\key.guan\Desktop\kggo.hprof.tar /sdcard/
C:\Users\key.guan\Desktop\kggo.hprof.tar: 1 file pushed. 0.0 MB/s (22210059 bytes in 439.870s)
C:\Users\key.guan>adb push C:\Users\key.guan\Desktop\kggo.hprof.tar /sdcard/tsts3
C:\Users\key.guan\Desktop\kggo.hprof.tar: 1 file pushed. 0.2 MB/s (22210059 bytes in 120.611s)
// 无线路由器方式 1.1 MB/s
C:\Users\key.guan>adb push C:\Users\key.guan\Desktop\kggo.hprof.tar /sdcard/tsts
C:\Users\key.guan\Desktop\kggo.hprof.tar: 1 file pushed. 1.1 MB/s (22210059 bytes in 19.918s)

// 有线方式 4.6 MB/s
C:\Users\key.guan>adb push C:\Users\key.guan\Desktop\kggo.hprof.tar /sdcard/tst
C:\Users\key.guan\Desktop\kggo.hprof.tar: 1 file pushed. 4.6 MB/s (22210059 bytes in 4.651s)
```

下面以PC无无线网卡为例,保证手机和PC在同一网段（推荐手机连接APP-OFFICE，PC是有线连接）
####　配置详解
1.	硬件环境：路由器，USB数据线，PC电脑，Android设备
2.	USB数据线连接后，cmd下 敲 adb tcpip 5555
3.	连接手机 adb connect ip (如 adb connect 10.81.1.123)
4.	拔掉USB线就好，adb shell
命令如下：
C:\Users\key.guan\AppData\Local\Android\sdk\platform-tools>adb connect 10.81.5.77
connected to 10.81.5.77:5555

### 常见问题
#### wifi的adb连接断开
手机与遥控器通过USB连接或断开的时候会导致adb连接失败,重新connect一下
#### 有多个USB链接设备
A:拔掉一些USB链接设备好了，或者指定-s sn某个设备
``` java
C:\Users\key.guan>adb shell
error: more than one device/emulator
C:\Users\key.guan>adb devices
List of devices attached
192.168.1.210:5555      device
2QNHFFIW4A      device
```
#### 掉线了
A：重连即可
C:\Users\key.guan>adb devices
List of devices attached
192.168.1.210:5555      offline




## 1.1.	常用操作
启动/关闭adb服务:adb start-server/kill-server
多设备（模拟器）指定设备号: adb devices -l; adb –s <serialNumber>
安装升级apk：adb install -r XX.apk；
安装降级apk：adb install -r –d XX.apk
卸载:adb uninstall <package>
卸载APK但keep数据和缓存文件:adb uninstall -k <package>

## 1.2.	文件处理
从手机复制文件出来：adb pull <remote> <local>
发送文件：adb push <local> <remote>
向手机发送文件eg. adb push foo.txt /sdcard/foo.txt
系统属性（/SYSTEM/BUILD.PROP）：adb shell getprop
日志重定向; adb logcat > bug1121.txt
CPU架构信息： /proc/cpuinfo
## 1.3.	Adb调试方法
