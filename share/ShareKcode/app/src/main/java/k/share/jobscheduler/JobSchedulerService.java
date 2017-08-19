package k.share.jobscheduler;

import android.app.job.JobParameters;
import android.app.job.JobService;

/**
 * @author :key.guan
 * @package :k.opt.jobscheduler
 * @date : 2017/6/5
 * Description:
 * Copyright (c) 2017. DJI All Rights Reserved.
 */

public class JobSchedulerService extends JobService {
    @Override
    public boolean onStartJob(JobParameters params) {
        return false;
    }

    @Override
    public boolean onStopJob(JobParameters params) {
        return false;
    }
}
