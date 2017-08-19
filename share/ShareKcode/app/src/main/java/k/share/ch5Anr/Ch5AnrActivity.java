package k.share.ch5Anr;

import android.app.Activity;
import android.os.Bundle;
import android.os.SystemClock;
import android.view.View;

import k.core.util.KLogUtil;
import k.share.R;

public class Ch5AnrActivity extends Activity {
    private final String TAG = "MainActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ch5_anr);
        findViewById(R.id.textView2).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                testAnr();
            }
        });
    }
//多线程同步，造成超时
    private void testAnr() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                initInChildThread();//2.
            }
        }).start();//1.
        KLogUtil.D(TAG,"1");
        SystemClock.sleep(10);//确保按照1.2.3执行
        initView();//3.
    }
    private synchronized void initView() {
        KLogUtil.D(TAG,"3");

    }
    private synchronized void initInChildThread() {
        KLogUtil.D(TAG,"2");
        SystemClock.sleep(30*1000);
    }
}
