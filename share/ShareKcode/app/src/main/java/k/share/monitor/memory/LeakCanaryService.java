package k.share.monitor.memory;

import com.squareup.leakcanary.AnalysisResult;
import com.squareup.leakcanary.DisplayLeakService;
import com.squareup.leakcanary.HeapDump;

import k.core.util.KLogUtil;

/**
 * @desc:
 * @ref:
 * @author: key.guan @ 2017/6/4
 */
public class LeakCanaryService extends DisplayLeakService{
    private final String TAG = "LeakCanaryService";
    @Override
    protected void afterDefaultHandling(HeapDump heapDump, AnalysisResult result, String leakInfo) {
        KLogUtil.D(TAG,"afterDefaultHandling:" + leakInfo);

        super.afterDefaultHandling(heapDump, result, leakInfo);
    }
}
