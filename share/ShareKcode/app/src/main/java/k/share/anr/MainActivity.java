package k.share.anr;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.view.View;
import android.widget.TextView;

import k.share.R;

public class MainActivity extends Activity {
    private final String TAG = "MainActivity";
    private TextView mWhileView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_anrs);
        registerReceiver(mTestReceiver, mFilter);
        mWhileView = (TextView) findViewById(R.id.tv_while);
        mWhileView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new MyCountDownTimer().start();
                while (true) {

                }
            }
        });
        findViewById(R.id.tv_broadcast).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new MyCountDownTimer().start();
                Intent intent = new Intent(TestAction);
                intent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);
                sendBroadcast(intent);
            }
        });
        findViewById(R.id.tv_ServiceTimeout).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new MyCountDownTimer().start();
                MainActivity.this.startService(new Intent(MainActivity.this, ServiceTimeout.class));
            }
        });
    }


    public class MyCountDownTimer extends CountDownTimer {
        public MyCountDownTimer() {
            super(60 * 1000L, 1000L);
        }

        @Override
        public void onTick(long millisUntilFinished) {
            long time = millisUntilFinished / 1000;
            if (time <= 59) {
                mWhileView.setText(String.format("倒计时开始  00:%02d", time));
            } else {
                mWhileView.setText(String.format("倒计时开始  %02d:%02d", time / 60, time % 60));
            }
        }

        @Override
        public void onFinish() {
            mWhileView.setText("倒计时结束  00:00");
        }
    }

    public final static String TestAction = "android.intent.action.test.share";
    private final IntentFilter mFilter = new IntentFilter(TestAction);
    public BroadcastReceiver mTestReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(TestAction)) {
                while (true) {
                    ;
                }
            }
        }
    };

}
