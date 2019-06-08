package k.opt.activity;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Trace;
import android.util.Log;
import android.view.View;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import k.opt.R;
import k.opt.ch2Draw.LayoutPerActivity;
import k.opt.monitor.time.TimeMonitorConfig;
import k.opt.monitor.time.TimeMonitorManager;


public class MainActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //  Debug.startMethodTracing("kopt");
        try {
            Trace.beginSection("t1");
            setContentView(R.layout.activity_main);
        } finally {
            Trace.endSection();
        }
        // Debug.stopMethodTracing();
        testLeak();
        testAutoBoxing();
        testAnr();
        testAnrTime();
        testLock();
        //FileObserver
    }

    private void testAutoBoxing() {
        TimeMonitorManager.getInstance().getTimeMonitor(TimeMonitorConfig.TIME_MONITOR_ID_JAVA)
                .recodingTimeTag("test1 start");
        int test1 = 0;
        for (int i = 0; i < 1000 * 1000; i++) {
            test1 += i;
        }
        TimeMonitorManager.getInstance().getTimeMonitor(TimeMonitorConfig.TIME_MONITOR_ID_JAVA)
                .recodingTimeTag("test1 end,test2 start");
        Integer test2 = 0;
        for (int i = 0; i < 1000 * 1000; i++) {
            test2 += i;
        }
        TimeMonitorManager.getInstance().getTimeMonitor(TimeMonitorConfig.TIME_MONITOR_ID_JAVA)
                .recodingTimeTag("test2 end");
        TimeMonitorManager.getInstance().getTimeMonitor(TimeMonitorConfig.TIME_MONITOR_ID_JAVA)
                .end(false);
    }

    private void testLeak() {
        findViewById(R.id.test_leak).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity(new Intent(MainActivity.this, LayoutPerActivity.class));
                getLevel(0);
            }
        });
    }

    private void testAnr() {
        findViewById(R.id.test_anr).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    Thread.sleep(15 * 1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        });
    }
    private void testAnrTime() {
        findViewById(R.id.test_anr_time).setOnClickListener(new View.OnClickListener() {
            long cnt = 0;
            @Override
            public void onClick(View v) {
                while (true){//耗时操作
                    if (cnt%100000==0)
                    Log.e("testAnrTime","cnt="+cnt++);
                    ;
                }
            }
        });
    }

    private  Lock lock1 = new ReentrantLock();
    private  Lock lock2 = new ReentrantLock();
    private void testLock() {
        findViewById(R.id.test_lock).setOnClickListener(new View.OnClickListener() {
            Integer a = 0, b = 0, cnt = 0;


            void lock1() {
                cnt = 0;

                while (cnt < 10 * 1000) {
                    lock1.lock();
                    lock2.lock();
                    synchronized (b) {
                        try {
                            Thread.sleep(1000);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        synchronized (a) {
                            a = b + 1;
                        }
                    }
                    cnt++;
                    lock1.unlock();
                   lock2.unlock();
                    Log.e("test_lock", "lock1,cnt=" + cnt + " a=" + a + ",b=" + b);
                }
                Log.e("test_lock", "lock1,finish, a=" + a + ",b=" + b);


            }

            ;

            void lock2() {
                while (cnt < 10 * 1000) {
                    lock2.lock();
                    lock1.lock();
                    synchronized (a) {
                        synchronized (b) {
                            b = a + 1;
                        }
                    }
                    cnt++;
                   // lock1.unlock();
                    lock2.unlock();
                    Log.e("test_lock", "lock2,cnt=" + cnt + " a=" + a + ",b=" + b);
                }
                Log.e("test_lock", "lock2,finish, a=" + a + ",b=" + b);
            }
            void lock3() {
                while (cnt < 10 * 1000) {
                    lock2.lock();
                    lock1.lock();
                    synchronized (a) {
                        synchronized (b) {
                            b = a + 1;
                        }
                    }
                    cnt++;
                    // lock1.unlock();
                    lock2.unlock();
                    Log.e("test_lock", "lock2,cnt=" + cnt + " a=" + a + ",b=" + b);
                }
                Log.e("test_lock", "lock2,finish, a=" + a + ",b=" + b);
            }
            ;

            @Override
            public void onClick(View v) {

                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        lock1();
                    }

                }, "bg-Thread1").start();
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        lock2();
                    }

                }, "bg-Thread2").start();
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        lock3();
                    }

                }, "bg-Thread3").start();
                try {
                    Thread.sleep(2*1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                lock1.lock();
                lock2.lock();


            }
        });
    }


    public static final int UI_PERF_LEVEL_0 = 0;
    public static final int UI_PERF_LEVEL_1 = 2;

    //@IntDef({UI_PERF_LEVEL_0,UI_PERF_LEVEL_1})
    @Retention(RetentionPolicy.SOURCE)
    public @interface PERF_LEVEL {
    }

    public int getLevel(@PERF_LEVEL int level) {
        switch (level) {
            case UI_PERF_LEVEL_0:
                return 0;
            case UI_PERF_LEVEL_1:
                return 2;
            default:
                // throw new IllegalAccessException("unkwon");
                return 3;
        }
    }

    public enum UI_PERF {
        LEVEL_0,
        LEVEL_1
    }

    public int getLevel(UI_PERF level) {
        switch (level) {
            case LEVEL_0:
                return 0;
            case LEVEL_1:
                return 2;
            default:
                //throw new IllegalAccessException("unkwon");
                return -1;
        }
    }

}
