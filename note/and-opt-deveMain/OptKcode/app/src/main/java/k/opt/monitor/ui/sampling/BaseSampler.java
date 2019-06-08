package k.opt.monitor.ui.sampling;

import android.os.Handler;
import android.os.HandlerThread;

import java.util.concurrent.atomic.AtomicBoolean;

import k.core.util.KLogUtil;

/**
 * Created by yuchengluo on 2016/4/1.
 */
public abstract class BaseSampler {
    private final String TAG = "BaseSampler";
    private Handler mControlHandler = null;
    private int intervalTime = 50; //ms�������
    private AtomicBoolean mIsSampling = new AtomicBoolean(false);
    public BaseSampler(){
       KLogUtil.D(TAG,"Init BaseSampler");
    }
    public void start(){
        if(!mIsSampling.get()) {
           KLogUtil.D(TAG,"start Sampler");
            getmControlHandler().removeCallbacks(mRunnable);
            getmControlHandler().post(mRunnable);
            mIsSampling.set(true);
        }
    }
    public void stop(){
        if(mIsSampling.get()){
           KLogUtil.D(TAG,"stop Sampler");
            getmControlHandler().removeCallbacks(mRunnable);
            mIsSampling.set(false);
        }
    }
    private Handler getmControlHandler(){
        if(null == mControlHandler){
            HandlerThread mHT = new HandlerThread("SamplerThread");
            mHT.start();
            mControlHandler = new Handler(mHT.getLooper());
        }
        return mControlHandler;
    }
    abstract void doSample();
    private Runnable mRunnable = new Runnable() {
        @Override
        public void run() {
            doSample();
            if(mIsSampling.get()){
                getmControlHandler().postDelayed(mRunnable,intervalTime);
            }
        }
    };
}
