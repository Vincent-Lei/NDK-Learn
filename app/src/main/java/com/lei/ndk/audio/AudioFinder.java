package com.lei.ndk.audio;

import android.content.Context;
import android.database.Cursor;
import android.provider.MediaStore;

import com.lei.ndk.util.FileUtil;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * Created by Vincent.Lei on 2018/11/29.
 * Title：
 * Note：
 */
public class AudioFinder {


    public static List<MusicBean> findLocalMusic(Context context) {
        FileUtil.writeLocalAssetFileToMusic(context, "See_You_Again.mp3");
        List<MusicBean> musicFileList = new ArrayList<>();
        String path = FileUtil.getMusicFolder(context);
        File file = new File(path);
        if (!file.exists())
            return musicFileList;
        File[] fileList = file.listFiles();
        for (int i = 0; i < fileList.length; i++) {
            if (!fileList[i].isFile() || !isFileIsMusic(fileList[i]))
                continue;
            String name = fileList[i].getName();
            if (name != null && name.endsWith(".ape"))
                continue;
            musicFileList.add(new MusicBean(name, fileList[i].getAbsolutePath()));
        }

//        Cursor cursor = context.getContentResolver().query(
//                MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
//                new String[]{MediaStore.Audio.Media._ID,
//                        MediaStore.Audio.Media.DISPLAY_NAME,
//                        MediaStore.Audio.Media.TITLE,
//                        MediaStore.Audio.Media.DURATION,
//                        MediaStore.Audio.Media.ARTIST,
//                        MediaStore.Audio.Media.ALBUM,
//                        MediaStore.Audio.Media.YEAR,
//                        MediaStore.Audio.Media.MIME_TYPE,
//                        MediaStore.Audio.Media.SIZE,
//                        MediaStore.Audio.Media.DATA},
//                null,
//                null, null);
//        if (cursor != null && cursor.getCount() > 0) {
//            while (cursor.moveToNext()) {
//                String name = cursor.getString(cursor.getColumnIndex(MediaStore.Audio.Media.DISPLAY_NAME));
//                if (name != null && name.endsWith(".ape"))
//                    continue;
//                musicFileList.add(new MusicBean(name, cursor.getString(cursor.getColumnIndex(MediaStore.Audio.Media.DATA))));
//            }
//            cursor.close();
//        }


        return musicFileList;
    }

    private static boolean isFileIsMusic(File file) {
        if (file != null && file.exists() && file.getName() != null) {
            String name = file.getName();
            return name.endsWith(".mp3") || name.endsWith(".ape") || name.endsWith(".wav");
        }
        return false;
    }
}
