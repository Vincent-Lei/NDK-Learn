package com.lei.ndk.nativetest;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;

import com.lei.ndk.R;

/**
 * Created by Vincent.Lei on 2018/11/21.
 * Title：
 * Note：
 */
public class NativeTestActivity extends AppCompatActivity implements View.OnClickListener {
    private NativeTestObject mNativeTestObject = new NativeTestObject();

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_native_test);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_ffmpeg_config_info:
                mNativeTestObject.printFFMPEGConfigInfo();
                break;
            case R.id.btn_native_call_java:
                mNativeTestObject.nativeCallJavaMethod();
                break;
            case R.id.btn_customer_and_producer:
                mNativeTestObject.nativeCustomerAndProducer();
                break;
        }
    }
}
