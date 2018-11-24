package com.lei.ndk.audio;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.TextView;

import com.lei.ndk.R;
import com.lei.ndk.util.FileUtil;
import com.lei.ndk.util.LogUtil;

import java.io.File;

/**
 * Created by Vincent.Lei on 2018/11/21.
 * Title：
 * Note：
 */
public class AudioActivity extends AppCompatActivity implements View.OnClickListener, LeiAudioPlayer.ICallBack {
    private LeiAudioPlayer mPlayer;
    TextView textView_time;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_test);
        textView_time = findViewById(R.id.tv_time);
        mPlayer = new LeiAudioPlayer();
        mPlayer.setCallBack(this);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mPlayer != null)
            mPlayer.destory();
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
            case R.id.btn_play_network:
                mPlayer.setDataSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
                mPlayer.prepared();
                mPlayer.start();
                break;
            case R.id.btn_pause:
                mPlayer.pause();
                break;
            case R.id.btn_resume:
                mPlayer.resume();
                break;
        }
    }

    @Override
    public void onDurationChanged(int current, int all) {
        textView_time.setText(formatTime(current) + "/" + formatTime(all));
    }

    private String formatTime(int second) {
        StringBuilder stringBuilder = new StringBuilder();
        int minute = second / 60;
        if (minute < 10)
            stringBuilder.append("0").append(minute);
        else
            stringBuilder.append(minute);
        stringBuilder.append(" : ");
        int secondLeft = second % 60;
        if (secondLeft < 10)
            stringBuilder.append("0").append(secondLeft);
        else
            stringBuilder.append(secondLeft);
        return stringBuilder.toString();
    }
}
