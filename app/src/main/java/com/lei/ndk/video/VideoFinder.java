package com.lei.ndk.video;

import android.content.Context;

import com.lei.ndk.audio.MusicBean;
import com.lei.ndk.util.FileUtil;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * Created by Vincent.Lei on 2018/11/29.
 * Title：
 * Note：
 */
public class VideoFinder {


    public static List<VideoBean> findLocalVideo(Context context) {
        FileUtil.writeLocalAssetFileToVideo(context, "video_1.mp4");
        List<VideoBean> videoFileList = new ArrayList<>();
        String path = FileUtil.getVideoFolder(context);
        File file = new File(path);
        if (!file.exists())
            return videoFileList;
        File[] fileList = file.listFiles();
        for (int i = 0; i < fileList.length; i++) {
            if (!fileList[i].isFile() || !isFileIsVideo(fileList[i]))
                continue;
            String name = fileList[i].getName();
            videoFileList.add(new VideoBean(name, fileList[i].getAbsolutePath()));
        }
        return videoFileList;
    }

    private static boolean isFileIsVideo(File file) {
        if (file != null && file.exists() && file.getName() != null) {
            String name = file.getName();
            return name.endsWith(".mp4") || name.endsWith(".avi") || name.endsWith(".rmvb");
        }
        return false;
    }
}
