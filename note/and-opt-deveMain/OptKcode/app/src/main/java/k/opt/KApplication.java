package k.opt;


import android.content.Context;
import android.content.res.Configuration;
import android.support.multidex.MultiDexApplication;

import com.squareup.leakcanary.AndroidExcludedRefs;
import com.squareup.leakcanary.LeakCanary;
import com.squareup.leakcanary.RefWatcher;

import k.opt.monitor.memory.LeakCanaryService;
import k.opt.monitor.time.TimeMonitorConfig;
import k.opt.monitor.time.TimeMonitorManager;

/**
 * Created by yuchengluo on 2015/6/25.
 */
public class KApplication extends MultiDexApplication {

    private static Context mContext = null;
    private static RefWatcher mRefWatcher = null;

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        mContext = this;

    }

    public static Context getmContext() {
        return mContext;
    }

    /**
     * @desc: KApplication.getRefWatcher().watch(this);
     * @ref: 另一个需要监控的重要对象就是Fragment实例或者其他自定义的UI容器窗口组件，因为它和Activity实例一样，
     * 可能持有大量的视图以及视图需要的资源（如Bitmap），需要监控这一类组件，可以在Fragment onDestroy方法中，
     * 或者自定义组件的周期结束回调接口加入如下实现：
     * @author: key.guan @ 2017/6/4
     */
    public static RefWatcher getRefWatcher(){
        return mRefWatcher;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        mRefWatcher  = LeakCanary.install(this, LeakCanaryService.class, AndroidExcludedRefs.createAppDefaults().build());
        InitModule();
        TimeMonitorManager.getInstance().getTimeMonitor(TimeMonitorConfig.TIME_MONITOR_ID_APPLICATION_START)
                .recodingTimeTag("ApplicationCreate");
    }

    @Override
    public void onTerminate() {
        super.onTerminate();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    @Override
    public void onLowMemory() {
        super.onLowMemory();
    }

    @Override
    public void onTrimMemory(int level) {
        super.onTrimMemory(level);
    }

    public static Context getContext() {
        return mContext;
    }

    private void InitModule() {
      /*  DBManager.InitDB(mContext);
        CrashHandler crashHandler = new CrashHandler();
        crashHandler.init(this);*/
    }
}
