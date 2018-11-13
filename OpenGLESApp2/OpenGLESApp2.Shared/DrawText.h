#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H


#include <../OpenGLESApp2.Android.NativeActivity/RQ_NDK_Utils.h>


extern FT_Library  library;
extern FT_Face     face;
extern int32_t deviceDPI;
extern FT_GlyphSlot  slot;
extern FT_Matrix     matrix;              /* transformation matrix */
extern FT_UInt       glyph_index;
extern FT_Vector     pen;                 /* untransformed origin */
extern int           n;

bool InitFreeType(struct android_app* state);
bool SetFaceSize(FT_F26Dot6  char_width, FT_F26Dot6  char_height );
bool DrawText( char *_text, FT_UInt _target_height, double _angle);