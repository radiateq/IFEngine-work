#pragma once


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

struct SFANNPong {
 int max_neurons = 40, neurons_between_reports = 0, input_neurons = 4, output_neurons = 1;
 float desired_error = 0, input_scale = 0.1, output_scale = 10.00;
};


void Init_IFAdapter(engine &engine);
void Train_Cascade_FANN_Forces_Callback(unsigned int num_data, unsigned int num_input, unsigned int num_output, fann_type *input, fann_type *output);
void Train_Cascade_FANN_PaddleBrains_Callback(unsigned int num_data, unsigned int num_input, unsigned int num_output, fann_type *input, fann_type *output);
void TESTFN_PostOperations(engine &engine);
void TESTFN_AddRandomBody(engine &engine);
void TEST_Cleanup();




