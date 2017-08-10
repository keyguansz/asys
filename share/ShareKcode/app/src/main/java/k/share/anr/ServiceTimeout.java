package k.share.anr;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

/**
 *@desc   和os的通信方式原来是广播方式，发现效率不行，现在改用binder方式
 *@ref:
 *@author : key.guan @ 2017/5/18 11:04
 */
public class ServiceTimeout extends Service {
    private static final String TAG = "ServiceTimeout";//


    // 必须实现的方法，用户返回Binder对象
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        while (true) {

        }



    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        //DJILogUtil.R("ServiceTimeout--onStartCommand()--");//为何这个命令频繁执行？
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public boolean onUnbind(Intent intent) {
        return super.onUnbind(intent);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }
}
