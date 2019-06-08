package k.core.activity;

import android.app.Activity;
import android.os.Bundle;

public abstract class AbsActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        initData();
        findViews();//findViewById():
        initViews();//set text, color,viable,属性
        initListener();//eventbus,register brocast,listener，observe
    }

    protected abstract void initData();
}
