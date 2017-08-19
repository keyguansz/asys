package com.wbw.view;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Random;
import java.util.concurrent.CountDownLatch;

import com.wbw.iloveyou.R;
import com.wbw.inter.AllSurfaceView;
import com.wbw.util.BitmapCache;
import com.wbw.util.Font16;
import com.wbw.util.Font24;
import com.wbw.util.Font32;
import com.wbw.util.SharedPreferencesXml;
import com.wbw.util.Util;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.graphics.PorterDuff.Mode;
import android.os.Handler;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class FirstSurfaceView extends SurfaceView implements
		SurfaceHolder.Callback,AllSurfaceView {
	private volatile boolean isallstop = false;
	// 鍥剧墖杞紩鐢�
	BitmapCache bitmapcache;
	int w;
	int h;
	// 鐢荤埍蹇冪殑铦磋澏
	int[] heart_all = { R.drawable.a1, R.drawable.a2, R.drawable.a3,
			R.drawable.a4, R.drawable.a5, R.drawable.a6, R.drawable.a7,
			R.drawable.a8, R.drawable.a9, R.drawable.a10, R.drawable.a11,
			R.drawable.a12, R.drawable.a13, R.drawable.a14, R.drawable.a15,
			R.drawable.a16, R.drawable.a17, R.drawable.a18, R.drawable.a19 };
	private SurfaceHolder holder;
	private Context mContext;
	ShowBackgroundThread sbthread;
	int[] xin_all = { R.drawable.xin1, R.drawable.xin2, R.drawable.xin3,
			R.drawable.xin4 };

	private Handler handler;

	private Font32 font32;
	
	private SharedPreferencesXml spxml;
	public FirstSurfaceView(Context context, int s_w, int s_h, Handler handler) {
		super(context);
		// TODO 鑷姩鐢熸垚鐨勬瀯閫犲嚱鏁板瓨鏍�
		this.setFocusable(true);
		this.setKeepScreenOn(true);
		this.w = s_w;
		this.h = s_h;
		this.mContext = context;
		this.bitmapcache = BitmapCache.getInstance();
		this.holder = getHolder();
		this.holder.addCallback(this);
		this.handler = handler;
		font32 = new Font32(context);
		//閫忔槑
		setZOrderOnTop(true);
		holder.setFormat(PixelFormat.TRANSPARENT); 
		goOn();
		spxml = SharedPreferencesXml.init();
		
	}

	private Thread goonthread ;
	private Thread shthread;
	private Thread swzthread;
	private Thread sxthread;
	private Thread sbkup,sbkdown,sbkmiddleft,sbkmiddright;
	public void setRun(boolean is){
		isallstop = is;
		if(isallstop){
			goonthread.interrupt();
			if(shthread != null && shthread.isAlive())
				shthread.interrupt();
			if(swzthread != null && swzthread.isAlive())
				swzthread.interrupt();
			if(sxthread != null && sxthread.isAlive())
				sxthread.interrupt();
			if(sbkup != null && sbkup.isAlive())
				sbkup.interrupt();
			if(sbkdown != null && sbkdown.isAlive())
				sbkdown.interrupt();
			if(sbkmiddleft != null && sbkmiddleft.isAlive())
				sbkmiddleft.interrupt();
			if(sbkmiddright != null && sbkmiddright.isAlive())
				sbkmiddright.interrupt();
		}
	}
	
	// 璁℃暟鍣紝涓変釜绾跨▼鍏ㄩ儴杩愯瀹屾椂鑷姩璺冲埌鏂扮晫闈�
	final CountDownLatch begin = new CountDownLatch(7);

	
	public void goOn() {
		
		goonthread = new Thread(new Runnable() {

			@Override
			public void run() {
				// TODO 鑷姩鐢熸垚鐨勬柟娉曞瓨鏍�
				try {
					begin.await();
					Thread.sleep(500);
					handler.sendEmptyMessage(4);
				} catch (InterruptedException e) {
					// TODO 鑷姩鐢熸垚鐨� catch 鍧�
					e.printStackTrace();
				}
			}
		});
		goonthread.start();
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		// TODO 鑷姩鐢熸垚鐨勬柟娉曞瓨鏍�
		System.out.println("create");
		showB(); 
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		// TODO 鑷姩鐢熸垚鐨勬柟娉曞瓨鏍�

	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		// TODO 鑷姩鐢熸垚鐨勬柟娉曞瓨鏍�
	}

	public void showBackground() // 鍙槸鐢ㄦ潵鍒濆鍖栵紝鏃犲澶ф剰涔�
	{
		this.holder = getHolder();
		this.holder.addCallback(this);
	}
	
	public void showHeart() // 鐢诲績
	{
		shthread = new showheart(this.holder, "showback", this.w, this.h);
		shthread.start();
	}

	public void showWenzi() // 鐢绘枃瀛�
	{
		swzthread = new showWenZi(this.holder, "showwenzi", "", this.w, this.h);
		swzthread.start();
	}

	public void showXin() // 鐢绘槦鏄�
	{
		sxthread = new ShowBackgroundThread(this.holder, "showback", this.w, this.h);
		sxthread.start();
	}
	public void showB(){
		sbkup = new ShowBianKuang(R.drawable.upmidd, w, h, 1, 3);
		sbkdown = new ShowBianKuang(R.drawable.downmidd, w, h, 2, 3);
		sbkmiddleft = new ShowBianKuang(R.drawable.midd_midd_left, w, h, 3, 3);
		sbkmiddright = new ShowBianKuang(R.drawable.midd_midd_right, w, h, 4, 3);
		sbkup.start();
		sbkdown.start();
		sbkmiddleft.start();
		sbkmiddright.start();
	}

	public void show_font32(String s, int stx, int sty, int w, int h,
			int beishu, int type) {
		boolean[][] arr = new boolean[32][32]; // 鎻掑叆鐨勬暟缁�
		arr = font32.drawString(s);
		int startx = stx, starty = sty;
		int weith = 32;
		int height = 32;
		int bei = beishu;
		int old_num = -1;
		int lCount;// 鎺у埗鍒�
		for (int i = 0; i < 32 && !isallstop; i++) {
			for (int j = 0; j < 32 && !isallstop; j++) {
				try {
					Thread.sleep(25);
				} catch (InterruptedException e1) {
					// TODO 鑷姩鐢熸垚鐨� catch 鍧�
					e1.printStackTrace();
				}
				float xx = (float) j;
				float yy = (float) i;
				if (arr[i][j] && !isallstop) {

					Random rm = new Random();
					Bitmap bitmap = null;
					int num = 0;
					if (type == 1) {
						num = rm.nextInt(heart_all.length - 1);
						bitmap = bitmapcache
								.getBitmap(heart_all[num], mContext);
					} else if (type == 2) {
						bitmap = bitmapcache.getBitmap(R.drawable.love,
								mContext);
					}
					int bw = bitmap.getWidth();
					int bh = bitmap.getHeight();
					synchronized (holder) {
						Canvas c = null;
						try {

							// 涓嶈杞绘槗鍘婚攣瀹氭暣涓睆骞�
							c = holder.lockCanvas(new Rect(startx + (int) xx
									* bei, starty + (int) yy * bei, startx
									+ (int) xx * bei + bw, starty + (int) yy
									* bei + bh));

							// c = holder.lockCanvas();
							Paint p = new Paint(); // 鍒涘缓鐢荤瑪
							p.setColor(Color.RED);
							// 涓嬮潰杩欐鏄繚璇佸弻缂撳啿鑳介兘鐢讳笂涓滆タ锛屼粠鑰屼笉浼氶棯鐑�

							c.drawBitmap(bitmap, startx + xx * bei, starty + yy
									* bei, p);

							old_num = num;
						} catch (Exception e) {
							e.printStackTrace();
						} finally {
							try{
								if (c != null){
									holder.unlockCanvasAndPost(c);// 缁撴潫閿佸畾鐢诲浘锛屽苟鎻愪氦鏀瑰彉銆�
								}
							}catch(Exception e){
								e.printStackTrace();
							}
						}
					}
				}
			}

		}

	}
	
	/**
	 * 16/24/32,瑕佸啓鐨勫瓧绗︼紝寮�濮嬬殑x,y,鍊嶆暟锛岃姳鎴栫埍蹇�
	 * @param font_kind
	 * @param s
	 * @param stx
	 * @param sty
	 * @param beishu
	 * @param type
	 */
	public void show_font16(int font_kind,String s, int stx, int sty, 
			int beishu, int type) {
		boolean[][] arr = null;
		int weith = 16;
		int height = 16;
		if(font_kind == 16){
			weith = 16;
			height = 16;
			arr = new boolean[weith][height];
			Font16 font16 = new Font16(mContext);
			arr = font16.drawString(s);
		}else if(font_kind == 24){
			weith = 24;
			height = 24;
			arr = new boolean[weith][height];
			Font24 font24 = new Font24(mContext);
			arr = font24.drawString(s);
		}else {
			weith = 32;
			height = 32;
			arr = new boolean[weith][height];
			Font32 font32 = new Font32(mContext);
			arr = font32.drawString(s);
		}
				
		int startx = stx, starty = sty;		
		int bei = beishu;
		int old_num = -1;
		int lCount;// 鎺у埗鍒�
		for (int i = 0; i < weith && !isallstop; i++) {
			for (int j = 0; j < height && !isallstop; j++) {
				try {
					Thread.sleep(25);
				} catch (InterruptedException e1) {
					// TODO 鑷姩鐢熸垚鐨� catch 鍧�
					e1.printStackTrace();
				}
				float xx = (float) j;
				float yy = (float) i;
				if (arr[i][j] && !isallstop) {

					Random rm = new Random();
					Bitmap bitmap = null;
					int num = 0;
					if (type == 1) {
						num = rm.nextInt(heart_all.length - 1);
						bitmap = bitmapcache
								.getBitmap(heart_all[num], mContext);
					} else if (type == 2) {
						bitmap = bitmapcache.getBitmap(R.drawable.love,
								mContext);
					}
					int bw = bitmap.getWidth();
					int bh = bitmap.getHeight();
					synchronized (holder) {
						Canvas c = null;
						try {

							// 涓嶈杞绘槗鍘婚攣瀹氭暣涓睆骞�
							c = holder.lockCanvas(new Rect(startx + (int) xx
									* bei, starty + (int) yy * bei, startx
									+ (int) xx * bei + bw, starty + (int) yy
									* bei + bh));

							// c = holder.lockCanvas();
							Paint p = new Paint(); // 鍒涘缓鐢荤瑪
							p.setColor(Color.RED);
							// 涓嬮潰杩欐鏄繚璇佸弻缂撳啿鑳介兘鐢讳笂涓滆タ锛屼粠鑰屼笉浼氶棯鐑�

							c.drawBitmap(bitmap, startx + xx * bei, starty + yy
									* bei, p);

							old_num = num;
						} catch (Exception e) {
							e.printStackTrace();
						} finally {
							try{
								if (c != null){
									holder.unlockCanvasAndPost(c);// 缁撴潫閿佸畾鐢诲浘锛屽苟鎻愪氦鏀瑰彉銆�
								}
							}catch(Exception e){
								e.printStackTrace();
							}
						}
					}
				}
			}

		}

	}
	
	
	
	// 鑺� type 1涓鸿姳锛�2涓虹埍蹇�
	/**
	 * 寮�濮媥,寮�濮媃鍧愭爣锛屽瓧鐨勫锛岄珮锛屾枃浠跺悕瀛楋紝鏀惧ぇ鍊嶆暟锛宼ype涓哄浘鐗囩绫�
	 * @param stx
	 * @param sty
	 * @param w
	 * @param h
	 * @param filename
	 * @param beishu
	 * @param type
	 */
	public void show_I(int stx, int sty, int w, int h, String filename,
			int beishu, int type) {
		int startx = stx, starty = sty;
		try {
			int weith = w;
			int height = h;
			boolean[][] arr = new boolean[weith][height]; // 鎻掑叆鐨勬暟缁�
			String file = filename;
			InputStream ins = Util.init().getAssetsInputStream(mContext, file);
			BufferedReader in = new BufferedReader(new InputStreamReader(ins)); //
			String line; // 涓�琛屾暟鎹�
			int row = 0;
			// 閫愯璇诲彇锛屽苟灏嗘瘡涓暟缁勬斁鍏ュ埌鏁扮粍涓�
			StringBuilder sb = new StringBuilder();
			while ((line = in.readLine()) != null) {
				sb.append(line);
			}
			System.out.println(sb.toString());
			in.close();
			String all = sb.toString();
			String[] all_a = all.split("n");
			//鍏堝緱鍒板潗鏍�
			for (int i = 0; i < all_a.length; i++) {
				String[] all_b = all_a[i].split("b");
				System.out.println();
				for (int j = 0; j < all_b.length; j++) {
					if (all_b[j].equals("0")) {
						arr[j][i] = false;
					} else
						arr[j][i] = true;
				}
			}
			int bei = beishu;
			int dis = 25;
			int old_num = -1;
			for (int j = 0; j < height && !isallstop; j++) {
				for (int i = 0; i <= weith && !isallstop; i++) {
					//涓�瀹氳sleep锛岃涓嶇劧鍏朵粬绾跨▼鐢讳笉浜嗕笢瑗�
					Thread.sleep(25);
					Random rm = new Random();
					Bitmap bitmap = null;
					int num = 0;
					if (type == 1) {
						num = rm.nextInt(heart_all.length - 1);
						bitmap = bitmapcache
								.getBitmap(heart_all[num], mContext);
					} else if (type == 2) {
						bitmap = bitmapcache.getBitmap(R.drawable.love,
								mContext);
					}
					int bw = bitmap.getWidth();
					int bh = bitmap.getHeight();
					if (i >= weith  && !isallstop) {
						//鍢垮樋锛屽ソ鍍忚繖娈祍y鍙互涓嶈锛屾噿寰楀幓楠岃瘉鍟︼紝鍔犱簡娌″澶у潖澶勫暒
						synchronized (holder) {
							Canvas c = null;
							try {
								Paint p = new Paint(); // 鍒涘缓鐢荤瑪
								p.setColor(Color.RED);
								int xx_b = i - 1;
								int yy_b = j;
								c = holder.lockCanvas(new Rect(startx
										+ (int) xx_b * bei - dis, starty
										+ (int) yy_b * bei - dis, startx
										+ (int) xx_b * bei + dis, starty
										+ (int) yy_b * bei + dis));

								if (arr[xx_b][yy_b]) {  
									if (old_num != -1) {
										if (type == 1)
											c.drawBitmap(bitmapcache.getBitmap(
													heart_all[old_num],
													mContext), startx + xx_b
													* bei, starty + yy_b * bei,
													p);
										else if (type == 2) {
											c.drawBitmap(bitmapcache.getBitmap(
													R.drawable.love, mContext),
													startx + xx_b * bei, starty
															+ yy_b * bei, p);
										}
									}
								}
							} catch (Exception e) {
								e.printStackTrace();
							} finally {
								try{
									if (c != null){
										holder.unlockCanvasAndPost(c);// 缁撴潫閿佸畾鐢诲浘锛屽苟鎻愪氦鏀瑰彉銆�
									}
								}catch(Exception e){
									e.printStackTrace();
								}
							}
							continue;
						}
					}

					synchronized (holder) {
						Canvas c = null;
						try {
							float xx = (float) i;
							float yy = (float) j;
							//涓嶈杞绘槗鍘婚攣瀹氭暣涓睆骞�
							c = holder.lockCanvas(new Rect(startx + (int) xx
									* bei, starty + (int) yy * bei,
									startx + (int) xx * bei + dis, starty
											+ (int) yy * bei + dis));

							// c = holder.lockCanvas();
							Paint p = new Paint(); // 鍒涘缓鐢荤瑪
							p.setColor(Color.RED);
							//涓嬮潰杩欐鏄繚璇佸弻缂撳啿鑳介兘鐢讳笂涓滆タ锛屼粠鑰屼笉浼氶棯鐑�
							if (i > 0 && !isallstop) {
								int xx_b = i - 1;
								int yy_b = j;
								if (arr[xx_b][yy_b]) {
									if (old_num != -1) {
										if (type == 1)
											c.drawBitmap(bitmapcache.getBitmap(
													heart_all[old_num],
													mContext), startx + xx_b
													* bei, starty + yy_b * bei,
													p);
										else if (type == 2) {
											c.drawBitmap(bitmapcache.getBitmap(
													R.drawable.love, mContext),
													startx + xx_b * bei, starty
															+ yy_b * bei, p);
										}
									}
								}
							}
							if (arr[i][j] && !isallstop) {
								c.drawBitmap(bitmap, startx + xx * bei, starty
										+ yy * bei, p);
							}
							old_num = num;
						} catch (Exception e) {
							e.printStackTrace();
						} finally {
							try{
								if (c != null){
									holder.unlockCanvasAndPost(c);// 缁撴潫閿佸畾鐢诲浘锛屽苟鎻愪氦鏀瑰彉銆�
								}
							}catch(Exception e){
								e.printStackTrace();
							}
						}
					}
					//System.out.print("@");// 鏇挎崲鎴愪綘鍠滄鐨勫浘妗�
					// } else {
					// }
				}
			}

		} catch (Exception e) {
			e.printStackTrace();
		}

	}

	class ShowBackgroundThread extends Thread {
		private SurfaceHolder holder;
		int sh;
		int sw;

		public ShowBackgroundThread(SurfaceHolder holder, String threadname,
				int sw, int sh) {
			this.holder = holder;
			setName(threadname);
			this.sw = sw;
			this.sh = sh;
		}

		@Override
		public void run() {
			for (int i = 0; i <= 10&& !isallstop; i++) {
				//褰搃绛変簬9鏃剁敾宸︿笅瑙掔殑铦磋澏锛屽叾浠栨椂鍊欑敾鏄熸槦
				if (i >= 9&& !isallstop) {
					Bitmap b = null;
					int b_w;
					int b_h ;
					int x_start;
					int x_end;
					int y_start;
					int y_end;
					Rect rt;
					if(i == 9 && !isallstop){
						b = bitmapcache
							.getBitmap(R.drawable.hudei, mContext);
						
						b_w = b.getWidth();
						b_h = b.getHeight();
						x_start = 3 * this.sw / 4-25;
						x_end = x_start + b_w;
						y_start = 2 * this.sh/4+100;
						y_end = y_start + b_h;
						rt = new Rect(x_start,y_start,x_end,y_end);
					}
					else{
						b = bitmapcache.getBitmap(R.drawable.love_m_left, mContext);
						b_w = b.getWidth();
						b_h = b.getHeight();
						x_start = 1 * this.sw / 4-85;
						x_end = x_start + b_w;
						y_start = 2 * this.sh/4+110;
						y_end = y_start + b_h;
						rt = new Rect(x_start,y_start,x_end,y_end);
					}
						
					for (int j = 1; j < 11 && !isallstop; j++) {
						try {
							Thread.sleep(200);
						} catch (InterruptedException e) {
							// TODO 鑷姩鐢熸垚鐨� catch 鍧�
							e.printStackTrace();
						}

						synchronized (holder) {
							Canvas c = null;
							try {
								
								//閿佸彸涓嬭竟涓�閮ㄥ垎
								c = holder.lockCanvas(rt);
								Paint p = new Paint();
								p.setAlpha(j * 10);  //閫忔槑搴�
								
								c.drawBitmap(b, x_start,y_start, p);
							} catch (Exception e) {

								e.printStackTrace();
							} finally {
								try{
									if (c != null){
										holder.unlockCanvasAndPost(c);// 缁撴潫閿佸畾鐢诲浘锛屽苟鎻愪氦鏀瑰彉銆�
									}
								}catch(Exception e){
									e.printStackTrace();
								}
							}
						}// syn
					}
					continue;
				}

				//闅忔満浣嶇疆闅忔満鏄熸槦
				Random rm = new Random();			
				int show_x = (int) Math.round(Math.random()*(sw-50-50)+50);
				int show_y = (int) Math.round(Math.random()*(sh-80-80)+80);
				int xin = rm.nextInt(3);
				
					Bitmap bit = bitmapcache.getBitmap(xin_all[xin], mContext);
				for (int j = 1; j < 11 && !isallstop; j++) {
					try {
						Thread.sleep(100);
					} catch (InterruptedException e) {
						// TODO 鑷姩鐢熸垚鐨� catch 鍧�
						e.printStackTrace();
					}

					synchronized (holder) {
						Canvas c = null;
						try {
							c = holder.lockCanvas(new Rect(show_x, show_y,
									show_x + 55, show_y + 55));
							Paint p = new Paint();
							p.setAlpha(j * 10);  //閫忔槑搴︿粠0寮�濮嬶紝娓愮幇
							c.drawBitmap(bit, show_x, show_y, p);
						} catch (Exception e) {

							e.printStackTrace();
						} finally {
							try{
								if (c != null){
									holder.unlockCanvasAndPost(c);// 缁撴潫閿佸畾鐢诲浘锛屽苟鎻愪氦鏀瑰彉銆�
								}
							}catch(Exception e){
								e.printStackTrace();
							}
						}
					}// syn
				}
			}
			begin.countDown();
		}
	}
	
	int yadd_1200 = 100;

	class showWenZi extends Thread {
		public final int[] mask = { 128, 64, 32, 16, 8, 4, 2, 1 };
		public final String ZK_PATH = "HZK16";
		public final String ENCODE = "GB2312";
		public final String ZK32_PATH = "HZK32F";
		int sw, sh;
		private SurfaceHolder holder;
		int startx = 300, starty = 300;

		private String wenzi;

		public showWenZi(SurfaceHolder holder, String name, String wenzi,
				int w, int h) {
			this.holder = holder;
			setName(name);
			this.sw = w;
			this.sh = h;
			this.wenzi = wenzi;
		}
		
		int onestartx,onestarty,twostartx,twostarty;

		@Override
		public void run() {
			onestartx = 3;
			onestarty = 40;
			twostartx = sw / 2 + 60;
			twostarty = 40;
			
			if(sw/2-60-30>160){
				twostartx = twostartx+20;
				onestartx = onestartx+5;
			}
			if(sw/2-60-50>160){
				twostartx = twostartx+30;
				onestartx = onestartx+10;
			}
			if(sw/2-60-70>160){
				twostartx = twostartx+30;
				onestartx = onestartx+10;
			}
			
			int bei = 8;
			int bei32 = 5;
			//閫傞厤
			if(sh >= 1200) {
				onestarty = onestarty+yadd_1200;
				twostarty = twostarty+yadd_1200;
			}else if(sh>=1000 && sh<1200){
				onestarty = onestarty+50;
				twostarty = twostarty+50;
			}
			
			if(sw >=600) {
				bei = 9;
				bei32= 7;
			}
			if(sw>=700){
				bei32 = 8;
			}
					
			System.out.println("create_wenzi");
			holder.setKeepScreenOn(true);
			// show_I(int stx,int sty,int w,int h,String filename,int beishu
			// show_I(3,20,19,19,"array_zhang.txt",9,2);
			String one = spxml.getConfigSharedPreferences("first_name_1", 
					spxml.getResourceString(R.string.first_et_1));
			if(one.equals("")) one = spxml.getResourceString(R.string.first_et_1);
			String one1 = one.substring(0, 1);
			//String one1 = one.charAt(0);
			if(one1.equals("鏅�"))
				show_I(onestartx, onestarty, 20, 20, "array_wan.txt", bei, 2);
			else if(one1.equals("妤�"))
				show_I(onestartx, onestarty, 20, 20, "array_chu.txt", bei, 2);
			else if(one1.equals("濞�"))
				show_I(onestartx, onestarty, 20, 20, "array_juang.txt", bei, 2);
			else {
				//show_font32(one1, onestartx, onestarty, sw, sh, bei32, 2);
				int bei16 = 8;
				show_font16(one1, onestartx, onestarty, sw, sh, bei16, 2);
			}
			//show_font32("妤�", 3, 40, 20, 20, 5, 2);
			
			String two = spxml.getConfigSharedPreferences("first_name_2", 
					spxml.getResourceString(R.string.first_et_2));
			if(two.equals("")){
				//two = spxml.getResourceString(R.string.first_et_1);
				//鍙湁涓�涓瓧
			}else{
				String two1 = two.substring(0, 1);
				if(two1.equals("鏅�"))
					show_I(twostartx,twostarty, 20, 20, "array_wan.txt", bei, 2);
				else if(two1.equals("妤�"))
					show_I(twostartx,twostarty, 20, 20, "array_chu.txt", bei, 2);
				else if(two1.equals("鐜�"))
					show_I(twostartx,twostarty, 20, 20, "array_ling.txt", bei, 2);
				else if(two1.equals("濞�"))
					show_I(twostartx, twostarty, 20, 20, "array_juang.txt", bei, 2);
				else show_font32(two1, twostartx,twostarty, sw, sh, bei32, 2);
			}
			
			//show_I(sw / 2 + 60, 40, 20, 20, "array_chu.txt", 8, 2);
			// drawText("鏅氭櫄");
			begin.countDown();
		}// run

	}

	class showheart extends Thread {
		private SurfaceHolder holder;
		int sh;
		int sw;

		public showheart(SurfaceHolder holder, String threadname, int sw, int sh) {
			this.holder = holder;
			setName(threadname);
			this.sw = sw;
			this.sh = sh;
		}
		
		int istartx,istarty,lovestartx,lovestarty,ustartx,ustarty;
		

		public void run() {
			
			//灞忓箷閫傞厤
			istartx = -50 + sw / 2;
			istarty = 50;
			
			
			lovestartx = sw / 2 - 16;
			lovestarty = sh / 2 - 68;
			
			ustartx = -94 + sw / 2;
			ustarty = 150 + sh / 2;
			
			if(sh/2 >180+150+118+20) ustarty = 150 + sh / 2 +20;
			if(sh/2 >180+150+118+40) ustarty = 150 + sh / 2 +40;
			if(sh/2 >180+150+118+60) ustarty = 150 + sh / 2 +60;
			if(sh >= 1200) {
				istarty = istarty+yadd_1200;
				ustarty = ustarty+yadd_1200;
			}
			
			System.out.println("create1");
			this.holder.setKeepScreenOn(true);
			FirstSurfaceView.this.show_I(istartx,istarty, 7, 9,
					"array_I.txt", 18, 1);
			run_hua_heart();
			FirstSurfaceView.this.show_I(ustartx,ustarty,
					10, 10, "array_U.txt", 18, 1);
			run_M_M();
			begin.countDown();
		}
		
		private void run_M_M(){
			int start_x = sw/2-100;
			int start_y = sh/2-90;
			Bitmap m_m = bitmapcache.getBitmap(R.drawable.h_m_m, mContext);
			int pic_w = m_m.getWidth();
			int pic_h = m_m.getHeight();
			Rect r = new Rect(start_x,start_y,start_x+pic_w,start_y+pic_h);
			Paint p = new Paint();
			for(int i =1;i<51  && !isallstop;i++){
				try {
					Thread.sleep(200);
				} catch (InterruptedException e) {
					// TODO 鑷姩鐢熸垚鐨� catch 鍧�
					e.printStackTrace();
				}
				p.setAlpha(i*2);
				synchronized (holder) {
					Canvas c = null;
					try {
						c = holder.lockCanvas(r);
						c.drawBitmap(m_m, start_x,start_y, p);  //鐢讳腑闂寸殑蹇�							
					}catch (Exception e) {
						e.printStackTrace();
					} finally {
						try{
							if (c != null){
								holder.unlockCanvasAndPost(c);// 缁撴潫閿佸畾鐢诲浘锛屽苟鎻愪氦鏀瑰彉銆�
							}
						}catch(Exception e){
							e.printStackTrace();
						}
					}					
				}//sy
			}
		}

		private void run_hua_heart() {
			// TODO 鑷姩鐢熸垚鐨勬柟娉曞瓨鏍�
			int startx = sw / 2 - 16, starty = sh / 2 - 68;
			int maxh = 100;  
			int y_dao = starty;
			double begin = 10; // 璧峰浣嶇疆
			Random rm = new Random();
			int old_num = -1;
			float old_xx = 0, old_yy = 0;
			for (int i = 0; i < maxh  && !isallstop; i++) {
				try {
					Thread.sleep(80);
				} catch (InterruptedException e1) {
					// TODO 鑷姩鐢熸垚鐨� catch 鍧�
					e1.printStackTrace();
				}

				int hua_num = rm.nextInt(18);
				Bitmap bit = bitmapcache
						.getBitmap(heart_all[hua_num], mContext);
				begin = begin + 0.2;  //瀵嗗害
				double b = begin / Math.PI;
				double a = 13.5 * (16 * Math.pow(Math.sin(b), 3));  //杩欓噷鐨�13.5鍙互鎺у埗澶у皬
				double d = -13.5
						* (13 * Math.cos(b) - 5 * Math.cos(2 * b) - 2
								* Math.cos(3 * b) - Math.cos(4 * b));
				synchronized (holder) {
					Canvas c = null;
					try {
						float xx = (float) a;
						float yy = (float) d;

						c = holder.lockCanvas(new Rect(
								(int) (startx + xx - 40),
								(int) (starty + yy - 40),
								(int) (startx + xx + 40),
								(int) (starty + yy + 40)));
						Paint p = new Paint(); // 鍒涘缓鐢荤瑪
						p.setColor(Color.RED);
						//鐢讳笂涓�涓紝瑕佷笉鐒朵細闂儊
						if (old_num != -1) {
							Bitmap bb = bitmapcache.getBitmap(
									heart_all[old_num], mContext);
							c.drawBitmap(bb, startx + old_xx, starty + old_yy,
									p);
						}
						c.drawBitmap(bit, startx + xx, starty + yy, p);
						old_num = hua_num;
						old_xx = xx;
						old_yy = yy;
						// c.drawPoint(startx+xx,starty+yy, p);
					} catch (Exception e) {
						e.printStackTrace();
					} finally {
						try{
							if (c != null){
								holder.unlockCanvasAndPost(c);// 缁撴潫閿佸畾鐢诲浘锛屽苟鎻愪氦鏀瑰彉銆�
							}
						}catch(Exception e){
							e.printStackTrace();
						}
					}
				}
			}

		}

	}

	class ShowBianKuang extends Thread{
		int resoureid,startx,starty,endx,endy,speed,type;
		int middlex,middlexy;
		int screen_w,screen_h;
		int pic_w,pic_h;
		Bitmap bit;
		/**
		 * type 1 椤堕儴浠庡乏鍒板彸  
		 * type 2 搴曢儴浠庡彸鍒板乏
		 * type 3 宸﹂儴涓棿
		 * type 4 鍙抽儴涓棿
		 * @param biankuangid
		 * @param screen_w
		 * @param screen_h
		 * @param type
		 * @param speed
		 */
		public ShowBianKuang(int biankuangid,int screen_w,int screen_h,int type,int speed){
			this.resoureid = biankuangid;
			this.speed = speed;
			bit = bitmapcache.getBitmap(resoureid, mContext);
			pic_w = bit.getWidth();
			pic_h = bit.getHeight();
			this.type = type;
			this.screen_h = screen_h;
			this.screen_w = screen_w;
			
			
		}
		
		public void run(){
			if(type == 1){
				startx = 0;
				starty = 0;
				endx = screen_w - pic_w;
				endy = 0;
				middlex = screen_w/2-pic_w/2;
				showUPBK();
			}else if(type == 2){
				startx = screen_w;
				starty = screen_h-pic_h-50;
				endx = 0;
				endy = screen_h;
				middlex = screen_w/2-pic_w/2;
				showDOWNBK();
			}
			else if(type == 3){
				startx = -54;
				starty = screen_h/2-120;
				endx = startx+pic_w;
				endy = starty+pic_h;
				showMidd();
			}else if(type == 4){
				startx = screen_w-62;
				starty = screen_h/2-120;
				endx = startx+pic_w;
				endy = starty+pic_h;
				showMidd();
			}
			begin.countDown();
		}
		
		
		private void showMidd(){
			try {
				Thread.sleep(4000);
			} catch (InterruptedException e1) {
				// TODO 鑷姩鐢熸垚鐨� catch 鍧�
				e1.printStackTrace();
			}
			Rect rt = null;
			rt = new Rect(startx,starty,endx,endy);
			for(int i =1;i<100 && !isallstop;i++){
				try {
					Thread.sleep(400);
				} catch (InterruptedException e) {
					// TODO 鑷姩鐢熸垚鐨� catch 鍧�
					e.printStackTrace();
				}
				Paint pa = new Paint(); // 鍒涘缓鐢荤瑪
				pa.setAlpha(1*i);
				synchronized (holder) {
					Canvas c = null;
					try {
						c = holder.lockCanvas(rt);
						c.drawBitmap(bit, startx,starty, pa);  //鐢讳腑闂寸殑蹇�							
					}catch (Exception e) {
						e.printStackTrace();
					} finally {
						try{
							if (c != null){
								holder.unlockCanvasAndPost(c);// 缁撴潫閿佸畾鐢诲浘锛屽苟鎻愪氦鏀瑰彉銆�
							}
						}catch(Exception e){
							e.printStackTrace();
						}
					}					
				}//sy
			}
		}
		
		public void showDOWNBK(){
			Rect rt = null;
			boolean isruning = true;
			boolean isone = false;
			while(isruning  && !isallstop){
				try {
					Thread.sleep(50);
				} catch (InterruptedException e1) {
					// TODO 鑷姩鐢熸垚鐨� catch 鍧�
					e1.printStackTrace();
				}
				synchronized (holder) {
					Canvas c = null;
					try {
						if(startx <= endx || isone) {
							isone = true;
							startx = startx +speed;
						}else
							startx = startx -speed;
						
						if(isone){
							if(startx >= middlex) isruning = false;
						}
						
						rt = new Rect(0,starty,screen_w,screen_h);
						
						c = holder.lockCanvas(rt);
						Paint p = new Paint(); // 鍒涘缓鐢荤瑪
						//c.drawColor(co);  //
						c.drawColor(Color.TRANSPARENT,Mode.CLEAR);
						c.drawBitmap(bit, startx, starty, p);
					}catch (Exception e) {
						e.printStackTrace();
					} finally {
						try{
							if (c != null){
								holder.unlockCanvasAndPost(c);// 缁撴潫閿佸畾鐢诲浘锛屽苟鎻愪氦鏀瑰彉銆�
							}
						}catch(Exception e){
							e.printStackTrace();
						}
					}
				}
			
			
			}//while
			
			//鍑哄績
			if(type == 2){	
				//浠ヤ笅涓虹敾蹇冪殑鏁版嵁
				Bitmap love_middle_down = bitmapcache.getBitmap(R.drawable.love_middle_down, mContext);
				int love_md_w = love_middle_down.getWidth();
				int love_md_h = love_middle_down.getHeight();
				int x = middlex+pic_w/2-love_md_w/2+2;
				int y = starty + 38;
				Rect love_middle_down_rt = new Rect(x-50,y,x+love_md_w+50,y+love_md_h);
				
				//淇鐨別ndy
				int endy_m = endy-50;
				//浠ヤ笅涓虹敾涓嬮儴宸﹁竟妗嗙殑鏁版嵁
				Bitmap love_down_left = bitmapcache.getBitmap(R.drawable.down_left, mContext);
				int love_dl_w = love_down_left.getWidth();
				int love_dl_h = love_down_left.getHeight();
				int dl_x = 0;
				int dl_y = endy_m-love_dl_h;
				Rect love_down_left_rt = new Rect(0,endy_m-love_dl_h,love_dl_w,endy);
				
				//鍙宠竟
				Bitmap love_down_right = bitmapcache.getBitmap(R.drawable.down_right, mContext);
				int love_dr_w = love_down_right.getWidth();
				int love_dr_h = love_down_right.getHeight();
				int dr_x = screen_w-love_dr_w-3;
				int dr_y = endy_m-love_dr_h;
				Rect love_down_right_rt = new Rect(dr_x,endy_m-love_dr_h,dr_x+love_dr_w,endy);
				
				for(int i =1;i<21  && !isallstop;i++){
					try {
						Thread.sleep(350);
					} catch (InterruptedException e) {
						// TODO 鑷姩鐢熸垚鐨� catch 鍧�
						e.printStackTrace();
					}
					Paint p = new Paint(); // 鍒涘缓鐢荤瑪
					p.setAlpha(5*i);
					synchronized (holder) {
						Canvas c = null;
						try {
							c = holder.lockCanvas(love_middle_down_rt);
							c.drawBitmap(love_middle_down, x,y, p);  //鐢讳腑闂寸殑蹇�							
						}catch (Exception e) {
							e.printStackTrace();
						} finally {
							holder.unlockCanvasAndPost(c);// 缁撴潫閿佸畾鐢诲浘锛屽苟鎻愪氦鏀瑰彉銆�
						}
						try {
							c = holder.lockCanvas(love_down_left_rt);
							c.drawBitmap(love_down_left, dl_x, dl_y, p);  //鐢诲乏杈�					
						}catch (Exception e) {
							e.printStackTrace();
						} finally {
							try{
								if (c != null){
									holder.unlockCanvasAndPost(c);// 缁撴潫閿佸畾鐢诲浘锛屽苟鎻愪氦鏀瑰彉銆�
								}
							}catch(Exception e){
								e.printStackTrace();
							}
						}
						try {
							c = holder.lockCanvas(love_down_right_rt);							
							c.drawBitmap(love_down_right,dr_x,dr_y,p);  //鐢诲彸杈�
						}catch (Exception e) {
							e.printStackTrace();
						} finally {
							try{
								if (c != null){
									holder.unlockCanvasAndPost(c);// 缁撴潫閿佸畾鐢诲浘锛屽苟鎻愪氦鏀瑰彉銆�
								}
							}catch(Exception e){
								e.printStackTrace();
							}
						}
					}//sy
				}//for	
			}//if
			
		}
		
		public void showUPBK(){
			Rect rt = null;
			boolean isruning = true;
			boolean isone = false;
			while(isruning && !isallstop){
				try {
					Thread.sleep(50);
				} catch (InterruptedException e1) {
					// TODO 鑷姩鐢熸垚鐨� catch 鍧�
					e1.printStackTrace();
				}
				synchronized (holder) {
					Canvas c = null;
					try {
						if(startx >= endx || isone) {
							isone = true;
							startx = startx -speed;
						}else
							startx = startx +speed;
						
						if(isone){
							if(startx <= middlex) isruning = false;
						}
						if(type==1){
							rt = new Rect(startx-25,starty,startx+pic_w+5,pic_h);
						}
						c = holder.lockCanvas(rt);
						Paint p = new Paint(); // 鍒涘缓鐢荤瑪
						//c.drawColor(co);  //
						c.drawColor(Color.TRANSPARENT,Mode.CLEAR);
						c.drawBitmap(bit, startx, starty, p);
					}catch (Exception e) {
						e.printStackTrace();
					} finally {
						try{
							if (c != null){
								holder.unlockCanvasAndPost(c);// 缁撴潫閿佸畾鐢诲浘锛屽苟鎻愪氦鏀瑰彉銆�
							}
						}catch(Exception e){
							e.printStackTrace();
						}
					}
				}
			
			
			}//while
			
			//鍑哄績
			if(type == 1){	
				//浠ヤ笅涓虹敾蹇冪殑鏁版嵁
				Bitmap love_middle_up = bitmapcache.getBitmap(R.drawable.love_middle_up, mContext);
				int love_mu_w = love_middle_up.getWidth();
				int love_mu_h = love_middle_up.getHeight();
				int x = middlex+pic_w/2-love_mu_w/2;
				Rect love_middle_up_rt = new Rect(x-50,0,x+love_mu_w+50,love_mu_h);
				
				//浠ヤ笅涓虹敾涓婇儴宸﹁竟妗嗙殑鏁版嵁
				Bitmap love_up_left = bitmapcache.getBitmap(R.drawable.up_left, mContext);
				int love_ul_w = love_up_left.getWidth();
				int love_ul_h = love_up_left.getHeight();
				Rect love_up_left_rt = new Rect(0,0,love_ul_w,love_ul_h);
				
				//鍙宠竟
				Bitmap love_up_right = bitmapcache.getBitmap(R.drawable.up_right, mContext);
				int love_up_w = love_up_right.getWidth();
				int love_up_h = love_up_right.getHeight();
				int ur_x = screen_w-love_ul_w-10;
				Rect love_up_right_rt = new Rect(ur_x,0,ur_x+love_up_w,love_up_h);
				
				for(int i =1;i<31 && !isallstop;i++){
					try {
						Thread.sleep(350);
					} catch (InterruptedException e) {
						// TODO 鑷姩鐢熸垚鐨� catch 鍧�
						e.printStackTrace();
					}
					Paint p = new Paint(); // 鍒涘缓鐢荤瑪
					p.setAlpha(33*i);
					synchronized (holder) {
						Canvas c = null;
						try {
							c = holder.lockCanvas(love_middle_up_rt);
							c.drawBitmap(love_middle_up, x,0, p);  //鐢讳腑闂寸殑蹇�							
						}catch (Exception e) {
							e.printStackTrace();
						} finally {
							try{
								if (c != null){
									holder.unlockCanvasAndPost(c);// 缁撴潫閿佸畾鐢诲浘锛屽苟鎻愪氦鏀瑰彉銆�
								}
							}catch(Exception e){
								e.printStackTrace();
							}
						}
						try {
							c = holder.lockCanvas(love_up_left_rt);
							c.drawBitmap(love_up_left, 0,0, p);  //鐢诲乏杈�					
						}catch (Exception e) {
							e.printStackTrace();
						} finally {
							try{
								if (c != null){
									holder.unlockCanvasAndPost(c);// 缁撴潫閿佸畾鐢诲浘锛屽苟鎻愪氦鏀瑰彉銆�
								}
							}catch(Exception e){
								e.printStackTrace();
							}
						}
						try {
							c = holder.lockCanvas(love_up_right_rt);							
							c.drawBitmap(love_up_right,ur_x,0,p);  //鐢诲彸杈�
						}catch (Exception e) {
							e.printStackTrace();
						} finally {
							try{
								if (c != null){
									holder.unlockCanvasAndPost(c);// 缁撴潫閿佸畾鐢诲浘锛屽苟鎻愪氦鏀瑰彉銆�
								}
							}catch(Exception e){
								e.printStackTrace();
							}
						}
					}//sy
				}//for	
			}//if
		}
		
	}
}
