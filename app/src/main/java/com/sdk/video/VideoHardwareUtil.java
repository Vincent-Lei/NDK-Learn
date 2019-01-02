package com.sdk.video;

import android.media.MediaCodecList;

import com.lei.ndk.util.LogUtil;

import java.util.HashMap;

/**
 * Created by Vincent.Lei on 2018/12/28.
 * Title：
 * Note：
 */
public class VideoHardwareUtil {

    private static HashMap<String, String> codecMap = new HashMap<>();

    static {
        codecMap.put("h264", "video/avc");
        codecMap.put("h265", "video/hevc");
        codecMap.put("h263", "video/3gpp");
        codecMap.put("mpeg4", "video/mp4v-es");
        codecMap.put("vp8", "video/x-vnd.on2.vp8");
        codecMap.put("vp9", "video/x-vnd.on2.vp9");
//        codecMap.put("rv40", "video/avc");
    }

    public static String findVideoCodecName(String ffmpegCodecName) {
        return codecMap.get(ffmpegCodecName);
    }

    public static void printHardwareCodecSupport() {
        String[] types;
        for (int i = 0, count = MediaCodecList.getCodecCount(); i < count; i++) {
            types = MediaCodecList.getCodecInfoAt(i).getSupportedTypes();
            for (int j = 0; j < types.length; j++) {
                if (types[j].startsWith("video"))
                    LogUtil.d(types[j]);
            }
        }
    }

    public static boolean isHardwareCodecSupport(String ffmpegCodecName) {
        if (ffmpegCodecName == null)
            return false;
        ffmpegCodecName = ffmpegCodecName.toLowerCase();
        LogUtil.d("ffmpegCodecName = " + ffmpegCodecName);
        String supportName = codecMap.get(ffmpegCodecName);
        if (supportName == null)
            return false;
        String[] types;
        for (int i = 0, count = MediaCodecList.getCodecCount(); i < count; i++) {
            types = MediaCodecList.getCodecInfoAt(i).getSupportedTypes();
            for (int j = 0; j < types.length; j++) {
                if (supportName.equals(types[j]))
                    return true;
            }
        }

        return false;
    }
}
