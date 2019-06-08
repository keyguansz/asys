package k.opt.jobscheduler;


import android.app.job.JobInfo;
import android.app.job.JobScheduler;
import android.content.Context;



class JobScheduleManager {
    private static final JobScheduleManager ourInstance = new JobScheduleManager();
    private Context mCtx;
    private JobScheduler mJS;

    static JobScheduleManager getInstance() {
        return ourInstance;
    }
    public void init(Context ctx){
        mCtx = ctx;
        mJS = (JobScheduler)mCtx.getSystemService(Context.JOB_SCHEDULER_SERVICE);
    }
    public boolean addJobScheduleTask(int task_id){
        JobInfo.Builder builder = null;
        switch (task_id){
            case 1:
                builder.setPeriodic(1000);
                break;
            case 2:
                builder.setPersisted(false);
                break;
            default:
        }
       if (mJS != null){
           return mJS.schedule(builder.build()) > 0;
       }else {
           return false;
       }
    }


    private JobScheduleManager() {
    }
}
