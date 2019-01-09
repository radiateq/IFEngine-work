/*
* Copyright (C) 2010 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/
#define _CRTDBG_MAP_ALLOC  

#include "pch.h"




//#include "Cube.h"

//Added after project generation
 //System Includes
 //External Includes
 //Domestic Includes
//#include "./CubeTest.h"


#include <MainProgram.h>
#include <Level01.h>



ifCB2Adapter IFAdapter;
ifCB2BodyUtils B2BodyUtils(&IFAdapter);



//extern TS_User_Data User_Data;
//extern ifCB2Adapter IFAdapter;
//extern GLuint TEST_textid;
//extern GLuint TEST_GUI_Tex_Ary[10];
//extern bool FANN_TEST_initialized;
//extern IFFANN::IFS_Cascade_FANN LittleBrains;
//extern ifCB2Body *last_touched_object;
//extern float last_y_acceleration, last_x_acceleration;

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))


/**
* Initialize an EGL context for the current display.
*/
static int engine_init_display(struct engine* engine) {
 // initialize OpenGL ES and EGL
 engine->EGL_initialized = false;
 /*
 * Here specify the attributes of the desired configuration.
 * Below, we select an EGLConfig with at least 8 bits per color
 * component compatible with on-screen windows
 */
 const EGLint attribs[] = {
  EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
  EGL_BLUE_SIZE, 8,
  EGL_GREEN_SIZE, 8,
  EGL_RED_SIZE, 8,
  EGL_NONE
 };
 EGLint w, h, format;
 EGLint numConfigs;
 EGLConfig config;
 EGLSurface surface;
 EGLContext context;

 EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
 if (display == EGL_NO_DISPLAY) {
  LOGW("EGL Init Error 1");
  return -1;
 }

 EGLint verMaj, verMin;
 if (!eglInitialize(display, &verMaj, &verMin)) {
  LOGW("EGL Init Error 2");
  return -1;
 }

 /* Here, the application chooses the configuration it desires. In this
 * sample, we have a very simplified selection process, where we pick
 * the first EGLConfig that matches our criteria */
 if (EGL_FALSE == eglChooseConfig(display, attribs, &config, 1, &numConfigs)) {
  LOGW("EGL Init Error 3");
  return -1;
 }

 /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
 * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
 * As soon as we picked a EGLConfig, we can safely reconfigure the
 * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
 if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
  LOGW("EGL Init Error 4");
  return -1;
 }

 if (ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format) < 0) {
  LOGW("EGL Init Error 5");
  return -1;
 }

 if (EGL_NO_SURFACE == (surface = eglCreateWindowSurface(display, config, engine->app->window, NULL))) {
  LOGW("EGL Init Error 6");
  return -1;
 }
 if (EGL_NO_CONTEXT == (context = eglCreateContext(display, config, NULL, NULL))) {
  LOGW("EGL Init Error 7");
  return -1;
 }

 if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
  LOGW("EGL Init Error 8");
  return -1;
 }

 eglQuerySurface(display, surface, EGL_WIDTH, &w);
 eglQuerySurface(display, surface, EGL_HEIGHT, &h);

 engine->EGL_initialized = true;
 engine->display = display;
 engine->context = context;
 engine->surface = surface;
 engine->width = w;
 engine->height = h;
 engine->state.angle = 0;

 // Initialize GL state.
 //CubeTest_setupGL(w, h);
 IFLevel[CIFLevel::current_level]->Setup_OpenGL(w, h);

 IFLevel[CIFLevel::current_level]->game_engine = engine;
 IFLevel[CIFLevel::current_level]->Init_IFAdapter();

 IFAudioSLES::BuildAudioEngine(User_Data.state->activity);

 return 0;
}

/**
* Just the current frame in the display.
*/

static void engine_draw_frame(struct engine* engine) {
 //if (engine->display == NULL) {
 // // No display.
 // return;
 //}


 //cleanup far bodies
 //B2BodyUtils.RemoveFarBodies();




 if (engine->display != NULL) {
  PrepareDraw();
 }

 if(User_Data.TrainInProgress){
  /////////////////////////////////////NC01START
  IFAdapter.StartSimFF();
  struct timespec temp_timespec;
  clock_gettime(CLOCK_MONOTONIC, &temp_timespec);
  unsigned long int start_time = RQNDKUtils::timespec2us64(&temp_timespec), end_time = start_time + 14667;
  while(end_time > start_time) {
   IFLevel[CIFLevel::current_level]->AdvanceGame();
   IFAdapter.UpdateSimFF();
   IFAdapter.UpdateGraphics();
   IFLevel[CIFLevel::current_level]->PostOperations();
   clock_gettime(CLOCK_MONOTONIC, &temp_timespec);
   start_time = RQNDKUtils::timespec2us64(&temp_timespec);
  }
  /////////////////////////////////////NC01STOP
 }else{
  IFLevel[CIFLevel::current_level]->AdvanceGame();
  IFAdapter.UpdateSim();
  IFAdapter.UpdateGraphics();
  IFLevel[CIFLevel::current_level]->PostOperations();
 }
//#define SPEEDUP
////#undef SPEEDUP
//#ifdef SPEEDUP
// //multiple box steps per frame
// IFAdapter.StartSimFF(30);
// for(unsigned int cnt = 0; cnt < 30; cnt++){
//  TESTFN_AddRandomBody(*engine);
//  IFAdapter.UpdateSimFF();
//  IFAdapter.UpdateGraphics();
//  TESTFN_PostOperations(*engine);
// }
// #else
// // One box step per frame
// TESTFN_AddRandomBody(*engine);
// IFAdapter.UpdateSim();
// IFAdapter.UpdateGraphics();
// TESTFN_PostOperations(*engine);
//#endif
//#undef SPEEDUP


 //CubeTest_draw();
 if (engine->display != NULL) {
  DrawBodies();
  eglSwapBuffers(engine->display, engine->surface);
 }
}


/**
* Tear down the EGL context currently associated with the display.
*/
static void engine_term_display(struct engine* engine) {
 if (engine->display != EGL_NO_DISPLAY) {
  eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  if (engine->context != EGL_NO_CONTEXT) {
   eglDestroyContext(engine->display, engine->context);
  }
  if (engine->surface != EGL_NO_SURFACE) {
   eglDestroySurface(engine->display, engine->surface);
  }
  eglTerminate(engine->display);
 }
 engine->animating = 0;
 engine->display = EGL_NO_DISPLAY;
 engine->context = EGL_NO_CONTEXT;
 engine->surface = EGL_NO_SURFACE;

 //CubeTest_tearDownGL();  
 //glDeleteTextures(1, &TEST_textid);
 //TEST_textid = GL_INVALID_VALUE;
 //glDeleteTextures(1, &User_Data.CubeTexture);
 //User_Data.CubeTexture = GL_INVALID_VALUE;
 //glDeleteTextures(10, TEST_GUI_Tex_Ary);
 //for(unsigned int cnt = 0; cnt < sizeof(TEST_GUI_Tex_Ary)/ sizeof(TEST_GUI_Tex_Ary[0]); cnt++){
 // TEST_GUI_Tex_Ary[cnt] = GL_INVALID_VALUE;
 //}


 IFAudioSLES::TearDownAudioEngine();
 
 IFLevel[CIFLevel::current_level]->Cleanup();
}

/**
* Process the next input event.
*/
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
 struct engine* engine = (struct engine*)app->userData;
 int32_t EventType = AInputEvent_getType(event);
 switch (EventType)
 {
 case AINPUT_EVENT_TYPE_MOTION:
  engine->state.x = AMotionEvent_getX(event, 0);
  engine->state.y = AMotionEvent_getY(event, 0);
  User_Data.Touch_Time = AMotionEvent_getEventTime(event);
  User_Data.Touch_Input_X = engine->state.x;
  User_Data.Touch_Input_Y = engine->state.y;
  switch (AInputEvent_getSource(event)) {
  case AINPUT_SOURCE_TOUCHSCREEN:
   int action = AKeyEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
   switch (action) {
   case AMOTION_EVENT_ACTION_DOWN:
    User_Data.Event_Indicator_Touch_Input++;
    User_Data.pointer_down = true;
    break;
   case AMOTION_EVENT_ACTION_UP:
    //User_Data.Event_Indicator_Touch_Input++;
    User_Data.pointer_down = false;
    break;
   case AMOTION_EVENT_ACTION_MOVE:
    User_Data.Event_Indicator_Touch_Input++;
    break;
   }
   break;
  } // end switch


  return 1;
 };
 return 0;
}

/**
* Process the next main command.
*/
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
 struct engine* engine = (struct engine*)app->userData;

 switch (cmd) {
 case APP_CMD_SAVE_STATE:
  // The system has asked us to save our current state.  Do so.
  engine->app->savedState = malloc(sizeof(struct saved_state));
  *((struct saved_state*)engine->app->savedState) = engine->state;
  engine->app->savedStateSize = sizeof(struct saved_state);
  break;
 case APP_CMD_INIT_WINDOW:
  // The window is being shown, get it ready.
  if (engine->app->window != NULL) {
   engine_init_display(engine);
   engine_draw_frame(engine);
  }
  break;
 case APP_CMD_TERM_WINDOW:
  // The window is being hidden or closed, clean it up.
  engine_term_display(engine);
  break;
 case APP_CMD_GAINED_FOCUS:
  // When our app gains focus, we start monitoring the accelerometer.
  if (engine->accelerometerSensor != NULL) {
   ASensorEventQueue_enableSensor(engine->sensorEventQueue,
    engine->accelerometerSensor);
   // We'd like to get 60 events per second (in us).
   ASensorEventQueue_setEventRate(engine->sensorEventQueue,
    engine->accelerometerSensor, (1000L / 1000) * 1000);
  }
  if (engine->sensor2 != NULL) {
   ASensorEventQueue_enableSensor(engine->sensorEventQueue2,
    engine->sensor2);
   // We'd like to get 60 events per second (in us).
   ASensorEventQueue_setEventRate(engine->sensorEventQueue2,
    engine->sensor2, (1000L / 1000) * 1000);
  }
  // Also stop animating.
  engine->animating = 1;
  engine_draw_frame(engine);
  break;
 case APP_CMD_LOST_FOCUS:
  // When our app loses focus, we stop monitoring the accelerometer.
  // This is to avoid consuming battery while not being used.
  if (engine->accelerometerSensor != NULL) {
   ASensorEventQueue_disableSensor(engine->sensorEventQueue,
    engine->accelerometerSensor);
  }
  if (engine->sensor2 != NULL) {
   ASensorEventQueue_disableSensor(engine->sensorEventQueue2,
    engine->sensor2);
  }
  // Also stop animating.
  engine->animating = 0;
  engine_draw_frame(engine);
  break;
 }
}

/**
* This is the main entry point of a native application that is using
* android_native_app_glue.  It runs in its own thread, with its own
* event loop for receiving input events and doing other things.
*/

void android_main(struct android_app* state) {




 Init_IFEngine();
 //               FREETYPE               SAMPLE START


 User_Data.state = state;

 //User_Data.state->activity->env;

 struct engine engine;

 memset(&engine, 0, sizeof(engine));
 state->userData = &engine;
 state->onAppCmd = engine_handle_cmd;
 state->onInputEvent = engine_handle_input;
 engine.app = state;

 // Prepare to monitor accelerometer
 engine.sensorManager = ASensorManager_getInstance();
 engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager, ASENSOR_TYPE_ACCELEROMETER);
 engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager, state->looper, LOOPER_ID_USER, NULL, NULL);

 engine.sensorManager2 = ASensorManager_getInstance();
 engine.sensor2 = ASensorManager_getDefaultSensor(engine.sensorManager2, ASENSOR_TYPE_LIGHT);
 engine.sensorEventQueue2 = ASensorManager_createEventQueue(engine.sensorManager2, state->looper, LOOPER_ID_USER + 1, NULL, NULL);


 //p_user_data = &User_Data;



 if (state->savedState != NULL) {
  // We are starting with a previous saved state; restore from it.
  engine.state = *(struct saved_state*)state->savedState;
 }

 engine.animating = 1;





 //----------------------------------------------------                   FANN NETWORK TEST START




 //IFFANNEngine::CFANNNetwork FANNNetwork;
 //FANNNetwork.AddToNetwork("test123", 1);




 //----------------------------------------------------                   FANN NETWORK TEST STOP


 // loop waiting for stuff to do.

 while (1) {
  // Read all pending events.
  int ident;
  int events;
  struct android_poll_source* source;



  ///////////////////////////////////////////////           IF TEST CODE START
  //IFAdapter.World->SetGravity( b2Vec2(-event.acceleration.x * 50.0, event.acceleration.y * 50.0 ) );
  //case 'q':
  // //apply gradual force upwards
  // bodies[0]->ApplyForce(b2Vec2(0, 50), bodies[0]->GetWorldCenter());
  // break;
  //case 'w':
  // //apply immediate force upwards
  // bodies[1]->ApplyLinearImpulse(b2Vec2(0, 50), bodies[1]->GetWorldCenter());
  // break;
  //case 'e':
  // //teleport or 'warp' to new location
  // bodies[2]->SetTransform(b2Vec2(10, 20), 0);
  // break;
  //default:
  // //run default behaviour
  // Test::Keyboard(key);
  ///////////////////////////////////////////////           IF TEST CODE STOP
  // Check if we are exiting.


  // If not animating, we will block forever waiting for events.
  // If animating, we loop until all events are read, then continue
  // to draw the next frame of animation.
  while ((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
   (void**)&source)) >= 0) {

   // Process this event.
   if (source != NULL) {
    source->process(state, source);
   }

   // If a sensor has data, process it now.
   if (ident == LOOPER_ID_USER) {
    if (engine.accelerometerSensor != NULL) {
     ASensorEvent event;
     while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
      &event, 1) > 0) { 
      //LOGI("accelerometer: x=%f y=%f z=%f",
      //	event.acceleration.x, event.acceleration.y,
      //	event.acceleration.z);
      if(CIFLevel::current_level==1){
       User_Data.accel[0].push_back(event.acceleration.x / 8.f);
       User_Data.accel[1].push_back(event.acceleration.y / 8.f);
      }
      //last_y_acceleration = event.acceleration.y;
     }     
    }
   }
   else if (ident == (LOOPER_ID_USER + 1)) {
    if (engine.sensor2 != NULL) {
     ASensorEvent event;
     while (ASensorEventQueue_getEvents(engine.sensorEventQueue2,
      &event, 1) > 0) {
      //LOGI("gyro: x=%f",
      // event.light);
      //for (unsigned int cntbdy = 0; cntbdy < IFAdapter.Bodies.size(); cntbdy++) {
      // if (IFAdapter.Bodies[cntbdy]->body->GetType() == b2_dynamicBody) {
      //  //IFAdapter.Bodies[cntbdy]->body->ApplyLinearImpulse(b2Vec2(0, event.light), IFAdapter.Bodies[cntbdy]->body->GetPosition(), true);
      //  last_light = event.light - last_light;
      //  IFAdapter.Bodies[cntbdy]->body->ApplyLinearImpulse(b2Vec2(0, (last_light) * ((last_y_acceleration >0)?1:-1)), IFAdapter.Bodies[cntbdy]->body->GetPosition(), true);
      //  avglight.push_back(last_light);
      //  avglight.resize(last_light_cnt);
      //  last_light = std::accumulate(avglight.begin(), avglight.end(), 0.0) / avglight.size();        
      // }
      //}
      // // IFAdapter.Bodies[0]->body->ApplyLinearImpulse(b2Vec2( event.light * 20.0, -event.light * 0.0), IFAdapter.Bodies[0]->body->GetWorldCenter(), true );
     }
    }
   }




   if (state->destroyRequested != 0) {
    engine_term_display(&engine);
    return;
   }
  }














  if (engine.EGL_initialized && engine.animating) {
   // Done with events; draw next animation frame.




   // Drawing is throttled to the screen update rate, so there
   // is no need to do timing here.
   engine_draw_frame(&engine);
  }
 }
}
