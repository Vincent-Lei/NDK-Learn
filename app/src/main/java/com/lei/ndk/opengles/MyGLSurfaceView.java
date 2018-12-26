package com.lei.ndk.opengles;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

import com.lei.ndk.R;

/**
 * Created by Vincent.Lei on 2018/12/6.
 * Title：
 * Note：
 */
public class MyGLSurfaceView extends GLSurfaceView {

    public MyGLSurfaceView(Context context) {
        this(context, null);
    }

    public MyGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);//设置opengl es版本为2.0
//        setRenderer(new NormalRender());//为glsurfaceview设置render
        setRenderer(new TextureRender(context, R.mipmap.activity_img_baza_glass));//为glsurfaceview设置render
    }
}
