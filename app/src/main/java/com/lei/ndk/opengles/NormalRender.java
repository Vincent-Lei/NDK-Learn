package com.lei.ndk.opengles;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

import com.sdk.video.GLProgramCreater;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by Vincent.Lei on 2018/12/21.
 * Title：
 * Note：
 */
public class NormalRender implements GLSurfaceView.Renderer {

//    float[] vertexData = {
//                -1.0f, 0.0f,//三角形左下角
//                0.0f, 1.0f,//三角形右下角
//                1.0f, 0.0f//三角形顶点
//        };

    float[] vertexData = {
            -1f, 0f,
            0f, -1f,
//            0f, 1f,
            1f, 0f
    };

    FloatBuffer vertexBuffer;
    int program;
    int avPosition;
    int afColor;

    public NormalRender() {
        //本地化三角形顶点

        //所谓本地化就是跳出java VM（Java虚拟机）的约束（垃圾回收）范围，
        // 使我们的顶点在程序运行时一直都有自己分配的内存地址，不会因为java的GC而把顶点内存地址给回收掉，导致顶点不存在，
        // 从而引起OpenGL找不到顶点位置等错误，所以在OpenGL中我们需要把顶点坐标给本地化。

        //用float数组来存储我们的顶点坐标，因为顶点坐标范围是在（-1f~1f）之间的所有小数都可以，所以我们先创建顶点数组
        //然后根据顶点数组分配底层内存地址，因为需要本地化，所以就和c/c++一样需要我们手动分配内存地址，这里用到了ByteBuffer

        vertexBuffer = ByteBuffer.allocateDirect(vertexData.length * 4)//分配内存空间（单位字节,float占4个字节）
                .order(ByteOrder.nativeOrder())//内存bit的排序方式和本地机器一致
                .asFloatBuffer()//转换成float的buffer,因为我们是放float类型的顶点
                .put(vertexData);//把数据放入内存中
        vertexBuffer.position(0);//把索引指针指向开头位置
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        //OpenGL的操作需要我们自己编写着色器（shader）程序给它，然后它会用GPU执行这个着色器程序，
        // 最终反馈执行结果给我们。我们用glsl语言来编写着色器程序，其语法方式和c语言类似

        //开始顶点着色器的编写（shader）
        //av_Position 用于在java代码中获取的属性
        //gl_Position是内置变量，opengl绘制顶点就是根据它的值绘制的，所以我们需要把我们自己的值赋值给它
        //attribute vec4 av_Position这句的意思：
        //attribute是表示顶点属性的，只能用在顶点坐标里面，然后在应用程序（java代码）中可以获取其变量，然后为其赋值。
        // vec4是一个包含4个值（x,y,z,w）的向量，x和y表示2d平面，加上z就是3d的图像了，最后的w是摄像机的距离，
        // 因为我们绘制的是2d图形，所以最后z和w的值可以不用管，OpenGL会有默认值1。
        // 所以这句话的意思就是：声明了一个名字叫av_Position的包含4个向量的attribute类型的变量，
        // 用于我们在java代码中获取并把我们的顶点（FloatBuffer vertexBuffer）值赋值给它。
        // 这样OpenGL执行这段着色器代码（程序）时，就有了具体的顶点数据，就会在相应的顶点之间绘制图形（我们定义的三角形）了。
        String vertexSource = "attribute vec4 av_Position;\n" +
                "void main(){" +
                "\ngl_Position = av_Position;" +
                "\n}";

        //片元着色器程序编写（shader）
        // precision mediump float 声明用中等精度的float
        //af_Color 用于在java层传递颜色数据
        //gl_FragColor内置变量，opengl渲染的颜色就是获取的它的值，这里我们把我们自己的值赋值给它。
        //用了uniform这个类型来声明变量，uniform是用于应用程序（java代码中）向顶点和片元着色器传递数据，
        // 和attribute的区别在于，attribute是只能用在顶点着色器程序中，并且它里面包含的是具体的顶点的数据，
        // 每次执行时都需要从顶点内存里面获取新的值，而uniform始终都是用同一个变量。
        // vec4 af_Color也是声明一个4个分量的变量af_Color，这个里面保存的是颜色的值了（rgba四个分量)

        String fragmentSource = "precision mediump float;\n" +
                "uniform vec4 af_Color;\n" +
                "void main(){\n" +
                "gl_FragColor = af_Color;\n" +
                "}";
        program = GLProgramCreater.createProgram(vertexSource, fragmentSource);
        ////获取顶点属性，后面会给它赋值（即：把我们的顶点赋值给它）
        avPosition = GLES20.glGetAttribLocation(program, "av_Position");
        //获取片元变量，后面可以通过它设置片元要显示的颜色。
        afColor = GLES20.glGetUniformLocation(program, "af_Color");
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES20.glViewport(0, 0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//用来清屏
        //开始绘制之前，先设置使用当前programId这个程序。
        GLES20.glUseProgram(program);
        //分别设置片元变量的rgba四个值（前面的glUniform4f:表示这是uniform类型的变量的4个float类型的值）
        GLES20.glUniform4f(afColor, 1, 0, 0, 1);
        //激活顶点属性数组，激活后才能对它赋值
        GLES20.glEnableVertexAttribArray(avPosition);
        //现在就是把我们的顶点vertexBuffer赋值给顶点着色器里面的变量。
        /**
         * 第一参数就是我们的顶点属性的句柄
         *
         * 第二个参数是我们用的几个分量表示的一个点，这里用的（x,y）2个分量，所以就填入2
         *
         * 第三个参数表示顶点的数据类型，因为我们用的float类型，所以就填入GL_FLOAT类型
         *
         * 第四个参数是是否做归一化处理，如果我们的坐标不在（-1,1）之间，就需要，由于我们的坐标是在（-1,1）之间，所以不需要，填入false
         *
         * 第五个参数是每个点所占空间大小，因为是（x，y）2个点，每个点是4个字节，所以一个点占8个空间大小，这个设置好后，OpenGL才知道8个字节表示一个点，就能按照这个规则，依次取出所有的点的值。
         *
         * 第六个参数就是OpenGL要从哪个内存中取出这些点的数据。
         */
        GLES20.glVertexAttribPointer(avPosition, 2, GLES20.GL_FLOAT, false, 8, vertexBuffer);
        ////绘制三角形，从我们的顶点数组里面第0个位置开始，绘制顶点的个数为3（因为三角形只有三个顶点）
            GLES20.glDrawArrays(GLES20.GL_TRIANGLES, 0, 3);
        //绘制四边形
//        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
    }
}
