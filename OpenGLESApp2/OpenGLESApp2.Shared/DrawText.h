#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H


#include <../OpenGLESApp2.Android.NativeActivity/RQ_NDK_Utils.h>


extern FT_Library  library;

bool InitFreeType(struct android_app* state);