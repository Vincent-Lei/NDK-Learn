package com.lei.ndk.transcode;

import android.os.AsyncTask;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.RadioGroup;
import android.widget.TextView;

import com.lei.ndk.R;
import com.lei.ndk.audio.AudioFinder;
import com.lei.ndk.audio.MusicBean;
import com.lei.ndk.util.FileUtil;
import com.sdk.audio.transcoding.AudioFileTranscoder;
import com.sdk.audio.transcoding.TranscoderFactory;

import java.io.File;
import java.util.List;

/**
 * Created by Vincent.Lei on 2018/12/14.
 * Title：
 * Note：
 */
public class TranscodeActivity extends AppCompatActivity {
    List<MusicBean> mMusicFileList;
    ListView listView;
    MusicAdapter mAdapter;
    private int transcodeType = TranscoderFactory.TYPE_AAC;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_transcode);
        TextView textView = findViewById(R.id.tv_music_folder);
        textView.setText("请将音乐文件放在" + FileUtil.getMusicFolder(this));
        listView = findViewById(R.id.lv_music);
        init();

        RadioGroup radioGroup = findViewById(R.id.rg);
        radioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                switch (checkedId) {
                    case R.id.rb_aac:
                        transcodeType = TranscoderFactory.TYPE_AAC;
                        break;
                    case R.id.rb_mp3:
                        transcodeType = TranscoderFactory.TYPE_MP3;
                        break;
                    case R.id.rb_amr:
                        transcodeType = TranscoderFactory.TYPE_AMR;
                        break;
                }
            }
        });
    }

    private void doTrans(int position) {
        final MusicBean musicBean = mMusicFileList.get(position);
        if (musicBean.inOnTranscoding)
            return;
        String fileName = musicBean.name;
        int index = fileName.lastIndexOf(".");
        if (index > 0)
            fileName = fileName.substring(0, index);
        String END = (transcodeType == TranscoderFactory.TYPE_AAC ? "-AAC" : (transcodeType == TranscoderFactory.TYPE_AMR ? "-AMR" : "-MP3"));
        String suffix = (transcodeType == TranscoderFactory.TYPE_AAC ? ".aac" : (transcodeType == TranscoderFactory.TYPE_AMR ? ".amr" : ".mp3"));

        File saveDest = FileUtil.getRecordFile(this, fileName + END, suffix, true);
        AudioFileTranscoder audioFileTranscoder = TranscoderFactory.createAudioFileTranscoder(transcodeType, new File(musicBean.path), saveDest);
        audioFileTranscoder.setCallBack(new TranscoderFactory.ITranscodeCallBack() {
            @Override
            public void onTranscodeFinished() {
                musicBean.inOnTranscoding = false;
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        mAdapter.notifyDataSetChanged();
                    }
                });

            }
        });
        audioFileTranscoder.start();
        musicBean.inOnTranscoding = true;
        mAdapter.notifyDataSetChanged();
    }

    private void init() {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                mMusicFileList = AudioFinder.findLocalMusic(TranscodeActivity.this);
                return null;
            }

            @Override
            protected void onPostExecute(Void aVoid) {
                super.onPostExecute(aVoid);
                mAdapter = new MusicAdapter();
                listView.setAdapter(mAdapter);
            }
        }.execute();


    }

    private class MusicAdapter extends BaseAdapter implements View.OnClickListener {
        @Override
        public int getCount() {
            return mMusicFileList == null ? 0 : mMusicFileList.size();
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
            if (convertView == null) {
                convertView = getLayoutInflater().inflate(R.layout.music_item, null);
                ViewHolder viewHolder = new ViewHolder(convertView);
                viewHolder.btn_trans.setOnClickListener(this);
                convertView.setTag(viewHolder);
            }
            refreshItem(convertView, position);
            return convertView;
        }

        public void refreshItem(View convertView, int position) {
            MusicBean musicBean = mMusicFileList.get(position);
            ViewHolder viewHolder = (ViewHolder) convertView.getTag();
            viewHolder.textView_name.setText(musicBean.name);
            viewHolder.textView_folder.setText(musicBean.path);
            viewHolder.btn_trans.setTag(position);
            viewHolder.btn_trans.setVisibility(musicBean.inOnTranscoding ? View.GONE : View.VISIBLE);
            viewHolder.progressBar.setVisibility(musicBean.inOnTranscoding ? View.VISIBLE : View.GONE);
        }

        @Override
        public void onClick(View v) {
            int position = (int) v.getTag();
            doTrans(position);
        }

        private class ViewHolder {
            TextView textView_name;
            TextView textView_folder;
            Button btn_trans;
            ProgressBar progressBar;

            public ViewHolder(View convertView) {
                textView_name = convertView.findViewById(R.id.tv_music_name);
                textView_folder = convertView.findViewById(R.id.tv_music_path);
                btn_trans = convertView.findViewById(R.id.btn_trans);
                progressBar = convertView.findViewById(R.id.pb);
            }
        }
    }
}
