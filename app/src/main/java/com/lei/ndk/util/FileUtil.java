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
    public static File getLocalAssetFile(Context context, String assetName) {
        File file = getFileByName(null, assetName);
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


    private static File getFileByName(String subPath, String fileName) {
        String path = getRootDir() + File.separator + (TextUtils.isEmpty(subPath) ? "cache" : subPath);
        File file = new File(path);
        if (!file.exists())
            file.mkdirs();
        file = new File(path + File.separator + fileName);
        return file;
    }

    private static String getRootDir() {
        File file = new File(Environment.getExternalStorageDirectory().getPath());
        if (!file.exists())
            file.mkdirs();
        return file.getAbsolutePath();
    }

    private static void writeFileToLocal(InputStream inputStream, File dest) {
        FileOutputStream fos = null;
        byte[] buff = new byte[1024];
        int len;
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
