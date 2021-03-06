package com.sdk.video;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.Surface;

import com.lei.ndk.R;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by Vincent.Lei on 2018/12/24.
 * Title：
 * Note：
 */
public class VideoGLSurfaceView extends GLSurfaceView implements SurfaceTexture.OnFrameAvailableListener {
    public static final int RENDER_SOFTWARE = 0;
    public static final int RENDER_HARDWARE = 1;
    private VideoRender mRender;


    public VideoGLSurfaceView(Context context) {
        this(context, null);
    }

    public VideoGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        mRender = new VideoRender(context);
        setRenderer(mRender);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        mRender.setFrameAvailableListener(this);
    }

    public void setYUVData(int width, int height, byte[] y, byte[] u, byte[] v) {
        setRenderType(RENDER_SOFTWARE);
        if (mRender != null) {
            mRender.setYUVRenderData(width, height, y, u, v);
            requestRender();
        }
    }

    public Surface getSurface() {
        return mRender.surface;
    }

    public void setRenderType(int type) {
        mRender.mRenderType = type;
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        requestRender();
    }

    private static class VideoRender implements GLSurfaceView.Renderer {

        private Context context;
        private int mRenderType = RENDER_SOFTWARE;
        private final float[] vertexData = {

                -1f, -1f,
                1f, -1f,
                -1f, 1f,
                1f, 1f

        };

        private final float[] textureData = {
                0f, 1f,
                1f, 1f,
                0f, 0f,
                1f, 0f
        };

        private FloatBuffer vertexBuffer;
        private FloatBuffer textureBuffer;
        private int program_yuv;
        private int avPosition_yuv;
        private int afPosition_yuv;

        private int sampler_y;
        private int sampler_u;
        private int sampler_v;
        private int[] textureId_yuv;

        private int width_yuv;
        private int height_yuv;
        private ByteBuffer y;
        private ByteBuffer u;
        private ByteBuffer v;

        //hardware
        private int program_mediacodec;
        private int avPosition_mediacodec;
        private int afPosition_mediacodec;
        private int samplerOES_mediacodec;
        private int textureId_mediacodec;
        private SurfaceTexture surfaceTexture;
        private Surface surface;

        public void setFrameAvailableListener(SurfaceTexture.OnFrameAvailableListener frameAvailableListener) {
            this.frameAvailableListener = frameAvailableListener;
        }

        private SurfaceTexture.OnFrameAvailableListener frameAvailableListener;

        private VideoRender(Context context) {
            this.context = context;
            vertexBuffer = ByteBuffer.allocateDirect(vertexData.length * 4)
                    .order(ByteOrder.nativeOrder())
                    .asFloatBuffer()
                    .put(vertexData);
            vertexBuffer.position(0);

            textureBuffer = ByteBuffer.allocateDirect(textureData.length * 4)
                    .order(ByteOrder.nativeOrder())
                    .asFloatBuffer()
                    .put(textureData);
            textureBuffer.position(0);
        }

        private void setYUVRenderData(int width, int height, byte[] y, byte[] u, byte[] v) {
            this.width_yuv = width;
            this.height_yuv = height;
            this.y = ByteBuffer.wrap(y);
            this.u = ByteBuffer.wrap(u);
            this.v = ByteBuffer.wrap(v);
        }

        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            initSoftwareRender();
            initHardwareRender();
        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            GLES20.glViewport(0, 0, width, height);
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
            GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            if (mRenderType == RENDER_HARDWARE)
                renderHardware();
            else
                renderSoftware();

            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
        }

        private void renderSoftware() {
            if (width_yuv > 0 && height_yuv > 0 && y != null && u != null && v != null) {
                GLES20.glUseProgram(program_yuv);

                GLES20.glEnableVertexAttribArray(avPosition_yuv);
                GLES20.glVertexAttribPointer(avPosition_yuv, 2, GLES20.GL_FLOAT, false, 8, vertexBuffer);

                GLES20.glEnableVertexAttribArray(afPosition_yuv);
                GLES20.glVertexAttribPointer(afPosition_yuv, 2, GLES20.GL_FLOAT, false, 8, textureBuffer);

                GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
                GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[0]);
                GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width_yuv, height_yuv, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, y);

                GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
                GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[1]);
                GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width_yuv / 2, height_yuv / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, u);

                GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
                GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[2]);
                GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width_yuv / 2, height_yuv / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, v);

                GLES20.glUniform1i(sampler_y, 0);
                GLES20.glUniform1i(sampler_u, 1);
                GLES20.glUniform1i(sampler_v, 2);

                y.clear();
                u.clear();
                v.clear();
                y = null;
                u = null;
                v = null;

            }
        }

        private void renderHardware() {
            surfaceTexture.updateTexImage();
            GLES20.glUseProgram(program_mediacodec);

            GLES20.glEnableVertexAttribArray(avPosition_mediacodec);
            GLES20.glVertexAttribPointer(avPosition_mediacodec, 2, GLES20.GL_FLOAT, false, 8, vertexBuffer);

            GLES20.glEnableVertexAttribArray(afPosition_mediacodec);
            GLES20.glVertexAttribPointer(afPosition_mediacodec, 2, GLES20.GL_FLOAT, false, 8, textureBuffer);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId_mediacodec);
            GLES20.glUniform1i(samplerOES_mediacodec, 0);
        }

        private void initHardwareRender() {
            String vertexSource = GLProgramCreater.readRawTxt(context, R.raw.hareware_vertex_shader);
            String fragmentSource = GLProgramCreater.readRawTxt(context, R.raw.fragment_hareware);
            program_mediacodec = GLProgramCreater.createProgram(vertexSource, fragmentSource);

            avPosition_mediacodec = GLES20.glGetAttribLocation(program_mediacodec, "av_Position");
            afPosition_mediacodec = GLES20.glGetAttribLocation(program_mediacodec, "af_Position");
            samplerOES_mediacodec = GLES20.glGetUniformLocation(program_mediacodec, "sTexture");

            int[] textureids = new int[1];
            GLES20.glGenTextures(1, textureids, 0);
            textureId_mediacodec = textureids[0];

            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);

            surfaceTexture = new SurfaceTexture(textureId_mediacodec);
            surface = new Surface(surfaceTexture);
            surfaceTexture.setOnFrameAvailableListener(frameAvailableListener);
        }

        private void initSoftwareRender() {
            String vertexSource = GLProgramCreater.readRawTxt(context, R.raw.video_vertex_shader);
            String fragmentSource = GLProgramCreater.readRawTxt(context, R.raw.video_fragment_shader);
            program_yuv = GLProgramCreater.createProgram(vertexSource, fragmentSource);

            avPosition_yuv = GLES20.glGetAttribLocation(program_yuv, "av_Position");
            afPosition_yuv = GLES20.glGetAttribLocation(program_yuv, "af_Position");

            sampler_y = GLES20.glGetUniformLocation(program_yuv, "sampler_y");
            sampler_u = GLES20.glGetUniformLocation(program_yuv, "sampler_u");
            sampler_v = GLES20.glGetUniformLocation(program_yuv, "sampler_v");

            textureId_yuv = new int[3];
            GLES20.glGenTextures(3, textureId_yuv, 0);

            for (int i = 0; i < 3; i++) {
                GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[i]);
                GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
                GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
                GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
                GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            }
        }
    }

}
