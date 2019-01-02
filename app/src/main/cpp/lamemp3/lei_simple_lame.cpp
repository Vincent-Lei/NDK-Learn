#include "jni.h"
#include <cwchar>
#include "lame.h"
#include "native-log.h"

extern "C"
JNIEXPORT jlong JNICALL
Java_com_sdk_audio_lame_SimpleLame_native_1create(JNIEnv *env, jobject instance) {
    lame_global_flags *glf = lame_init();
    return (jlong) (glf);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sdk_audio_lame_SimpleLame_native_1init(JNIEnv *env, jobject instance, jlong ptr,
                                                jint inSampleRate,
                                                jint outChannel, jint outSampleRate,
                                                jint outBitrate,
                                                jint quality) {

    lame_global_flags *glf = (lame_global_flags *) (ptr);
    if (glf) {
        lame_set_in_samplerate(glf, inSampleRate);
        lame_set_num_channels(glf, outChannel);
        lame_set_out_samplerate(glf, outSampleRate);
        lame_set_brate(glf, outBitrate);
        lame_set_quality(glf, quality);
        lame_init_params(glf);
    }

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_sdk_audio_lame_SimpleLame_native_1flush(JNIEnv *env, jobject instance, jlong ptr,
                                                 jbyteArray mp3buf_) {
    jbyte *mp3buf = env->GetByteArrayElements(mp3buf_, NULL);
    lame_global_flags *glf = (lame_global_flags *) (ptr);
    int result = -1;
    if (glf) {
        const jsize mp3buf_size = env->GetArrayLength(mp3buf_);
        result = lame_encode_flush(glf, (u_char *) mp3buf, mp3buf_size);
    }
    env->ReleaseByteArrayElements(mp3buf_, mp3buf, 0);
    return result;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sdk_audio_lame_SimpleLame_native_1close(JNIEnv *env, jobject instance, jlong ptr) {
    lame_global_flags *glf = (lame_global_flags *) (ptr);
    if (glf)
        lame_close(glf);

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_sdk_audio_lame_SimpleLame_native_1encode(JNIEnv *env, jobject instance, jlong ptr,
                                                  jshortArray buffer_l_,
                                                  jshortArray buffer_r_, jint samples,
                                                  jbyteArray mp3buf_) {
    jshort *buffer_l = env->GetShortArrayElements(buffer_l_, NULL);
    jshort *buffer_r = env->GetShortArrayElements(buffer_r_, NULL);
    jbyte *mp3buf = env->GetByteArrayElements(mp3buf_, NULL);
    lame_global_flags *glf = (lame_global_flags *) (ptr);
    int result = -1;
    if (glf) {
        const jsize mp3buf_size = env->GetArrayLength(mp3buf_);
        result = lame_encode_buffer(glf, buffer_l, buffer_r, samples, (u_char *) mp3buf,
                                    mp3buf_size);
    }

    env->ReleaseShortArrayElements(buffer_l_, buffer_l, 0);
    env->ReleaseShortArrayElements(buffer_r_, buffer_r, 0);
    env->ReleaseByteArrayElements(mp3buf_, mp3buf, 0);
    return result;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_sdk_audio_lame_SimpleLame_native_1pcmToMp3(JNIEnv *env, jclass type, jint inSampleRate,
                                                    jint outChannel, jint outSampleRate,
                                                    jint outBitrate, jint quality,
                                                    jstring fileSource_, jstring fileDest_) {
    const char *fileSource = env->GetStringUTFChars(fileSource_, 0);
    const char *fileDest = env->GetStringUTFChars(fileDest_, 0);

    lame_global_flags *glf = lame_init();
    if (glf) {
        lame_set_in_samplerate(glf, inSampleRate);
        lame_set_num_channels(glf, outChannel);
        lame_set_out_samplerate(glf, outSampleRate);
        lame_set_brate(glf, outBitrate);
        lame_set_quality(glf, quality);
        lame_init_params(glf);
        FILE *pcm = fopen(fileSource, "rb");
        FILE *mp3 = fopen(fileDest, "wb");
        const int PCM_SIZE = 8192;
        const int MP3_SIZE = 8192;
        short int pcm_buffer[PCM_SIZE * 2];
        unsigned char mp3_buffer[MP3_SIZE];
        int read = 0;
        int write = 0;
        do {
            read = fread(pcm_buffer, 2 * sizeof(short int), PCM_SIZE, pcm);
            if (read == 0)
                write = lame_encode_flush(glf, mp3_buffer, MP3_SIZE);
            else
                write = lame_encode_buffer_interleaved(glf, pcm_buffer, read, mp3_buffer, MP3_SIZE);
            if (write > 0)
                fwrite(mp3_buffer, sizeof(unsigned char), write, mp3);
        } while (read != 0);

        lame_close(glf);
        fclose(mp3);
        fclose(pcm);
    }

    env->ReleaseStringUTFChars(fileSource_, fileSource);
    env->ReleaseStringUTFChars(fileDest_, fileDest);
    return 0;
}

