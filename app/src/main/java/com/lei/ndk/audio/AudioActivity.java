package com.lei.ndk.audio;

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
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.TextView;

import com.lei.ndk.R;
import com.lei.ndk.util.FileUtil;

import java.io.File;
import java.text.DecimalFormat;
import java.util.List;

/**
 * Created by Vincent.Lei on 2018/11/21.
 * Title：
 * Note：
 */
public class AudioActivity extends AppCompatActivity implements View.OnClickListener, LeiAudioPlayer.ICallBack {
    private static final String NET_SOURCE = "http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3";
    private static final int PLAY_STATUS_STOPED = 0;
    private static final int PLAY_STATUS_PLAYING = 1;
    private static final int PLAY_STATUS_PAUSE = 2;
    private LeiAudioPlayer mPlayer;
    TextView textView_currentDuration;
    TextView textView_allDuration;
    Button button_audioMute;
    Button button_status;
    Button button_pitch;
    Button button_speed;
    Button button_recordStatus;
    Button button_recordStop;
    ListView listView;
    SeekBar seekBar_playDuration;
    SeekBar seekBar_volume;
    ProgressBar progressBar_amplitude;
    int currentSec;
    int allSec;
    List<MusicBean> mMusicFileList;
    MusicAdapter mAdapter;
    int mPlaySelection = -1;
    int mPlayStatus;
    int colorMusicName;
    int colorMusicPath;
    int colorPlayStatus;
    int audioMute = LeiAudioPlayer.Mute.DOUBLE;
    float pitch = 1.0f, speed = 1.0f;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_test);

        colorMusicName = getResources().getColor(R.color.colorAccent);
        colorMusicPath = Color.rgb(100, 100, 100);
        colorPlayStatus = getResources().getColor(R.color.colorplay);


        TextView textView = findViewById(R.id.tv_music_folder);
        textView.setText("请将音乐文件放在" + FileUtil.getMusicFolder(this));
        textView_currentDuration = findViewById(R.id.tv_current_duration);
        textView_allDuration = findViewById(R.id.tv_all_duration);
        seekBar_playDuration = findViewById(R.id.seekBar_playDuration);
        button_status = findViewById(R.id.btn_status);
        button_pitch = findViewById(R.id.btn_pitch);
        button_speed = findViewById(R.id.btn_speed);
        button_recordStatus = findViewById(R.id.btn_start_record);
        button_recordStop = findViewById(R.id.btn_stop_record);
        seekBar_volume = findViewById(R.id.seekBar_volume);
        progressBar_amplitude = findViewById(R.id.pb_amplitude);
        listView = findViewById(R.id.lv_music);
        button_audioMute = findViewById(R.id.btn_mute);

        mPlayer = new LeiAudioPlayer();
        mPlayer.setCallBack(this);


        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                mPlaySelection = position;
                playMusic();
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
                mPlayer.seek((int) (progress * allSec / 100.0f));

            }
        });

        seekBar_volume.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mPlayer.setVolume(seekBar.getProgress());

            }
        });

        init();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mPlayer != null)
            mPlayer.destroy();
    }

    private void init() {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                mMusicFileList = AudioFinder.findLocalMusic(AudioActivity.this);
                mMusicFileList.add(new MusicBean("在线", NET_SOURCE));
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

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_status:
                if (mPlayStatus == PLAY_STATUS_STOPED) {
                    if (mMusicFileList == null || mMusicFileList.isEmpty())
                        return;
                    mPlaySelection = 0;
                    playMusic();
                    button_status.setText("暂停");
                    return;
                }
                if (mPlayStatus == PLAY_STATUS_PLAYING) {
                    mPlayer.pause();
                    mPlayStatus = PLAY_STATUS_PAUSE;
                    button_status.setText("播放");
                    return;
                }
                if (mPlayStatus == PLAY_STATUS_PAUSE) {
                    mPlayer.resume();
                    mPlayStatus = PLAY_STATUS_PLAYING;
                    button_status.setText("暂停");
                    return;
                }
                break;

            case R.id.btn_mute:
                if (audioMute == LeiAudioPlayer.Mute.DOUBLE) {
                    audioMute = LeiAudioPlayer.Mute.LEFT;
                    button_audioMute.setText("左声道");
                } else if (audioMute == LeiAudioPlayer.Mute.LEFT) {
                    audioMute = LeiAudioPlayer.Mute.RIGHT;
                    button_audioMute.setText("右声道");
                } else if (audioMute == LeiAudioPlayer.Mute.RIGHT) {
                    audioMute = LeiAudioPlayer.Mute.DOUBLE;
                    button_audioMute.setText("立体声");
                }
                mPlayer.setMute(audioMute);
                break;
            case R.id.btn_pitch:
                pitch += 0.2f;
                if (pitch > 2.0f)
                    pitch = 1.0f;
                button_pitch.setText("音调" + formatFeeFloat(pitch) + "倍");
                mPlayer.setPitch(pitch);
                break;
            case R.id.btn_speed:
                speed += 0.2f;
                if (speed > 2.0f)
                    speed = 1.0f;
                button_speed.setText("音速" + formatFeeFloat(speed) + "倍");
                mPlayer.setSpeed(speed);
                break;
            case R.id.btn_start_record:
                if (mPlayer.isPauseRecording()) {
                    mPlayer.resumeRecordAAC();
                    button_recordStatus.setText("暂停录音");
                    return;
                }
                if (mPlayer.isOnRecording()) {
                    mPlayer.pauseRecordAAC();
                    button_recordStatus.setText("继续录音");
                    return;
                }
                if (mPlaySelection < 0)
                    return;
                String fileName = mMusicFileList.get(mPlaySelection).name;
                int index = fileName.lastIndexOf(".");
                if (index > 0)
                    fileName = fileName.substring(0, index);
                File saveDest = FileUtil.getRecordFile(this, fileName, ".aac", true);
                mPlayer.start2RecordAAC(saveDest);
                button_recordStatus.setText("暂停录音");
                break;
            case R.id.btn_stop_record:
                mPlayer.stopRecordAAC();
                button_recordStatus.setText("开始录音");
                break;
        }
    }

    private void playMusic() {
        button_recordStatus.setText("开始录音");
        mAdapter.notifyDataSetChanged();
        textView_currentDuration.postDelayed(new Runnable() {
            @Override
            public void run() {
                seekBar_playDuration.setProgress(0);
                mPlayer.setDataSource(mMusicFileList.get(mPlaySelection).path);
                mPlayer.prepared();
                mPlayer.setVolume(seekBar_volume.getProgress());
                mPlayer.start();
                mPlayStatus = PLAY_STATUS_PLAYING;
            }
        }, 1000);

    }

    @Override
    public void onDurationChanged(int current, int all) {
        currentSec = current;
        allSec = all;
        textView_currentDuration.setText(formatTime(current));
        textView_allDuration.setText(formatTime(all));
        int percent = (int) ((current * 1.0f / all) * 100);
        seekBar_playDuration.setProgress(percent);
    }

    @Override
    public void onAmplitudeChanged(int amplitude) {
        progressBar_amplitude.setProgress(amplitude);
    }

    @Override
    public void onPlayFinished() {
        if (mMusicFileList == null || mMusicFileList.isEmpty())
            return;
        mPlaySelection++;
        if (mPlaySelection >= mMusicFileList.size())
            mPlaySelection = 0;
        playMusic();
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

    private class MusicAdapter extends BaseAdapter {
        @Override
        public int getCount() {
            return (mMusicFileList == null ? 0 : mMusicFileList.size());
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
                textView = new TextView(AudioActivity.this);
                textView.setTextColor(colorMusicPath);
            }
            textView.setGravity(Gravity.CENTER);
            textView.setPadding(20, 20, 20, 20);
            String musicName = mMusicFileList.get(position).name;
            SpannableString spannableString = new SpannableString(musicName + "\n\n" + mMusicFileList.get(position).path);
            if (!TextUtils.isEmpty(musicName))
                spannableString.setSpan(new ForegroundColorSpan(colorMusicName), 0, musicName.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            textView.setText(spannableString);
            convertView = textView;
            convertView.setBackgroundColor((mPlaySelection == position) ? colorPlayStatus : Color.WHITE);
            return convertView;
        }
    }

    public static String formatFeeFloat(float oil) {
        DecimalFormat fnum = new DecimalFormat("##0.00");
        float dd = Float.parseFloat(fnum.format(oil));
        float n = oil - dd;
        if (n >= 0.01) {
            dd = dd + 0.01f;
        }
        return fnum.format(dd);
    }
}
