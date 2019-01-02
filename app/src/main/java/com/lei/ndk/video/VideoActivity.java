package com.lei.ndk.video;

import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.style.ForegroundColorSpan;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.SeekBar;
import android.widget.TextView;

import com.lei.ndk.R;
import com.lei.ndk.util.FileUtil;
import com.sdk.video.VideoGLSurfaceView;
import com.sdk.video.VideoHardwareUtil;
import com.sdk.video.VideoPlayer;

import java.util.List;

/**
 * Created by Vincent.Lei on 2018/12/24.
 * Title：
 * Note：
 */
public class VideoActivity extends AppCompatActivity implements View.OnClickListener, VideoPlayer.IVideoCallBack {
    private VideoPlayer mPlayer;
    private VideoGLSurfaceView glSurfaceView;
    ListView listView;
    private List<VideoBean> mVideoList;
    int colorMusicName;
    int colorMusicPath;
    int colorPlayStatus;
    private int mPlaySelection = -1;
    private VideoAdapter mAdapter;

    SeekBar seekBar_playDuration;
    TextView textView_currentDuration;
    TextView textView_allDuration;

    private int allDuration;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video);
        colorMusicName = getResources().getColor(R.color.colorAccent);
        colorMusicPath = Color.rgb(100, 100, 100);
        colorPlayStatus = getResources().getColor(R.color.colorplay);

        TextView textView = findViewById(R.id.tv_video_folder);
        textView.setText("请将视频文件放在" + FileUtil.getVideoFolder(this));
        textView_currentDuration = findViewById(R.id.tv_current_duration);
        textView_allDuration = findViewById(R.id.tv_all_duration);
        seekBar_playDuration = findViewById(R.id.seekBar_playDuration);
        listView = findViewById(R.id.lv);
        glSurfaceView = findViewById(R.id.view_gl_sv);




        ViewGroup.LayoutParams lp = glSurfaceView.getLayoutParams();
        lp.height = (int) (getResources().getDisplayMetrics().widthPixels * 9 / 16.0f);
        glSurfaceView.setLayoutParams(lp);
        mPlayer = new VideoPlayer();
        mPlayer.setVideoCallBack(this);
        mPlayer.setVideoGLSurfaceView(glSurfaceView);
        init();


        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (mPlaySelection == position)
                    return;
                mPlaySelection = position;
                VideoBean videoBean = mVideoList.get(mPlaySelection);
                mPlayer.prepare(videoBean.path);
                mPlayer.start();
                mAdapter.notifyDataSetChanged();
            }
        });


        seekBar_playDuration.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                int progress = seekBar.getProgress();
                if (allDuration > 0) {
                    mPlayer.seek((int) (progress * allDuration / 100.0f));
                }

            }
        });

    }

    @Override
    public void onClick(View v) {
    }

    private void init() {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                mVideoList = VideoFinder.findLocalVideo(getApplicationContext());
                return null;
            }

            @Override
            protected void onPostExecute(Void aVoid) {
                mAdapter = new VideoAdapter();
                listView.setAdapter(mAdapter);
            }
        }.execute();
    }


    @Override
    protected void onStop() {
        if (isFinishing() && mPlayer != null) {
            mPlayer.destory();
        }
        super.onStop();
    }

    @Override
    public void onPreparedFinished(boolean success, int duration, int width, int height) {
        allDuration = duration;
        if (allDuration == 0) {
            textView_allDuration.setText(null);
        } else
            textView_allDuration.setText(formatTime(allDuration));
    }

    @Override
    public void onUpdateTimeClock(int currentClock) {
        textView_currentDuration.setText(formatTime(currentClock));
        if (allDuration > 0) {
            int percent = (int) ((currentClock * 1.0f / allDuration) * 100);
            seekBar_playDuration.setProgress(percent);
        }
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

    private class VideoAdapter extends BaseAdapter {
        @Override
        public int getCount() {
            return (mVideoList == null ? 0 : mVideoList.size());
        }

        @Override
        public Object getItem(int position) {
            return null;
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            TextView textView = (TextView) convertView;
            if (textView == null) {
                textView = new TextView(VideoActivity.this);
                textView.setTextColor(colorMusicPath);
            }
            textView.setGravity(Gravity.CENTER);
            textView.setPadding(20, 20, 20, 20);
            String musicName = mVideoList.get(position).name;
            SpannableString spannableString = new SpannableString(musicName + "\n\n" + mVideoList.get(position).path);
            if (!TextUtils.isEmpty(musicName))
                spannableString.setSpan(new ForegroundColorSpan(colorMusicName), 0, musicName.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            textView.setText(spannableString);
            convertView = textView;
            convertView.setBackgroundColor((mPlaySelection == position) ? colorPlayStatus : Color.WHITE);
            return convertView;
        }
    }


}
