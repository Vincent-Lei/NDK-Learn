package com.lei.ndk.audio;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;

import com.lei.ndk.R;
import com.lei.ndk.util.FileUtil;
import com.lei.ndk.util.LogUtil;

import java.io.File;

/**
 * Created by Vincent.Lei on 2018/11/21.
 * Title：
 * Note：
 */
public class AudioActivity extends AppCompatActivity implements View.OnClickListener {
    private LeiAudioPlayer mPlayer;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_test);
        mPlayer = new LeiAudioPlayer();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_normal_play:
                File file = FileUtil.getLocalAssetFile(this, "See_You_Again.mp3");
                if (file != null && file.exists()) {
                    mPlayer.setDataSource(file.getAbsolutePath());
                    mPlayer.prepared();
                    mPlayer.start();
                    return;
                }
                LogUtil.e("no found See_You_Again.mp3");
                break;
        }
    }
}
