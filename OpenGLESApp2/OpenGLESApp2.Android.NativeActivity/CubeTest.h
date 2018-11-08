
#ifndef __CUBETESTSOURCECOPYRQ__
#define __CUBETESTSOURCECOPYRQ__

#include <stdio.h>
#ifdef __ANDROID__
#include <GLES3/gl3.h>
#elif __APPLE__
#include <OpenGLES/ES3/gl.h>
#endif

#include <EGL/eglext.h>
#include <png.h>


//#include <../OpenGLESApp2.Shared/common/shader.hpp>


//Before calling update, call Cube_Test_Init
//if pCube_Test_Init not null function pointed will be called at the end of the Cube_Test_Init
//data may be passed as void *


void CubeTest_setupGL(double width, double height);
void CubeTest_tearDownGL();
void CubeTest_update();
void CubeTest_prepare();
void CubeTest_draw();
GLuint png_texture_load(const char * , int * , int * );

//int Cube_Test_Init(const void *);

#endif /* defined(__HelloCubeNative__main__) */




