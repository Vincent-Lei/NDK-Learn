package com.lei.ndk.util;

import android.content.Context;
import android.os.Environment;
import android.text.TextUtils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * Created by Vincent.Lei on 2018/11/22.
 * Title：
 * Note：
 */
public class FileUtil {
    public static final String MUSIC_FOLDER = "Music";
    public static final String VIDEO_FOLDER = "Movies";

    public static String getMusicFolder(Context context) {
        String path = FileUtil.getRootDir() + File.separator + MUSIC_FOLDER;
        File file = new File(path);
        if (!file.exists())
            file.mkdirs();
        return path;
    }

    public static String getVideoFolder(Context context) {
        String path = FileUtil.getRootDir() + File.separator + VIDEO_FOLDER;
        File file = new File(path);
        if (!file.exists())
            file.mkdirs();
        return path;
    }

    public static File writeLocalAssetFileToMusic(Context context, String assetName) {
        File file = new File(getMusicFolder(context) + File.separator + assetName);
        if (file.exists())
            return file;
        try {
            InputStream inputStream = context.getAssets().open(assetName);
            writeFileToLocal(inputStream, file);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return file;
    }

    public static File writeLocalAssetFileToVideo(Context context, String assetName) {
        File file = new File(getVideoFolder(context) + File.separator + assetName);
        if (file.exists())
            return file;
        try {
            InputStream inputStream = context.getAssets().open(assetName);
            writeFileToLocal(inputStream, file);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return file;
    }

    public static String getRootDir() {
        File file = new File(Environment.getExternalStorageDirectory().getPath());
        if (!file.exists())
            file.mkdirs();
        return file.getAbsolutePath();
    }

    public static File getRecordFile(Context context, String name, String suffixName, boolean isAutoCreate) {
        File file = new File(getRootDir() + File.separator + context.getPackageName());
        if (!file.exists())
            file.mkdirs();
        if (TextUtils.isEmpty(name))
            name = String.valueOf(System.currentTimeMillis());
        file = new File(file.getAbsolutePath() + File.separator + name + suffixName);
        if (!file.exists() && isAutoCreate) {
            try {
                file.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return file;
    }

    private static void writeFileToLocal(InputStream inputStream, File dest) {
        FileOutputStream fos = null;
        byte[] buff = new byte[1024];
        int len;
        LogUtil.d("write file " + dest.getAbsolutePath());
        try {
            while ((len = inputStream.read(buff)) > 0) {
                if (!dest.exists())
                    dest.createNewFile();
                if (fos == null)
                    fos = new FileOutputStream(dest);
                fos.write(buff, 0, len);
            }
            fos.flush();
        } catch (Exception e) {
            if (dest.exists())
                dest.delete();
        } finally {
            if (inputStream != null) {
                try {
                    inputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (fos != null) {
                try {
                    fos.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
