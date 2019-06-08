package k.opt.monitor.time;


import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;

import k.core.util.KLogUtil;


public class TimeMonitor {
    private final String TAG = "TimeMonitor";
    private int monitorId = -1;
    private LinkedHashMap<String, Long> mTimeTag = new LinkedHashMap<String, Long>();
    private long mStartTime = 0;

    public TimeMonitor(int id) {
        KLogUtil.D(TAG,"init TimeMonitor id:" + id);
        monitorId = id;
        startMoniter();
    }

    public int getMonitorId() {
        return monitorId;
    }

    public void startMoniter() {
        if (mTimeTag.size() > 0) {
            mTimeTag.clear();
        }
        mStartTime = System.currentTimeMillis();
    }

    public void recodingTimeTag(String tag) {
        if (mTimeTag.get(tag) != null) {
            mTimeTag.remove(tag);
        }
        long time = System.currentTimeMillis() - mStartTime;
        KLogUtil.D(TAG, tag + ":" + time + "ms");
        mTimeTag.put(tag, time);
    }
    public void end(String tag,boolean writeLog){
        recodingTimeTag(tag);
        end(writeLog);
    }
    public void end(boolean writeLog) {
        if (writeLog) {
            //TODO write local
        }
        testShowData();
    }
    public void testShowData(){
        if(mTimeTag.size() <= 0){
            KLogUtil.D(TAG,"mTimeTag is empty!");
            return;
        }
        Iterator iterator = mTimeTag.keySet().iterator();
        while (iterator != null && iterator.hasNext()){
           String tag = (String)iterator.next();
            KLogUtil.D(TAG,tag + ":" +  mTimeTag.get(tag));
        }
    }
    public HashMap<String, Long> getTimeTags() {
        return mTimeTag;
    }
}
