package com.example.mylove;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.support.v4.view.ViewPager.LayoutParams;
import android.view.Menu;
import android.view.View;
import android.view.animation.AnimationUtils;
import android.widget.ImageSwitcher;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.ViewSwitcher.ViewFactory;

@SuppressLint("HandlerLeak")
public class MainActivity extends Activity implements ViewFactory {
	private TextView mylove = null;
	public ImageSwitcher myloveimage = null;
	int[] pictrue;
	Myhandler myhandler = null;
	int i = 0;

	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		mylove = (TextView) findViewById(R.id.mylove);
		myloveimage = (ImageSwitcher) findViewById(R.id.myloveimage);
		pictrue = new int[] { R.drawable.a, R.drawable.b, R.drawable.c,
				R.drawable.d, R.drawable.e, R.drawable.f, R.drawable.g,
				R.drawable.h, R.drawable.i, R.drawable.g, R.drawable.k,
				R.drawable.l, R.drawable.m, R.drawable.n, R.drawable.o,
				R.drawable.p, R.drawable.q, R.drawable.r, R.drawable.s,
				R.drawable.t, R.drawable.u };
		mylove.setAnimation(AnimationUtils.loadAnimation(MainActivity.this, R.anim.move));
		myhandler = new Myhandler();
		myloveimage.setFactory(this);
		new Thread(new Mythread()).start();
	}

	class Myhandler extends Handler {
		@SuppressLint("HandlerLeak")
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			mylove.setVisibility(View.GONE);
			myloveimage.setAnimation(AnimationUtils.loadAnimation(
					MainActivity.this, R.anim.left));
			myloveimage
					.setImageDrawable(getResources().getDrawable(pictrue[i]));
			i++;
			if (i == pictrue.length) {
				i = 0;
			}

		}
	}

	class Mythread implements Runnable {
		public void run() {
			// TODO Auto-generated method stub
			while (true) {
				try {
					Thread.sleep(4000);
					myhandler.sendEmptyMessage(1);
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}

	@Override
	public View makeView() {
		// TODO Auto-generated method stub

		ImageView image = new ImageView(this);
		image.setMinimumHeight(200);
		image.setMinimumWidth(200);
		image.setScaleType(ImageView.ScaleType.FIT_CENTER);
		image.setLayoutParams(new ImageSwitcher.LayoutParams(
				LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT));
		return image;

	}
}
