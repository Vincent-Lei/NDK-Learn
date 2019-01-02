package com.sdk.video;

import android.media.MediaCodec;
import android.media.MediaFormat;
import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.view.Surface;

import com.lei.ndk.util.LogUtil;

import java.nio.ByteBuffer;


/**
 * Created by Vincent.Lei on 2018/12/24.
 * Title：
 * Note：
 */
public class VideoPlayer {
    private static final Handler mHandler = new Handler(Looper.getMainLooper());

    public interface IVideoCallBack {
        void onPreparedFinished(boolean success, int duration, int width, int height);

        void onUpdateTimeClock(int currentClock);
    }

    private long mNativePtr;
    private String mDataSource;
    private VideoGLSurfaceView videoGLSurfaceView;
    private MediaFormat mediaFormat;
    private MediaCodec mediaCodec;
    private MediaCodec.BufferInfo info;
    private IVideoCallBack callBack;

    public void setVideoCallBack(IVideoCallBack callBack) {
        this.callBack = callBack;
    }

    public VideoPlayer() {
        mNativePtr = nativeInit();
    }

    public void prepare(String dataSource) {
        this.mDataSource = dataSource;
        if (!TextUtils.isEmpty(mDataSource)) {
            nativePrepare(mNativePtr, dataSource);
        }
    }

    public void start() {
        if (TextUtils.isEmpty(mDataSource)) {
            LogUtil.d("dataSource is null");
            return;
        }
        nativeStart(mNativePtr);
    }

    public void seek(int second) {
        nativeSeek(mNativePtr, second);
    }

    public void destory() {
        nativeDestory(mNativePtr);
        releaseMediaCodec();
        videoGLSurfaceView = null;
        mNativePtr = 0;
    }

    public void setVideoGLSurfaceView(VideoGLSurfaceView videoGLSurfaceView) {
        this.videoGLSurfaceView = videoGLSurfaceView;
    }

    public void nativeCallPreparedFinished(final boolean success, final int duration, final int width, final int height) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (callBack != null)
                    callBack.onPreparedFinished(success, duration, width, height);
            }
        });
    }

    public void nativeCallUpdateTimeClock(final int currentClock) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (callBack != null)
                    callBack.onUpdateTimeClock(currentClock);
            }
        });
    }

    private void nativeCallRenderYUV(int width, int height, byte[] y, byte[] u, byte[] v) {
        if (videoGLSurfaceView != null)
            videoGLSurfaceView.setYUVData(width, height, y, u, v);
    }

    private boolean nativeCheckIsHardwareCodecEnable(String codec) {
        return VideoHardwareUtil.isHardwareCodecSupport(codec);
    }

    private void nativeCallInitHardwareCodec(String codecName, int width, int height, byte[] csd_0, byte[] csd_1) {
        releaseMediaCodec();
        if (videoGLSurfaceView != null) {
            try {
                videoGLSurfaceView.setRenderType(VideoGLSurfaceView.RENDER_HARDWARE);
                String mime = VideoHardwareUtil.findVideoCodecName(codecName);
                mediaFormat = MediaFormat.createVideoFormat(mime, width, height);
                mediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, width * height);
                mediaFormat.setByteBuffer("csd-0", ByteBuffer.wrap(csd_0));
                mediaFormat.setByteBuffer("csd-1", ByteBuffer.wrap(csd_1));
                LogUtil.d(mediaFormat.toString());
                mediaCodec = MediaCodec.createDecoderByType(mime);

                info = new MediaCodec.BufferInfo();
                mediaCodec.configure(mediaFormat, videoGLSurfaceView.getSurface(), null, 0);
                mediaCodec.start();
                LogUtil.d("nativeCallInitHardwareCodec success");

            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private void nativeCallHarewareDecodeAVPacket(int dataSize, byte[] data) {
        if (videoGLSurfaceView != null && dataSize > 0 && data != null && mediaCodec != null) {
            int intputBufferIndex = mediaCodec.dequeueInputBuffer(10);
            if (intputBufferIndex >= 0) {
                ByteBuffer byteBuffer = mediaCodec.getInputBuffers()[intputBufferIndex];
                byteBuffer.clear();
                byteBuffer.put(data);
                mediaCodec.queueInputBuffer(intputBufferIndex, 0, dataSize, 0, 0);
            }
            int outputBufferIndex = mediaCodec.dequeueOutputBuffer(info, 10);
            while (outputBufferIndex >= 0) {
                mediaCodec.releaseOutputBuffer(outputBufferIndex, true);
                outputBufferIndex = mediaCodec.dequeueOutputBuffer(info, 10);
            }
        }
    }

    private void releaseMediaCodec() {
        if (mediaCodec != null) {
            LogUtil.d("--releaseMediaCodec--");
            mediaCodec.flush();
            mediaCodec.stop();
            mediaCodec.release();
            mediaCodec = null;
            mediaFormat = null;
            info = null;
        }
    }


    private native long nativeInit();

    private native void nativePrepare(long ptr, String dataSource);

    private native void nativeStart(long ptr);

    private native void nativeSeek(long ptr, int second);

    private native void nativeDestory(long ptr);
}
