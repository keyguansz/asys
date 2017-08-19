package k.demo.asys.activity;

import android.app.Activity;
import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.SystemClock;

import k.demo.asys.R;


public class DemoActivity extends Activity {


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_demo);
        PackageManager pm = getPackageManager();
        ComponentName cn = new ComponentName(this, DemoActivity.class);
     //   pm.setComponentEnabledSetting(cn, PackageManager.COMPONENT_ENABLED_STATE_DISABLED, PackageManager.DONT_KILL_APP);

        Long time = System.currentTimeMillis()+100*1000;
      //  alarmMgr.setTime(time);//<uses-permission android:name="android.permission.SET_TIME"/>!just for sys-app!
        AlarmManager alarmMgr = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
        Intent intent = new
                Intent("android.intent.action.POWER_USAGE_SUMMARY");
        alarmMgr.set(AlarmManager.ELAPSED_REALTIME
                , SystemClock.elapsedRealtime() + 5 * 1000
                , PendingIntent.getActivity(getApplicationContext(),0, intent, PendingIntent.FLAG_UPDATE_CURRENT));
        alarmMgr.getNextAlarmClock();
        AlarmManagerService
    }
}
