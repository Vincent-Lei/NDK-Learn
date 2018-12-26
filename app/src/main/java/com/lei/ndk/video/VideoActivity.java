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
import android.widget.TextView;

import com.lei.ndk.R;
import com.lei.ndk.util.FileUtil;
import com.lei.ndk.util.LogUtil;
import com.sdk.video.VideoGLSurfaceView;
import com.sdk.video.VideoPlayer;

import java.util.List;

/**
 * Created by Vincent.Lei on 2018/12/24.
 * Title：
 * Note：
 */
public class VideoActivity extends AppCompatActivity implements View.OnClickListener {
    private VideoPlayer mPlayer;
    private VideoGLSurfaceView glSurfaceView;
    ListView listView;
    private List<VideoBean> mVideoList;
    int colorMusicName;
    int colorMusicPath;
    int colorPlayStatus;
    private int mPlaySelection = -1;
    private VideoAdapter mAdapter;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video);
        colorMusicName = getResources().getColor(R.color.colorAccent);
        colorMusicPath = Color.rgb(100, 100, 100);
        colorPlayStatus = getResources().getColor(R.color.colorplay);

        TextView textView = findViewById(R.id.tv_video_folder);
        textView.setText("请将视频文件放在" + FileUtil.getVideoFolder(this));

        listView = findViewById(R.id.lv);
        glSurfaceView = findViewById(R.id.view_gl_sv);
        mPlayer = new VideoPlayer();
        mPlayer.setVideoGLSurfaceView(glSurfaceView);
        init();


        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                mPlaySelection = position;
                VideoBean videoBean = mVideoList.get(mPlaySelection);
                mPlayer.prepare(videoBean.path);
                mPlayer.start();
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
