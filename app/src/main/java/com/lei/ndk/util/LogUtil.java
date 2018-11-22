package com.lei.ndk.util;

import android.util.Log;

/**
 * Created by Vincent.Lei on 2018/11/21.
 * Title：
 * Note：
 */
public class LogUtil {
    private static final String TAG = "Vincent";

    public static void d(String msg) {
        if (msg != null)
            Log.d(TAG, msg);
    }
    public static void e(String msg) {
        if (msg != null)
            Log.e(TAG, msg);
    }
}
