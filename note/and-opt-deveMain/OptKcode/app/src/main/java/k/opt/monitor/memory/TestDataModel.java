package k.opt.monitor.memory;

import android.app.Activity;
import android.widget.TextView;

import k.opt.ch2Draw.LayoutPerActivity;

public class TestDataModel {

    private static TestDataModel sInstance;
    private TextView mRetainedTextView;

    public static TestDataModel getInstance() {
        if (sInstance == null) {
            sInstance = new TestDataModel();
        }
        return sInstance;
    }

    public void setRetainedTextView(TextView textView) {
        mRetainedTextView = textView;
    }
Activity retainedActiviy;
    public void setRetainedActiviy(LayoutPerActivity retainedActiviy) {
        this.retainedActiviy = retainedActiviy;
    }
}