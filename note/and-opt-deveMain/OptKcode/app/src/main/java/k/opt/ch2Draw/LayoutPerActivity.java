package k.opt.ch2Draw;

import android.app.Activity;
import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.os.Bundle;
import android.widget.TextView;

import java.lang.ref.WeakReference;

import k.core.util.KLogUtil;
import k.opt.R;
import k.opt.monitor.memory.TestDataModel;

public class LayoutPerActivity extends Activity {
    private final String TAG  = "LayoutPerActivity";
    private NewHandler mHandler = new NewHandler(this);
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.getWindow().setBackgroundDrawable(null);
        setContentView(R.layout.activity_layout_per);
        //make a leak object
        TextView test = (TextView)this.findViewById(R.id.layout_per_txt_1);
        TestDataModel.getInstance().setRetainedTextView(test);
        TestDataModel.getInstance().setRetainedActiviy(this);
    }
   private static class NewHandler extends Handler{
       private WeakReference<Context> mContext = null;
       public NewHandler(Context ctx){
           mContext = new WeakReference<Context>(ctx);
       }
       @Override
       public void handleMessage(Message msg) {
           super.handleMessage(msg);
       }
   }
    private void doGetDataAsyncTask(){
        Message msg = Message.obtain();
        mHandler.sendMessage(msg);
    }

    @Override
    protected void onDestroy() {
        KLogUtil.D(TAG,"onDestroy");
        super.onDestroy();
        mHandler.removeCallbacksAndMessages(null);
    }
}
