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
import android.widget.SeekBar;
import android.widget.TextView;

import com.lei.ndk.R;
import com.lei.ndk.util.FileUtil;

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
    TextView textView_playDuration;
    TextView textView_playTitle;
    Button button_audioMute;
    Button button_status;
    Button button_pitch;
    Button button_speed;
    ListView listView;
    SeekBar seekBar_playDuration;
    SeekBar seekBar_volume;
    SeekBar seekBar_amplitude;
    int currentSec;
    int allSec;
    List<MusicBean> mMusicFileList;
    MusicAdapter mAdapter;
    int mPlaySelection;
    int mPlayStatus;
    int colorMusicName;
    int colorMusicPath;
    int audioMute = LeiAudioPlayer.Mute.DOUBLE;
    float pitch = 1.0f, speed = 1.0f;
   
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_test);
        TextView textView = findViewById(R.id.tv_music_folder);
        textView.setText("请将音乐文件放在" + FileUtil.getMusicFolder(this));
        textView_playDuration = findViewById(R.id.tv_playDuration);
        seekBar_playDuration = findViewById(R.id.seekBar_playDuration);
        textView_playTitle = findViewById(R.id.tv_play_title);
        button_status = findViewById(R.id.btn_status);
        button_pitch = findViewById(R.id.btn_pitch);
        button_speed = findViewById(R.id.btn_speed);
        seekBar_volume = findViewById(R.id.seekBar_volume);
        seekBar_amplitude = findViewById(R.id.seekBar_amplitude);
        seekBar_amplitude.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                return true;
            }
        });
        listView = findViewById(R.id.lv_music);
        button_audioMute = findViewById(R.id.btn_mute);

        mPlayer = new LeiAudioPlayer();
        mPlayer.setCallBack(this);
        colorMusicName = getResources().getColor(R.color.colorAccent);
        colorMusicPath = Color.rgb(100, 100, 100);
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
            case R.id.btn_init:
                init();
                break;
            case R.id.btn_status:
                if (mPlayStatus == PLAY_STATUS_STOPED) {
                    if (mMusicFileList == null || mMusicFileList.isEmpty())
                        return;
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
                pitch += 0.5f;
                if (pitch > 3.0f)
                    pitch = 1.0f;
                button_pitch.setText("音调" + pitch + "倍");
                mPlayer.setPitch(pitch);
                break;
            case R.id.btn_speed:
                speed += 0.5f;
                if (speed > 3.0f)
                    speed = 1.0f;
                button_speed.setText("音速" + speed + "倍");
                mPlayer.setSpeed(speed);
                break;
        }
    }

    private void playMusic() {
        textView_playTitle.postDelayed(new Runnable() {
            @Override
            public void run() {
                seekBar_playDuration.setProgress(0);
                mPlayer.setDataSource(mMusicFileList.get(mPlaySelection).path);
                textView_playTitle.setText(mMusicFileList.get(mPlaySelection).name);
                mPlayer.prepared();
                mPlayer.setVolume(seekBar_volume.getProgress());
                mPlayer.start();
                mPlayStatus = PLAY_STATUS_PLAYING;
            }
        }, 2000);

    }

    @Override
    public void onDurationChanged(int current, int all) {
        currentSec = current;
        allSec = all;
        textView_playDuration.setText("进度：" + formatTime(current) + "/" + formatTime(all));
        int percent = (int) ((current * 1.0f / all) * 100);
        seekBar_playDuration.setProgress(percent);
    }

    @Override
    public void onAmplitudeChanged(int amplitude) {
        seekBar_amplitude.setProgress(amplitude);
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
            return convertView;
        }
    }
}
