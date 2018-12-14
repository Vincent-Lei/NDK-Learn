cmake_minimum_required(VERSION 3.4.1)
include_directories(src/main/cpp/test)
include_directories(src/main/cpp/globle)
include_directories(src/main/cpp/include)
include_directories(src/main/cpp/audio)
include_directories(src/main/cpp/libaudio)
include_directories(src/main/cpp/soundtouch/include)
include_directories(src/main/cpp/soundtouch/SoundTouch)
include_directories(src/main/cpp/lamemp3)
include_directories(src/main/cpp/lamemp3/lib)

file(GLOB native_test_files "${CMAKE_SOURCE_DIR}/src/main/cpp/test/*.cpp")
file(GLOB native_cpp_files "${CMAKE_SOURCE_DIR}/src/main/cpp/*.cpp")
file(GLOB native_c_files "${CMAKE_SOURCE_DIR}/src/main/cpp/*.c")
file(GLOB native_globle_files "${CMAKE_SOURCE_DIR}/src/main/cpp/globle/*.cpp")
file(GLOB native_lib_audio_files "${CMAKE_SOURCE_DIR}/src/main/cpp/libaudio/*.cpp")
file(GLOB native_soundtouch_files "${CMAKE_SOURCE_DIR}/src/main/cpp/soundtouch/SoundTouch/*.cpp")
file(GLOB native_lamemp3_1_files "${CMAKE_SOURCE_DIR}/src/main/cpp/lamemp3/*.cpp")
file(GLOB native_lamemp3_2_files "${CMAKE_SOURCE_DIR}/src/main/cpp/lamemp3/lib/*.c")

add_library(native-lib SHARED
        ${native_test_files}
        ${native_globle_files}
        ${native_cpp_files}
        ${native_c_files}
        ${native_lib_audio_files}
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

add_library(avcodec-57 SHARED IMPORTED)
set_target_properties(avcodec-57
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavcodec-57.so)


add_library(avdevice-57 SHARED IMPORTED)
set_target_properties(avdevice-57
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavdevice-57.so)


add_library(avfilter-6 SHARED IMPORTED)
set_target_properties(avfilter-6
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavfilter-6.so)


add_library(avformat-57 SHARED IMPORTED)
set_target_properties(avformat-57
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavformat-57.so)


add_library(avutil-55 SHARED IMPORTED)
set_target_properties(avutil-55
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavutil-55.so)

add_library(postproc-54 SHARED IMPORTED)
set_target_properties(postproc-54
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libpostproc-54.so)

add_library(swresample-2 SHARED IMPORTED)
set_target_properties(swresample-2
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libswresample-2.so)


add_library(swscale-4 SHARED IMPORTED)
set_target_properties(swscale-4
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libswscale-4.so)

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

target_link_libraries( # Specifies the target library.
        native-lib
        avcodec-57
        avdevice-57
        avfilter-6
        avformat-57
        avutil-55
        postproc-54
        swresample-2
        swscale-4
        OpenSLES

        ${log-lib})