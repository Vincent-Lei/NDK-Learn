package com.lei.ndk;

import android.app.Application;

/**
 * Created by Vincent.Lei on 2018/11/21.
 * Title：
 * Note：
 */
public class NDKApplication extends Application {
    private static NDKApplication mApplication;

    public static NDKApplication getApplication() {
        return mApplication;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mApplication = this;
    }
}
