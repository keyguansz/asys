package k.opt.ch2Draw;

import android.app.Activity;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewStub;
import android.widget.CompoundButton;
import android.widget.ToggleButton;

import k.opt.R;

public class Ch2StubActivity extends Activity {

    private ViewStub viewStub;
    private ViewStub viewstub_image;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ch2_stub);
        ((ToggleButton)findViewById(R.id.toggleButton)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked){
                   // viewStub.inflate();
                    viewStub.setVisibility(View.VISIBLE);
                    viewstub_image.setVisibility(View.VISIBLE);
                }else {
                    viewStub.setVisibility(View.INVISIBLE);
                    viewstub_image.setVisibility(View.INVISIBLE);
                }
            }
        });
        viewStub = ((ViewStub) findViewById(R.id.viewstub_text));
        viewstub_image = ((ViewStub) findViewById(R.id.viewstub_image));

    }



}
