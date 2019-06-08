package k.core.util;

import android.util.Log;

public class KLogUtil {
    private static final String TAG = "OPT";

    public static void D(String log) {
        Log.e(TAG, log);
    }

    public static void D(String clsName, String log) {
        D(clsName + "->" + log);
    }

    public static void D(String clsName, String methodName, String log) {
        D(clsName + "->" + methodName + "():" + log);
    }
}
