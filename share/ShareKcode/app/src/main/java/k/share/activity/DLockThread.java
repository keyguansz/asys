package k.share.activity;

import android.util.Log;

/**
 * @author :key.guan
 * @package :k.opt.activity
 * @date : 2017/6/22
 * Description:
 * Copyright (c) 2017. DJI All Rights Reserved.
 */

public class DLockThread extends Thread{
    static Object a = new Object();
    static Object b = new Object();
    String firstObj = "";
    public DLockThread(String firstObj, String threadName){
        this.firstObj = firstObj;
        super.setName(threadName);
    }

    public void run() {
        while(true)
            if("a".equals(firstObj)){
                synchronized(a){//获取对象a的锁，然后去获取对象b的锁
                    Log.e("DLockThread",Thread.currentThread().getName()+"--->"+a.toString());
                    synchronized(b){
                        Log.e("DLockThread",Thread.currentThread().getName()+"--->"+b.toString());
                    }
                }
            }else if("b".equals(firstObj)) {
                synchronized(b){
                    Log.e("DLockThread",Thread.currentThread().getName()+"--->"+b.toString());
                    synchronized(a){
                        Log.e("DLockThread",Thread.currentThread().getName()+"--->"+a.toString());
                    }
                }
            }
    }
    public static void main() {
        DLockThread t1 = new DLockThread("a", "t1");
        DLockThread t2 = new DLockThread("b", "t2");
        t1.start();
        t2.start();
    }
}




