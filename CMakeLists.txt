cmake_minimum_required (VERSION 2.8.9)
project (libdaku)

# Version 0.1
set (libdaku_VERSION_MAJOR 0)
set (libdaku_VERSION_MINOR 1)

# Source

AUX_SOURCE_DIRECTORY (utils src_util_files)
AUX_SOURCE_DIRECTORY (effects src_effect_files)
AUX_SOURCE_DIRECTORY (. src_core_files)
SET (SOURCE_FILES
    ${src_util_files}
    ${src_effect_files}
    ${src_core_files}
    "demo/playplay.c"
)

# Library ---- EDIT YOURSELF!

INCLUDE_DIRECTORIES (
    E:/ffmpeg-20150717-git-8250943-win64-dev/include
    E:/freetype/src/freetype/2.3.5/freetype-2.3.5/include
    E:/freetype/src/freetype/2.3.5/freetype-2.3.5/include/freetype
)

LINK_DIRECTORIES (
    E:/ffmpeg-20150717-git-8250943-win64-dev/lib
    "E:/freetype/src/freetype/2.3.5/freetype-2.3.5/builds/win32/visualc/x64/Release Singlethreaded"
)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(/TP)
endif()

ADD_EXECUTABLE (daku ${SOURCE_FILES})


target_link_libraries (
    daku
    avcodec.lib
    avfilter.lib
    avformat.lib
    avutil.lib
    swresample.lib
    swscale.lib
    freetype.lib
)
