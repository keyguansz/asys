------------------------------【开发须知】---------------------------------------------
ZS600需求池:https://confluence.djicorp.com/pages/viewpage.action?pageId=10915976
交互设计：@tony.chen
UI设计:https://app.zeplin.io/project.html#pid=585b5961ccc11fa308e0b0a3&dashboard
       djisoftware@gmail.com/Dji123456
开发计划：https://confluence-rd.djicorp.com/pages/viewpage.action?pageId=32845650
分支说明：远端主分支develop，远端个人分支develop-key,develop-andy;
          本地开发---在本地从develop-{Name}拉取分支develop-{Name}-{Feat};
          提交代码到远端个人分支---develop-{Name}-{Feat}合并到develop-{Name}，提交develop-{Name}即可;
          提交代码到远端主分支——自测通过后，develop合并到develop-{Name}，解决冲突->自测通过后->develop-{Name} 合并到develop->提交;
log-path： adb pull /sdcard/DJI/dji.system.upgrade/LOG/CACHE/log-2013-02-06.txt
adb pull /sdcard/DJI/dji.go.v4/LOG/CRASH/
root@zs600b:/sdcard/DJI/dji.go.v4/LOG/CRASH # ls
crash-2017-03-27-11-57-54.txt

开发环境：小屏768dp*436dp；dumpsys power
需求要点(未完成):Notification ui(1d),国家码(setup设置[1d]，go更新[3d])，导航栏[2h，peaker]，.F1/F2（go不相关部分,4h，peacker），5.录屏浮窗(1d,预演)，电池详情（1d），Sre(2d,预演，peaker，可能干掉)，顶部栏屏蔽（4h，@zhongping），wifi（1d）
 翻译+文案+适配（3d，peacker）； setup也要适配大屏（+1d）
需求池：多任务（3d），F1/F2（go相关部分，预演，4d），设备识别（1d）；Wi-Fi免流量传视频照片到手机（预演，5d）
--------------------------------【发布日志】------------------------------------------

