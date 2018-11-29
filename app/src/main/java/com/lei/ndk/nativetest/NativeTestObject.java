package com.lei.ndk.nativetest;

import android.widget.Toast;

import com.lei.ndk.NDKApplication;

/**
 * Created by Vincent.Lei on 2018/11/21.
 * Title：
 * Note：
 */
public class NativeTestObject {
    public native void printFFMPEGConfigInfo();

    public native void nativeCallJavaMethod();

    public native void nativeCustomerAndProducer();

    public void callByNative(String nativeMsg) {
        Toast.makeText(NDKApplication.getApplication(), nativeMsg, Toast.LENGTH_SHORT).show();
    }

    public native void watchUninstall(String packageDir, int sdkVersion);
}
