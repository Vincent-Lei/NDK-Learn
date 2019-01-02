cmake_minimum_required(VERSION 3.4.1)
include_directories(src/main/cpp/test)
include_directories(src/main/cpp/globle)
include_directories(src/main/cpp/include)
include_directories(src/main/cpp/audio)
include_directories(src/main/cpp/libaudio)
include_directories(src/main/cpp/libvideo)
include_directories(src/main/cpp/soundtouch/include)
include_directories(src/main/cpp/soundtouch/SoundTouch)
include_directories(src/main/cpp/lamemp3)
include_directories(src/main/cpp/lamemp3/lib)

file(GLOB native_test_files "${CMAKE_SOURCE_DIR}/src/main/cpp/test/*.cpp")
file(GLOB native_cpp_files "${CMAKE_SOURCE_DIR}/src/main/cpp/*.cpp")
file(GLOB native_c_files "${CMAKE_SOURCE_DIR}/src/main/cpp/*.c")
file(GLOB native_globle_files "${CMAKE_SOURCE_DIR}/src/main/cpp/globle/*.cpp")
file(GLOB native_lib_audio_files "${CMAKE_SOURCE_DIR}/src/main/cpp/libaudio/*.cpp")
file(GLOB native_lib_video_files "${CMAKE_SOURCE_DIR}/src/main/cpp/libvideo/*.cpp")
file(GLOB native_soundtouch_files "${CMAKE_SOURCE_DIR}/src/main/cpp/soundtouch/SoundTouch/*.cpp")
file(GLOB native_lamemp3_1_files "${CMAKE_SOURCE_DIR}/src/main/cpp/lamemp3/*.cpp")
file(GLOB native_lamemp3_2_files "${CMAKE_SOURCE_DIR}/src/main/cpp/lamemp3/lib/*.c")

add_library(native-lib SHARED
        ${native_test_files}
        ${native_globle_files}
        ${native_cpp_files}
        ${native_c_files}
        ${native_lib_audio_files}
        ${native_lib_video_files}
        ${native_lamemp3_1_files}
        ${native_lamemp3_2_files}
        ${native_soundtouch_files})

#add_library( # Sets the name of the library.
#        native-lib
#
#        # Sets the library as a shared library.
#        SHARED
#
#        # Provides a relative path to your source file(s).
#        src/main/cpp/native-lib.cpp)

add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavcodec.so)


add_library(avdevice SHARED IMPORTED)
set_target_properties(avdevice
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavdevice.so)


add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavfilter.so)


add_library(avformat SHARED IMPORTED)
set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavformat.so)


add_library(avutil SHARED IMPORTED)
set_target_properties(avutil
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavutil.so)

add_library(postproc SHARED IMPORTED)
set_target_properties(postproc
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libpostproc.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libswresample.so)


add_library(swscale SHARED IMPORTED)
set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libswscale.so)

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

target_link_libraries( # Specifies the target library.
        native-lib
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        postproc
        swresample
        swscale
        OpenSLES

        ${log-lib})