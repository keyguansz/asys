package k.opt.activity;

import android.app.Activity;
import android.content.ComponentName;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;

/**
 * @package :k.opt.activity

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




