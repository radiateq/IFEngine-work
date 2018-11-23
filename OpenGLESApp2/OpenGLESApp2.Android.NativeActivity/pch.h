//
// pch.h
// Header for standard system include files.
//
// Used by the build system to generate the precompiled header. Note that no
// pch.cpp is needed and the pch.h is automatically included in all cpp files
// that are part of the project
//

#include <jni.h>
#include <errno.h>

#include <stdlib.h>  
#include <string.h>
#include <unistd.h>
#include <numeric>
#include <sys/resource.h>

#include <EGL/egl.h>
#include <GLES/gl.h>
//#include <GLES3/gl31.h> //OpenGL ES 3.1 Header File.
//#include <GLES2/gl2ext.h> //OpenGL ES Extension Header File.
//#include <GLES3/gl3platform.h> //OpenGL ES 3.1 Platform-Dependent Macros (this header is shared with OpenGL ES 3.0).


#include <android/sensor.h>


#include <android/log.h>
#include "android_native_app_glue.h"

#include <deque>  



//Added after project generation
 //System Includes
 //Android SDK/NDK
#include <android/input.h> 
#include <android/keycodes.h>
 //External Includes
#include <png.h>
#include "android_fopen.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <unsupported/Eigen/FFT>
 //Domestic Includes
#include "RQ_NDK_Utils.h"
#include "DrawText.h"


