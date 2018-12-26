package com.sdk.video;

import android.content.Context;
import android.opengl.GLES20;

import com.lei.ndk.util.LogUtil;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;

/**
 * Created by Vincent.Lei on 2018/12/21.
 * Title：
 * Note：
 */
public class GLProgramCreater {

    public static String readRawTxt(Context context, int rawId) {
        InputStream inputStream = context.getResources().openRawResource(rawId);
        BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream));
        StringBuffer sb = new StringBuffer();
        String line;
        try
        {
            while((line = reader.readLine()) != null)
            {
                sb.append(line).append("\n");
            }
            reader.close();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        return sb.toString();
    }

    public static int loadShader(int shaderType, String source) {
        //通过GLES20.glCreateShader(shaderType)创建（顶点或片元）类型的代码程序
        int shader = GLES20.glCreateShader(shaderType);
        if (shader != 0) {
            //加载shader源码并编译shader
            GLES20.glShaderSource(shader, source);//这里根据我们创建的类型加载相应类型的着色器（如：顶点类型）
            GLES20.glCompileShader(shader);//编译我们自己写的着色器代码程序
            int[] compile = new int[1];
            GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compile, 0);
            if (compile[0] != GLES20.GL_TRUE) {
                LogUtil.e("shader compile error");
                GLES20.glDeleteShader(shader);
                shader = 0;
            }
        }
        return shader;
    }

    public static int createProgram(String vertexSource, String fragmentSource) {
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, vertexSource);
        if (vertexShader == 0) {
            return 0;
        }
        int fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER, fragmentSource);
        if (fragmentShader == 0) {
            return 0;
        }
        ////创建一个program程序
        int program = GLES20.glCreateProgram();
        if (program != 0) {
            ////把顶点着色器加入program程序中
            GLES20.glAttachShader(program, vertexShader);
            ////把片元着色器加入program程序中
            GLES20.glAttachShader(program, fragmentShader);
            ////最终链接顶点和片元着色器，后面在program中就可以访问顶点和片元着色器里面的属性了。
            GLES20.glLinkProgram(program);
            int[] linsStatus = new int[1];
            GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS, linsStatus, 0);
            if (linsStatus[0] != GLES20.GL_TRUE) {
                LogUtil.e("link program error");
                GLES20.glDeleteProgram(program);
                program = 0;
            }
        }
        return program;

    }

}
