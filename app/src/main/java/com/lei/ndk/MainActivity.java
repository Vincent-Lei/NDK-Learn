package com.lei.ndk;

import android.Manifest;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.view.View;

import com.lei.ndk.audio.AudioActivity;
import com.lei.ndk.nativetest.NativeTestActivity;
import com.lei.ndk.opengles.OpenGLESTestActivity;
import com.lei.ndk.transcode.TranscodeActivity;
import com.lei.ndk.video.VideoActivity;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        NDKObject.nativeHello();
        ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
    }


    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_demo_info:
                startActivity(new Intent(this, NativeTestActivity.class));
                break;
            case R.id.btn_audio:
                startActivity(new Intent(this, AudioActivity.class));
                break;
            case R.id.btn_opengles_test:
                startActivity(new Intent(this, OpenGLESTestActivity.class));
                break;
            case R.id.btn_transcode:
                startActivity(new Intent(this, TranscodeActivity.class));
                break;
            case R.id.btn_video:
                startActivity(new Intent(this, VideoActivity.class));
                break;
        }
    }
}
