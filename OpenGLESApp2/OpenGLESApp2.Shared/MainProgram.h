#pragma once


#include <cstdarg>

#include <../OpenGLESApp2.Android.NativeActivity/IFGlobals.h>
#include "IFBox2DAdapter.h"
#include <../OpenGLESApp2.Android.NativeActivity/IFAudioAdapter.h>
#include "IFEUtils.h"
#include "IFGameEditor.h"
#include "IFFANN.h"
#include "IFFANNEngine.h"


#include <Eigen/Dense>
#include <Eigen/Geometry> 



/**
* Our saved state data.
*/
struct saved_state {
 float angle;
 int32_t x;
 int32_t y;
};
/**
* Shared state for our app.
*/
struct engine {
 bool EGL_initialized;

 struct android_app *app;

 ASensorManager *sensorManager, *sensorManager2;
 const ASensor *accelerometerSensor, *sensor2;
 ASensorEventQueue *sensorEventQueue, *sensorEventQueue2;

 int animating;
 EGLDisplay display;
 EGLSurface surface;
 EGLContext context;
 int32_t width;
 int32_t height;
 struct saved_state state;
};


extern ifCB2Adapter IFAdapter;
extern ifCB2BodyUtils B2BodyUtils;





class CIFLevel {
public:
 static int current_level;
 engine *game_engine = NULL;
 virtual void Init_IFAdapter() = 0;
 virtual void AdvanceGame() = 0;
 virtual void PostOperations() = 0;
 virtual void Cleanup() = 0;
};

extern CIFLevel *IFLevel[10];









