package com.lei.ndk.pthread;

/**
 * Created by Vincent.Lei on 2019/4/17.
 * Title：
 * Note：
 */
public class PthreadTest {
    public static void testMutex1() {
        new Thread(new Runnable() {
            @Override
            public void run() {
//                testMutex();
//                testRWLock();
//                testThreadCallBack();
                testThreadRelease();
            }
        }).start();
    }


    private native static void testMutex();

    private native static void testRWLock();

    private native static void testThreadCallBack();

    private native static void testThreadRelease();
}
