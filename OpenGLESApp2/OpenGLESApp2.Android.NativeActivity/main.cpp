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

#include "IFGlobals.h"
#include "IFBox2DAdapter.h"
#include "IFAudioAdapter.h"
#include "IFEUtils.h"
#include "IFGameEditor.h"
#include "IFFANN.h"
#include "IFFANNEngine.h"

#include <Eigen/Dense>
#include <Eigen/Geometry> 

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))



ifCB2Adapter IFAdapter;

ifCB2BodyUtils B2BodyUtils(&IFAdapter);

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


TS_User_Data User_Data;

//DEMO 1 GLOBAL VARIABLES START
GLuint TEST_textid;
GLfloat TEST_text_ub, TEST_text_vb, TEST_text_ut, TEST_text_vt;
struct timespec TEST_Last_Added_Body_Time;
float last_y_acceleration = 0, last_x_acceleration = 0, last_light = 0;
size_t last_light_cnt = 30;
std::vector<float>avglight;
//DEMO 2 - FANN - GLOBAL VARIABLES START
bool FANN_TEST_initialized = false;
IFFANN::IFS_Cascade_FANN LittleBrains;
ifCB2Body *anns_body, *anns_learned_body;
bool FANN_Learning_Phase = false;
std::vector<fann_type> input_data, output_data;
unsigned int input_data_sets = 0;
static float last_pos_x = FLT_MAX, last_pos_y = FLT_MAX;
int max_neurons = 20, neurons_between_reports = 0, input_layers = 3, output_layers = 2;
float desired_error = 0.001, input_scale = 0.01, output_scale = 0.01;
int touchx, touchy;
//DEMO 3 - BODY + GUI 
bool BODY_DEMO_initialized = false;
ifCB2Body *last_touched_object = NULL;
GLuint TEST_GUI_Tex_Ary[10] = {0};
ifCB2Body *buttons_body[10];
//DEMO 4 - Simple Game
ifCB2Body *game_body[10];
bool DEMO4_initialized = false;
float thickness;
float left, bottom, right, top;
float radius, dummy;
IFFANN::IFS_Cascade_FANN PaddleBrains;
bool Pong_Learning_Phase = false;
struct SFANNPong{
 int max_neurons = 8, neurons_between_reports = 0, input_layers = 2, output_layers = 1;
 float desired_error = 0.00001, input_scale = 0.001, output_scale = 10.0;
} FANNPong;
IFFANNEngine::CNetwork Network;
IFFANNEngine::CNode Node1;
IFFANNEngine::CNode Node2;

void Train_Cascade_FANN_Forces_Callback(unsigned int num_data, unsigned int num_input, unsigned int num_output, fann_type *input, fann_type *output){
 //Inputs are:
 // 1 - gravity y
 // 2 - acceleration x
 // 3 - acceleration y
 //Outpus are:
 // 1 - delta position x
 // 2 - delta position y
  //input[0] = IFA_World->GetGravity().y / LittleBrains.input_scale;
  //input[1] = last_x_acceleration / LittleBrains.input_scale;
  //input[2] = last_y_acceleration / LittleBrains.input_scale; 

  //float x = anns_body->body->GetPosition().x, y = anns_body->body->GetPosition().y;

  //if(last_pos_x == FLT_MAX ){
  // output[0] = 0;
  //}else{
  // output[0] = anns_body->body->GetPosition().x - last_pos_x;
  //}
  //last_pos_x = anns_body->body->GetPosition().x;
  //if (last_pos_y == FLT_MAX) {
  // output[1] = 0;
  //}
  //else {
  // output[1] = anns_body->body->GetPosition().y - last_pos_y;
  //}
  //last_pos_y = anns_body->body->GetPosition().y;

  //output[0] /= LittleBrains.output_scale;
  //output[1] /= LittleBrains.output_scale;

 input[0] = input_data[num_data * 3 + 0];
 input[1] = input_data[num_data * 3 + 1];
 input[2] = input_data[num_data * 3 + 2];



 output[0] = output_data[(num_data) * 2 + 0];
 output[1] = output_data[(num_data) * 2 + 1];


}
void Train_Cascade_FANN_PaddleBrains_Callback(unsigned int num_data, unsigned int num_input, unsigned int num_output, fann_type *input, fann_type *output) {
 input[0] = input_data[num_data * 2 + 0];
 input[1] = input_data[num_data * 2 + 1];
 output[0] = output_data[(num_data) * 1 + 0];
int a = 1;
a = 3;
}
void TESTFN_PostOperations(engine &engine){
 if (engine.EGL_initialized) {
  //DEMO 3 - USER INTERFACE - START
  if (!BODY_DEMO_initialized) {
  }else{
   if (last_touched_object) {

    Eigen::Transform<float, 3, Eigen::TransformTraits::Affine> t;
    Eigen::Matrix<float, 4, 4> mScale;
    //void CopyFloat16ToMatrix(MatrixXf &mf, float *mfa);
    //void CopyMatrix16ToFloat(MatrixXf &mf, float *mfa);
    IFGeneralUtils::CopyFloat16ToMatrix(mScale, last_touched_object->OGL_body->modelview_matrix);
    t.matrix() = mScale;

    GLuint texture_index;
   
    if(buttons_body[0]== last_touched_object){
     texture_index = 0;
    }else{
     texture_index = 1;
    }

    glDeleteTextures(0,&TEST_GUI_Tex_Ary[texture_index]);
    char outstring[120];
    strcpy(outstring,"touched");    
    TEST_GUI_Tex_Ary[texture_index] = last_touched_object->OGL_body->texture_ID = DrawText(outstring, 15, FT_Vector() = { 160 * 64,40 * 64 }, 0.0, &TEST_text_ub, &TEST_text_vb, &TEST_text_ut, &TEST_text_vt);

    static bool _temp_lastScale = false;
    if (_temp_lastScale) {
     t *= Eigen::Scaling<float>(1.2, 1.2, 1.0);
    }
    else {
     t *= Eigen::Scaling<float>(1.0, 1.0, 1.0);
    }
    _temp_lastScale = !_temp_lastScale;

    //mScale = t.matrix();
    memcpy(last_touched_object->OGL_body->modelview_matrix, t.data(), sizeof(float) * 16);

    //


    //if (abs(last_touched_object->OGL_body->z_pos )< abs(zDefaultLayer))
     //last_touched_object->OGL_body->modelview_matrix
 //= zDefaultLayer * 1.1;
   // else
     //last_touched_object->OGL_body->z_pos = zDefaultLayer * 0.9;

   }
  }
 }
}
//DEMO 2 - FANN - GLOBAL VARIABLES STOP
void TESTFN_AddRandomBody(engine &engine){
 if (engine.EGL_initialized) {

 if(!DEMO4_initialized) {
  DEMO4_initialized = true;
  thickness = RQNDKUtils::getDensityDpi(android_app_state) / 25.4;
  left = thickness; bottom = thickness; right = engine.width- thickness; top = engine.height- thickness;
  Window2ObjectCoordinates(left, bottom, zDefaultLayer, engine.width, engine.height);
  left /= IFA_box2D_factor; bottom /= IFA_box2D_factor;
  Window2ObjectCoordinates(right, top, zDefaultLayer, engine.width, engine.height);
  right /= IFA_box2D_factor; top /= IFA_box2D_factor;

  IFAdapter.OrderBody();
  IFAdapter.OrderedBody()->body_def->type = b2_staticBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :    
  b2EdgeShape *edgeShape = new b2EdgeShape;
  edgeShape->Set(b2Vec2(left,bottom), b2Vec2(left,top ));
  b2FixtureDef *fixture = new b2FixtureDef;
  fixture->shape = edgeShape;
  fixture->density = 1.0;
  fixture->friction = 0.0;
  fixture->restitution = 1.000;
  //fixture->filter.categoryBits = 0x0008;
  //fixture->filter.maskBits = 0x0010;
  IFAdapter.OrderedBody()->AddShapeAndFixture(edgeShape, fixture);
  game_body[0] = IFAdapter.OrderedBody();
  if (!IFAdapter.MakeBody())
   return;
  game_body[0]->body->SetTransform(b2Vec2(0.0, 0.0), 0.0);
  game_body[0]->OGL_body->texture_ID = User_Data.CubeTexture;
  game_body[0]->OGL_body->line_thickness = thickness;


  IFAdapter.OrderBody();
  IFAdapter.OrderedBody()->body_def->type = b2_staticBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :    
  edgeShape = new b2EdgeShape;
  edgeShape->Set(b2Vec2(right, bottom), b2Vec2(right, top));
  fixture = new b2FixtureDef;
  fixture->shape = edgeShape;
  fixture->density = 1.0;
  fixture->friction = 0.0;
  fixture->restitution = 1.000;
  //fixture->filter.categoryBits = 0x0008;
  //fixture->filter.maskBits = 0x0010;
  IFAdapter.OrderedBody()->AddShapeAndFixture(edgeShape, fixture);
  game_body[1] = IFAdapter.OrderedBody();
  if (!IFAdapter.MakeBody())
   return;
  game_body[1]->body->SetTransform(b2Vec2(0.0, 0.0), 0.0);
  game_body[1]->OGL_body->texture_ID = User_Data.CubeTexture;
  game_body[1]->OGL_body->line_thickness = thickness;






  IFAdapter.OrderBody();
  IFAdapter.OrderedBody()->body_def->type = b2_dynamicBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody : 
  b2CircleShape *polyShape = new b2CircleShape;
  polyShape->m_p.SetZero();
  radius = engine.width>engine.height?engine.width:engine.height, dummy = 0.0;
  radius*=0.02;
  radius+=engine.width*0.5;
  Window2ObjectCoordinates(radius, dummy, zDefaultLayer, engine.width, engine.height);
  radius /= IFA_box2D_factor;
  polyShape->m_radius = radius;
  fixture = new b2FixtureDef;
  fixture->shape = polyShape;
  fixture->density = 1.0;
  fixture->friction = 0.0;
  fixture->restitution = 1.000;
  //fixture->filter.categoryBits = 0x0008;
  //fixture->filter.maskBits = 0x0010;
  IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape, fixture);
  game_body[2] = IFAdapter.OrderedBody();
  if (!IFAdapter.MakeBody())
   return;
  game_body[2]->OGL_body->texture_ID = User_Data.CubeTexture;









  IFAdapter.OrderBody();
  IFAdapter.OrderedBody()->body_def->type = b2_kinematicBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :    
  b2PolygonShape *polyShape2 = new b2PolygonShape;
  b2Vec2 shapeCoords[8];
  float zoom_factor;
  zoom_factor = engine.width < engine.height ? engine.width : engine.height, dummy = 0.0;
  zoom_factor /= 12.0 * 24.0;
  zoom_factor += engine.width*0.5;
  Window2ObjectCoordinates(zoom_factor, dummy, zDefaultLayer, engine.width, engine.height);
  //shapeCoords[0] = { zoom_factor *-5.0, zoom_factor * 0.0 };
  //shapeCoords[1] = { zoom_factor *-3, zoom_factor *  -2 };
  //shapeCoords[2] = { zoom_factor * 0,zoom_factor *-3};
  //shapeCoords[3] = { zoom_factor * 3,zoom_factor *-2};
  //shapeCoords[4] = { zoom_factor * 5,zoom_factor * 0};
  //shapeCoords[5] = { zoom_factor * 2,zoom_factor * 2};
  //shapeCoords[6] = { zoom_factor * 0,zoom_factor * 3 };
  //shapeCoords[7] = { zoom_factor *-2, zoom_factor * 2 };
  shapeCoords[0].x = zoom_factor *-12, shapeCoords[0].y = zoom_factor *-1;
  shapeCoords[1].x = zoom_factor * 12, shapeCoords[1].y = zoom_factor *-1;
  shapeCoords[2].x = zoom_factor * 12, shapeCoords[2].y = zoom_factor * 1;
  shapeCoords[3].x = zoom_factor *-12, shapeCoords[3].y = zoom_factor * 1;
  polyShape2->Set(shapeCoords, 4);
  fixture = new b2FixtureDef;
  fixture->shape = polyShape2;
  fixture->density = 1.0;
  fixture->friction = 0.0;
  fixture->restitution = 1.000;
  //fixture->filter.categoryBits = 0x0008;
  //fixture->filter.maskBits = 0x0010;
  IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape2, fixture);
  game_body[3] = IFAdapter.OrderedBody();
  if (!IFAdapter.MakeBody())
   return;
  game_body[3]->body->SetTransform(b2Vec2(0.0, -bottom), 0.0);
  game_body[3]->OGL_body->texture_ID = User_Data.CubeTexture;









  IFAdapter.OrderBody();
  IFAdapter.OrderedBody()->body_def->type = b2_kinematicBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :    
  polyShape2 = new b2PolygonShape;
  zoom_factor = engine.width < engine.height ? engine.width : engine.height, dummy = 0.0;
  zoom_factor /= 12.0 * 24.0;
  zoom_factor += engine.width*0.5;
  Window2ObjectCoordinates(zoom_factor, dummy, zDefaultLayer, engine.width, engine.height);
  shapeCoords[0].x = zoom_factor * -12, shapeCoords[0].y = zoom_factor * -1;
  shapeCoords[1].x = zoom_factor * 12, shapeCoords[1].y = zoom_factor * -1;
  shapeCoords[2].x = zoom_factor * 12, shapeCoords[2].y = zoom_factor * 1;
  shapeCoords[3].x = zoom_factor * -12, shapeCoords[3].y = zoom_factor * 1;
  polyShape2->Set(shapeCoords, 4);
  fixture = new b2FixtureDef;
  fixture->shape = polyShape2;
  fixture->density = 1.0;
  fixture->friction = 0.0;
  fixture->restitution = 1.000;
  //fixture->filter.categoryBits = 0x0008;
  //fixture->filter.maskBits = 0x0010;
  IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape2, fixture);
  game_body[4] = IFAdapter.OrderedBody();
  if (!IFAdapter.MakeBody())
   return;
  game_body[4]->body->SetTransform(b2Vec2(0.0, bottom), 0.0);
  game_body[4]->OGL_body->texture_ID = User_Data.CubeTexture;









  IFAdapter.OrderBody();
  IFAdapter.OrderedBody()->body_def->type = b2_staticBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :    
  polyShape2 = new b2PolygonShape;
  shapeCoords[8];
#define  zoom_factor 1.8
  //shapeCoords[0] = { zoom_factor *-5.0, zoom_factor * 0.0 };
  //shapeCoords[1] = { zoom_factor *-3, zoom_factor *  -2 };
  //shapeCoords[2] = { zoom_factor * 0,zoom_factor *-3};
  //shapeCoords[3] = { zoom_factor * 3,zoom_factor *-2};
  //shapeCoords[4] = { zoom_factor * 5,zoom_factor * 0};
  //shapeCoords[5] = { zoom_factor * 2,zoom_factor * 2};
  //shapeCoords[6] = { zoom_factor * 0,zoom_factor * 3 };
  //shapeCoords[7] = { zoom_factor *-2, zoom_factor * 2 };
  shapeCoords[0] = { zoom_factor *-6, zoom_factor *-2 };
  shapeCoords[1] = { zoom_factor * 6, zoom_factor * -2 };
  shapeCoords[2] = { zoom_factor * 6, zoom_factor * 2 };
  shapeCoords[3] = { zoom_factor *-6, zoom_factor * 2 };
#undef zoom_factor
  polyShape2->Set(shapeCoords, 4);
  fixture = new b2FixtureDef;
  fixture->shape = polyShape2;
  fixture->density = 1.1;
  fixture->friction = 0.3;
  fixture->restitution = 0.001;
  fixture->filter.categoryBits = 0x0008;
  fixture->filter.maskBits = 0x0010;
  IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape2, fixture);
  game_body[5] = IFAdapter.OrderedBody();
  if (!IFAdapter.MakeBody())
   return;
  game_body[5]->body->SetTransform(b2Vec2(-16.0, 0.0), 0.0);
  game_body[5]->OGL_body->z_pos -= 0.1;
  SetFaceSize(100 * 64, 60 * 64);
  char outstring[120];
  strcpy(outstring, "train");
  //TEST_textid = first_body->OGL_body->texture_ID = DrawText(outstring, 5, FT_Vector()={160*64,40*64}, 3.141593*0.50, &TEST_text_ub, &TEST_text_vb, &TEST_text_ut, &TEST_text_vt);
  TEST_GUI_Tex_Ary[0] = game_body[5]->OGL_body->texture_ID = DrawText(outstring, 15, FT_Vector() = { 160 * 64,40 * 64 }, 0.0, &TEST_text_ub, &TEST_text_vb, &TEST_text_ut, &TEST_text_vt);
  //TEST_textid = first_body->OGL_body->texture_ID = DrawText(outstring, 5, FT_Vector() = { 40 * 64,60 * 64 }, 3.141593*0.0, &TEST_text_ub, &TEST_text_vb, &TEST_text_ut, &TEST_text_vt);
  size_t UVsize = game_body[5]->OGL_body->UVmapping_cnt;
  for (int cntuv = 0; cntuv < UVsize; cntuv++) {
   game_body[5]->OGL_body->UVmapping[cntuv] *= TEST_text_ut;
   cntuv++;
   game_body[5]->OGL_body->UVmapping[cntuv] *= TEST_text_vt;
  }










  if (IFFANN::Check_Save_Cascade_FANN("pongpaddle", IFFANN::CnTrainedFannPostscript)) {
   IFFANN::Load_Cascade_FANN(&PaddleBrains, "pongpaddle", IFFANN::CnTrainedFannPostscript);   
  }
  if (!PaddleBrains.ann) IFFANN::Setup_Train_Cascade_FANN(IFFANN::Create_Cascade_FANN(IFFANN::Init_Cascade_FANN(&PaddleBrains), FANNPong.input_layers, FANNPong.output_layers, "pongpaddle"), FANNPong.max_neurons, FANNPong.neurons_between_reports, FANNPong.desired_error, FANNPong.input_scale, FANNPong.output_scale);
  //IFFANN::Setup_Train_Cascade_FANN(IFFANN::Create_Cascade_FANN(IFFANN::Init_Cascade_FANN(&PaddleBrains), FANNPong.input_layers, FANNPong.output_layers, "pongpaddle"), FANNPong.max_neurons, FANNPong.neurons_between_reports, FANNPong.desired_error, FANNPong.input_scale, FANNPong.output_scale);
  
  Pong_Learning_Phase = false;
  input_data_sets = 0;

  Network.NodeRegister.Register(&Node1);
  IFFANN::Load_Cascade_FANN(&PaddleBrains, "pongpaddle", IFFANN::CnTrainedFannPostscript);
  IFFANN::Save_Cascade_FANN(&PaddleBrains, IFFANN::CnFinalFannPostscript);
  Node1.LoadCore("pongpaddle");





  game_body[2]->body->ApplyLinearImpulse(b2Vec2(drand48()*500.0, (drand48() * 1.0-0.5) * 500.0), game_body[2]->body->GetPosition(), true);


  }else{


   float maxx = engine.width;
   float maxy = engine.height;
   Window2ObjectCoordinates(maxx, maxy, zDefaultLayer, maxx, maxy);
   float cut_off_distance = b2Distance(b2Vec2(0, 0), b2Vec2(maxx / IFA_box2D_factor, maxy / IFA_box2D_factor)) * 1;

   {

    b2Vec2 position = game_body[3]->body->GetPosition();
    game_body[3]->body->SetTransform(b2Vec2(-last_x_acceleration * 2.4, -bottom), game_body[3]->body->GetAngle());

   }

   if (b2Distance(b2Vec2(game_body[2]->body->GetPosition().x, game_body[2]->body->GetPosition().y), b2Vec2(0, 0)) > cut_off_distance) {
    if (game_body[2]->body->GetPosition().y < 0) {
     int deleteamount = output_data.size() - 30;
     if (deleteamount >= 0) {
      while (input_data.size() > (deleteamount * 2))input_data.pop_back();
      while (output_data.size() > (deleteamount * 1))output_data.pop_back();
      input_data_sets = output_data.size();
     }
    }
    game_body[2]->body->SetTransform(b2Vec2(0, 0), game_body[2]->body->GetAngle());
    game_body[2]->body->SetLinearVelocity(b2Vec2(0, 0));
    game_body[2]->body->SetAngularVelocity(0);   
    game_body[2]->body->ApplyLinearImpulse(b2Vec2(drand48() * 500.0, (drand48() * 1.0 - 0.5) * 500.0), game_body[2]->body->GetPosition(), true);
   }else{



    if(abs(game_body[2]->body->GetLinearVelocity().y)<10.0){
     game_body[2]->body->ApplyLinearImpulse(b2Vec2(0, (game_body[2]->body->GetLinearVelocity().y<0?-100.0:100.0)), game_body[2]->body->GetPosition(), true);
    }


    if (IFGameEditor::GetTouchEvent()) {
     while (IFGameEditor::GetTouchEvent(&touchx, &touchy));

     struct timespec temp_timespec;
     clock_gettime(CLOCK_MONOTONIC, &temp_timespec);
     //temp_int64 = timespec2ms64(&temp_timespec) - timespec2ms64(&game_time_0);
     unsigned long int temp_int64 = RQNDKUtils::timespec2ms64(&temp_timespec) - RQNDKUtils::timespec2ms64(&TEST_Last_Added_Body_Time);
     if (temp_int64 > 2000) {
      {
       IFGameEditor::GetTouchEvent(&touchx, &touchy);

       typename std::list<ifCB2Body*>::iterator iter;
       for (iter = IFAdapter.Bodies.begin(); iter != IFAdapter.Bodies.end(); iter++) {
        if(game_body[5]==*iter){
         if (B2BodyUtils.RayTestHitpoint(touchx, touchy, *iter)) {
          //while (IFGameEditor::GetTouchEvent(&touchx, &touchy));

          if(input_data_sets>20){
           Pong_Learning_Phase = true;        
          }

          break;
         }
        }
       }

      }
      TEST_Last_Added_Body_Time = temp_timespec;
     }   
    }



    {


     if(Pong_Learning_Phase){
      IFFANN::Setup_Train_Cascade_FANN(IFFANN::Create_Cascade_FANN(IFFANN::Init_Cascade_FANN(&PaddleBrains), FANNPong.input_layers, FANNPong.output_layers, "pongpaddle"), FANNPong.max_neurons, FANNPong.neurons_between_reports, FANNPong.desired_error, FANNPong.input_scale, FANNPong.output_scale);
      IFFANN::Train_Cascade_FANN(&PaddleBrains, Train_Cascade_FANN_PaddleBrains_Callback, input_data_sets);
      IFFANN::Load_Cascade_FANN(&PaddleBrains, "pongpaddle", IFFANN::CnTrainedFannPostscript);
      IFFANN::Save_Cascade_FANN(&PaddleBrains, IFFANN::CnFinalFannPostscript);
      Node1.LoadCore("pongpaddle");
      Pong_Learning_Phase = false;
      struct timespec temp_timespec;
      clock_gettime(CLOCK_MONOTONIC, &temp_timespec);
      TEST_Last_Added_Body_Time = temp_timespec;
     }

     //input_data_sets = 0;
     //input_data.clear();
     //output_data.clear();




     float ball_angle_to_pad;
     b2Vec2 balllinvel = game_body[2]->body->GetLinearVelocity();
     b2Vec2 ballposition = game_body[2]->body->GetPosition();
     b2Vec2 paddleposition ;
     bool AIPaddle=false;
     if( ballposition.y > 0 ){
      AIPaddle = true;
     }
     if(AIPaddle){
      paddleposition = game_body[4]->body->GetPosition();     
     }else{
      paddleposition = game_body[3]->body->GetPosition();
      balllinvel.x *= -1.0;
      balllinvel.y *= -1.0;
     }
     {
      if (AIPaddle) {
       unsigned int ID;
       fann_type *pin_value;
       //Set all inputs to some value
       Network.NodeRegister.InputPinRegister.input_pins.ResetIterator();
       int cnt = 0;
       while (0 <= Network.NodeRegister.InputPinRegister.input_pins.GetNextIterator(ID, pin_value)) {
        switch(cnt++){
        case 0:
         *pin_value = b2Distance(paddleposition, ballposition) / Node1.ifann.input_scale;
        break;
        case 1:
         *pin_value = atan2(balllinvel.x, balllinvel.y) / Node1.ifann.input_scale;
        break;
       
        };       
       }
       fann_scale_input(Node1.ifann.ann, Node1.inputs);
       Network.Run();
       fann_descale_output(Node1.ifann.ann, Node1.outputs);
       //get output values
       Network.NodeRegister.OutputPinRegister.output_pins.ResetIterator();
       while (0 <= Network.NodeRegister.OutputPinRegister.output_pins.GetNextIterator(ID, pin_value)) {
        if (pin_value) {
         fann_type val = *pin_value * Node1.ifann.output_scale;
         b2Vec2 position = game_body[4]->body->GetPosition();
         game_body[4]->body->SetTransform(b2Vec2( val * 1.0, position.y), game_body[4]->body->GetAngle());
        }
       }
      }else{
       static int skipCnt = 0;
       if(skipCnt++==0){    
        skipCnt = 0;
        input_data.push_back( b2Distance(paddleposition, ballposition) / PaddleBrains.input_scale);
        input_data.push_back(atan2(balllinvel.x, balllinvel.y) / PaddleBrains.input_scale);
        output_data.push_back(paddleposition.x / PaddleBrains.output_scale);
        input_data_sets++;
        if (input_data.size() > ((60*60*60 *2 ) + 120)) {
         input_data.erase(input_data.begin(), input_data.begin() + (input_data.size() - (60 * 60 * 60 *2)));
         input_data_sets = input_data.size() / 2;
         output_data.erase(output_data.begin(), output_data.begin() + input_data_sets);
        }
       }
      }
     }
    }
   }  
  }
  //DEMO 4 - Simple Game - START

  return;
  //DEMO 4 - Simple Game - STOP



  //DEMO 3 - USER INTERFACE - START
  if (!BODY_DEMO_initialized) {
   BODY_DEMO_initialized = true;

   last_touched_object = NULL;
   
   IFAdapter.OrderBody();
   IFAdapter.OrderedBody()->body_def->type = b2_staticBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody : 
   b2CircleShape *polyShape = new b2CircleShape;
   polyShape->m_p.SetZero();
   polyShape->m_radius = 5.5;
   b2FixtureDef *fixture = new b2FixtureDef;
   fixture->shape = polyShape;
   fixture->density = 1.1;
   fixture->friction = 0.3;
   fixture->restitution = 0.001;
   fixture->filter.categoryBits = 0x0008;
   fixture->filter.maskBits = 0x0010;
   IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape, fixture);
   buttons_body[0] = IFAdapter.OrderedBody();
   if (!IFAdapter.MakeBody())
    return;
   buttons_body[0]->OGL_body->texture_ID = User_Data.CubeTexture;


   memset(TEST_GUI_Tex_Ary, GL_INVALID_VALUE, sizeof(TEST_GUI_Tex_Ary[0]) * sizeof(TEST_GUI_Tex_Ary));


   IFAdapter.OrderBody();
   IFAdapter.OrderedBody()->body_def->type = b2_staticBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :    
   b2PolygonShape *polyShape2 = new b2PolygonShape;
   b2Vec2 shapeCoords[8];
   #define  zoom_factor 2.0
   //shapeCoords[0] = { zoom_factor *-5.0, zoom_factor * 0.0 };
   //shapeCoords[1] = { zoom_factor *-3, zoom_factor *  -2 };
   //shapeCoords[2] = { zoom_factor * 0,zoom_factor *-3};
   //shapeCoords[3] = { zoom_factor * 3,zoom_factor *-2};
   //shapeCoords[4] = { zoom_factor * 5,zoom_factor * 0};
   //shapeCoords[5] = { zoom_factor * 2,zoom_factor * 2};
   //shapeCoords[6] = { zoom_factor * 0,zoom_factor * 3 };
   //shapeCoords[7] = { zoom_factor *-2, zoom_factor * 2 };
   shapeCoords[0] = { zoom_factor *-6, zoom_factor *-2 };
   shapeCoords[1] = { zoom_factor *6, zoom_factor * -2};
   shapeCoords[2] = { zoom_factor *6, zoom_factor * 2 };
   shapeCoords[3] = { zoom_factor *-6, zoom_factor * 2 };
#undef zoom_factor
   polyShape2->Set(shapeCoords, 4);
   fixture = new b2FixtureDef;
   fixture->shape = polyShape2;
   fixture->density = 1.1;
   fixture->friction = 0.3;
   fixture->restitution = 0.001;
   fixture->filter.categoryBits = 0x0008;
   fixture->filter.maskBits = 0x0010;
   IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape2, fixture);
   buttons_body[1] = IFAdapter.OrderedBody();
   if (!IFAdapter.MakeBody())
    return;
   buttons_body[1]->body->SetTransform(b2Vec2(0.0,-10.0), 0.0);
   SetFaceSize(100 * 64, 60 * 64);
   char outstring[120];
   strcpy(outstring,"Option 1");
   //TEST_textid = first_body->OGL_body->texture_ID = DrawText(outstring, 5, FT_Vector()={160*64,40*64}, 3.141593*0.50, &TEST_text_ub, &TEST_text_vb, &TEST_text_ut, &TEST_text_vt);
   TEST_GUI_Tex_Ary[0] = buttons_body[1]->OGL_body->texture_ID = DrawText(outstring, 15, FT_Vector() = { 160 * 64,40 * 64 },0.0, &TEST_text_ub, &TEST_text_vb, &TEST_text_ut, &TEST_text_vt);
   //TEST_textid = first_body->OGL_body->texture_ID = DrawText(outstring, 5, FT_Vector() = { 40 * 64,60 * 64 }, 3.141593*0.0, &TEST_text_ub, &TEST_text_vb, &TEST_text_ut, &TEST_text_vt);
   size_t UVsize = buttons_body[1]->OGL_body->UVmapping_cnt;
   for (int cntuv = 0; cntuv < UVsize; cntuv++) {
    buttons_body[1]->OGL_body->UVmapping[cntuv] *= TEST_text_ut;
    cntuv++;
    buttons_body[1]->OGL_body->UVmapping[cntuv] *= TEST_text_vt;
   }




   IFAdapter.OrderBody();
   IFAdapter.OrderedBody()->body_def->type = b2_staticBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :    
   polyShape2 = new b2PolygonShape;
#define  zoom_factor 2.0
   //shapeCoords[0] = { zoom_factor *-5.0, zoom_factor * 0.0 };
   //shapeCoords[1] = { zoom_factor *-3, zoom_factor *  -2 };
   //shapeCoords[2] = { zoom_factor * 0,zoom_factor *-3};
   //shapeCoords[3] = { zoom_factor * 3,zoom_factor *-2};
   //shapeCoords[4] = { zoom_factor * 5,zoom_factor * 0};
   //shapeCoords[5] = { zoom_factor * 2,zoom_factor * 2};
   //shapeCoords[6] = { zoom_factor * 0,zoom_factor * 3 };
   //shapeCoords[7] = { zoom_factor *-2, zoom_factor * 2 };
   shapeCoords[0] = { zoom_factor *-6, zoom_factor *-2 };
   shapeCoords[1] = { zoom_factor * 6, zoom_factor * -2 };
   shapeCoords[2] = { zoom_factor * 6, zoom_factor * 2 };
   shapeCoords[3] = { zoom_factor *-6, zoom_factor * 2 };
#undef zoom_factor
   polyShape2->Set(shapeCoords, 4);
   fixture = new b2FixtureDef;
   fixture->shape = polyShape2;
   fixture->density = 1.1;
   fixture->friction = 0.3;
   fixture->restitution = 0.001;
   fixture->filter.categoryBits = 0x0008;
   fixture->filter.maskBits = 0x0010;
   IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape2, fixture);
   buttons_body[2] = IFAdapter.OrderedBody();
   if (!IFAdapter.MakeBody())
    return;
   buttons_body[2]->body->SetTransform(b2Vec2(0.0, -25.0), 0.0);
   SetFaceSize(100 * 64, 60 * 64);
   strcpy(outstring, "Very long button text");
   //TEST_textid = first_body->OGL_body->texture_ID = DrawText(outstring, 5, FT_Vector()={160*64,40*64}, 3.141593*0.50, &TEST_text_ub, &TEST_text_vb, &TEST_text_ut, &TEST_text_vt);
   TEST_GUI_Tex_Ary[1] = buttons_body[2]->OGL_body->texture_ID = DrawText(outstring, 15, FT_Vector() = { 160 * 64,40 * 64 }, 0.0, &TEST_text_ub, &TEST_text_vb, &TEST_text_ut, &TEST_text_vt);
   //TEST_textid = first_body->OGL_body->texture_ID = DrawText(outstring, 5, FT_Vector() = { 40 * 64,60 * 64 }, 3.141593*0.0, &TEST_text_ub, &TEST_text_vb, &TEST_text_ut, &TEST_text_vt);
   UVsize = buttons_body[2]->OGL_body->UVmapping_cnt;
   for (int cntuv = 0; cntuv < UVsize; cntuv++) {
    buttons_body[2]->OGL_body->UVmapping[cntuv] *= TEST_text_ut;
    cntuv++;
    buttons_body[2]->OGL_body->UVmapping[cntuv] *= TEST_text_vt;
   }


  }else{

   if (IFGameEditor::GetTouchEvent()) {
    while (IFGameEditor::GetTouchEvent(&touchx, &touchy));

    struct timespec temp_timespec;
    clock_gettime(CLOCK_MONOTONIC, &temp_timespec);
    //temp_int64 = timespec2ms64(&temp_timespec) - timespec2ms64(&game_time_0);
    unsigned long int temp_int64 = RQNDKUtils::timespec2ms64(&temp_timespec) - RQNDKUtils::timespec2ms64(&TEST_Last_Added_Body_Time);
    if (temp_int64 > 2000) {
//     if (touchy > (engine.height * 0.5)) {
//     }else
     {
      IFGameEditor::GetTouchEvent(&touchx, &touchy);
     
      typename std::list<ifCB2Body*>::iterator iter;
      for (iter = IFAdapter.Bodies.begin(); iter != IFAdapter.Bodies.end(); iter++) {
       if (B2BodyUtils.RayTestHitpoint(touchx, touchy, *iter)) {

        //IFAdapter.Bodies.removeElement(*iter);
        //if(last_touched_object)
        // last_touched_object->OGL_body->z_pos = zDefaultLayer;        

        last_touched_object = (*iter);
        break;
       }
      }

     }
     TEST_Last_Added_Body_Time = temp_timespec;
    }
    while (IFGameEditor::GetTouchEvent(&touchx, &touchy));
   }





  }

 // return;
//DEMO 3 - USER INTERFACE - STOP
  //DEMO 2 - FANN - START

  if(!FANN_TEST_initialized){
   FANN_TEST_initialized = true;

   if(IFFANN::Check_Save_Cascade_FANN("forces01", IFFANN::CnTrainedFannPostscript )){
    IFFANN::Load_Cascade_FANN(&LittleBrains, "forces01", IFFANN::CnTrainedFannPostscript);
   }
   if(!LittleBrains.ann) IFFANN::Setup_Train_Cascade_FANN(IFFANN::Create_Cascade_FANN(IFFANN::Init_Cascade_FANN(&LittleBrains), input_layers, output_layers, "forces01"), max_neurons, neurons_between_reports, desired_error, input_scale, output_scale);
   FANN_Learning_Phase = false;
   input_data_sets = 0;
   
   IFAdapter.OrderBody();
   IFAdapter.OrderedBody()->body_def->type = b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody : 
   ifCB2Body *first_body = IFAdapter.OrderedBody();
   b2CircleShape *polyShape = new b2CircleShape;
   polyShape->m_p.SetZero();
   polyShape->m_radius = 2.5;
   b2FixtureDef *fixture = new b2FixtureDef;
   fixture->shape = polyShape;
   fixture->density = 1.1;
   fixture->friction = 0.3;
   fixture->restitution = 0.001;
   fixture->filter.categoryBits = 0x0008;
   fixture->filter.maskBits = 0x0010;
   IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape, fixture);
   anns_body = IFAdapter.OrderedBody();
   if (!IFAdapter.MakeBody())
    return;
   anns_body->OGL_body->texture_ID = User_Data.CubeTexture;



   IFAdapter.OrderBody();
   IFAdapter.OrderedBody()->body_def->type = b2_kinematicBody;//((drand48() > 0.5) ? b2_staticBody : 
   first_body = IFAdapter.OrderedBody();
   polyShape = new b2CircleShape;
   polyShape->m_p.SetZero();
   polyShape->m_radius = 2.5;
   fixture = new b2FixtureDef;
   fixture->shape = polyShape;
   fixture->density = 1.1;
   fixture->friction = 0.3;
   fixture->restitution = 0.001;
   fixture->filter.categoryBits= 0x0002;
   fixture->filter.maskBits = 0x0004;
   IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape, fixture);
   anns_learned_body = IFAdapter.OrderedBody();
   if (!IFAdapter.MakeBody())
    return;
   anns_learned_body->OGL_body->texture_ID = User_Data.CubeTexture;

  }

  if (IFGameEditor::GetTouchEvent()) {
   while (IFGameEditor::GetTouchEvent(&touchx, &touchy));

   struct timespec temp_timespec;
   clock_gettime(CLOCK_MONOTONIC, &temp_timespec);
   //temp_int64 = timespec2ms64(&temp_timespec) - timespec2ms64(&game_time_0);
   unsigned long int temp_int64 = RQNDKUtils::timespec2ms64(&temp_timespec) - RQNDKUtils::timespec2ms64(&TEST_Last_Added_Body_Time);
   if (temp_int64 > 2000) {
    if(touchy > (engine.height * 0.5) ){
     if(input_data_sets>1) FANN_Learning_Phase = true;
    }else{
     FANN_Learning_Phase = false;
     IFFANN::Setup_Train_Cascade_FANN(IFFANN::Create_Cascade_FANN(IFFANN::Init_Cascade_FANN(&LittleBrains), input_layers, output_layers, "forces01"), max_neurons, neurons_between_reports, desired_error, input_scale, output_scale);     //input_data_sets = 0;
     //input_data.clear();
     //output_data.clear();
    }
    TEST_Last_Added_Body_Time = temp_timespec;
   }   
   while (IFGameEditor::GetTouchEvent(&touchx, &touchy));
  }

  float maxx = engine.width;
  float maxy = engine.height;
  Window2ObjectCoordinates(maxx, maxy, zDefaultLayer, maxx, maxy);
  float cut_off_distance = b2Distance(b2Vec2(0, 0), b2Vec2(maxx / IFA_box2D_factor, maxy / IFA_box2D_factor)) * 1;

  last_x_acceleration*=0.1;
  last_y_acceleration *=0.1;  
  if (b2Distance(b2Vec2(anns_body->body->GetPosition().x, anns_body->body->GetPosition().y), b2Vec2(0, 0)) > cut_off_distance) {
   anns_body->body->SetTransform(b2Vec2(0, 0), anns_learned_body->body->GetAngle());
   anns_body->body->SetLinearVelocity(b2Vec2(0, 0));
   anns_body->body->SetAngularVelocity(0);
   last_pos_x = 0, last_pos_y = 0;
  } else if (!FANN_Learning_Phase) {
   last_pos_x = anns_body->body->GetPosition().x;
   last_pos_y = anns_body->body->GetPosition().y;
   
anns_body->body->SetTransform(b2Vec2(last_x_acceleration + anns_body->body->GetPosition().x, last_y_acceleration + anns_body->body->GetPosition().y), anns_body->body->GetAngle());//Has immediate effect, set last_pos_x before

//float coordx = touchx;
//float coordy = touchy;
//Window2ObjectCoordinates(coordx , coordy , zDefaultLayer, engine.width, engine.height);
//anns_body->body->SetTransform(b2Vec2(coordx / IFA_box2D_factor, coordy / IFA_box2D_factor), anns_body->body->GetAngle());

//anns_body->body->ApplyLinearImpulse(b2Vec2(last_x_acceleration, last_y_acceleration), anns_body->body->GetPosition(), true);//Has effect after world step, set last_pos_x after

   if ((last_pos_x != FLT_MAX)&&(last_pos_y != FLT_MAX)){
    input_data.push_back(IFA_World->GetGravity().y / LittleBrains.input_scale);
    //input_data[(input_data_sets+0) * 3 + 0] = IFA_World->GetGravity().y / LittleBrains.input_scale;
    input_data.push_back(last_x_acceleration / LittleBrains.input_scale);
    //input_data[(input_data_sets+0) * 3 + 1] = last_x_acceleration / LittleBrains.input_scale;
    input_data.push_back(last_y_acceleration / LittleBrains.input_scale);
    //input_data[(input_data_sets+0) * 3 + 2] = last_y_acceleration / LittleBrains.input_scale;
    output_data.push_back((last_pos_x - anns_body->body->GetPosition().x) / LittleBrains.output_scale);
    //output_data[input_data_sets * 2 + 0] = (last_pos_x - anns_body->body->GetPosition().x) / LittleBrains.output_scale;
    output_data.push_back((last_pos_y - anns_body->body->GetPosition().y) / LittleBrains.output_scale);
    //output_data[input_data_sets * 2 + 1] = (last_pos_y - anns_body->body->GetPosition().y) / LittleBrains.output_scale;
    input_data_sets++;
    if (input_data.size() > ((60 * 60 * 60 *  3) + 120)) {
     input_data.erase(input_data.begin(), input_data.begin() + (input_data.size() - (60 * 60 * 60 * 3)));
     input_data_sets = input_data.size()/3;
     output_data.erase(output_data.begin(), output_data.begin() + input_data_sets*2);
    }
   }
  }
  if (FANN_Learning_Phase&&input_data_sets){
   IFFANN::Setup_Train_Cascade_FANN(IFFANN::Create_Cascade_FANN(IFFANN::Init_Cascade_FANN(&LittleBrains), input_layers, output_layers, "forces01"), max_neurons, neurons_between_reports, desired_error, input_scale, output_scale);
   IFFANN::Train_Cascade_FANN(&LittleBrains, Train_Cascade_FANN_Forces_Callback, input_data_sets);  
   //fann_set_scaling_params(LittleBrains.ann, LittleBrains.ann_train->train_data, -1, 1, min_output, max_output);
   //input_data_sets = 0;
   FANN_Learning_Phase = false;

   input_data_sets = 0;
   input_data.clear();
   output_data.clear();


  }else{
//   IFFANN::Save_Cascade_FANN(&LittleBrains, IFFANN::CnFinalFannPostscript);
//   IFFANNEngine::TestNetwork();

   fann_type inputs[3];
   inputs[0] = IFA_World->GetGravity().y / LittleBrains.input_scale;
   inputs[1] = last_x_acceleration / LittleBrains.input_scale;
   inputs[2] = last_y_acceleration / LittleBrains.input_scale;   
   fann_scale_input(LittleBrains.ann, inputs);
   fann_type *outputs = IFFANN::Run_Cascade_FANN(&LittleBrains, inputs);
   fann_descale_output(LittleBrains.ann,outputs);
   outputs[0] = anns_learned_body->body->GetPosition().x - outputs[0] * LittleBrains.output_scale;
   outputs[1] = anns_learned_body->body->GetPosition().y - outputs[1] * LittleBrains.output_scale;
   if (b2Distance(b2Vec2(outputs[0], outputs[1]), b2Vec2(0, 0)) <= cut_off_distance) {
    anns_learned_body->body->SetTransform(b2Vec2(outputs[0], outputs[1]), anns_learned_body->body->GetAngle());
   }
   else {
    anns_learned_body->body->SetTransform(b2Vec2(0, 0), anns_learned_body->body->GetAngle());
   }
  }

  


  //DEMO 2 - FANN - STOP
  //return;
  
 //OLD DEMO 1 START 

  if (IFGameEditor::GetTouchEvent()){   
   int touchx, touchy;

   struct timespec temp_timespec;
   clock_gettime(CLOCK_MONOTONIC, &temp_timespec);
   //temp_int64 = timespec2ms64(&temp_timespec) - timespec2ms64(&game_time_0);
   unsigned long int temp_int64 = RQNDKUtils::timespec2ms64(&temp_timespec) - RQNDKUtils::timespec2ms64(&TEST_Last_Added_Body_Time);
   if(temp_int64<200){
    while(IFGameEditor::GetTouchEvent(&touchx, &touchy));
    return;
   }
   TEST_Last_Added_Body_Time = temp_timespec;

   IFGameEditor::GetTouchEvent(&touchx, &touchy);   

   typename std::list<ifCB2Body*>::iterator iter;
   for (iter = IFAdapter.Bodies.begin(); iter != IFAdapter.Bodies.end(); iter++) {
    if( B2BodyUtils.RayTestHitpoint( touchx, touchy, *iter ) ){
     IFAdapter.Bodies.removeElement(*iter);
     return;
    }
   }

   IFAdapter.OrderBody();
   IFAdapter.OrderedBody()->body_def->type = ((drand48()>0.5)?b2_staticBody: b2_dynamicBody);
   if( drand48() > 0.3){


    b2EdgeShape *polyShape = new b2EdgeShape;
    b2Vec2 shapeCoords[8];
    b2FixtureDef *fixture = new b2FixtureDef;




    float screenx = touchx;
    float screeny = touchy;
    Window2ObjectCoordinates(screenx, screeny, zDefaultLayer, engine.width, engine.height);
    IFAdapter.OrderedBody()->body_def->position.Set(screenx / IFA_box2D_factor, screeny / IFA_box2D_factor);
    shapeCoords[0] = { -4,  0 };
    shapeCoords[1] = { -3,  0 };
    shapeCoords[2] = { 3,   0 };
    shapeCoords[3] = { 4,   0 };

    polyShape->Set(shapeCoords[1], shapeCoords[2]);
    polyShape->m_hasVertex0 = true;
    polyShape->m_hasVertex3 = true;
    polyShape->m_vertex0 = shapeCoords[0];
    polyShape->m_vertex3 = shapeCoords[3];

    //polyShape->Set(shapeCoords, 4);
    fixture->shape = polyShape;
    fixture->density = 1.1;
    fixture->friction = 0.3;
    fixture->restitution = 0.001;

    IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape, fixture);


   }else{


    b2CircleShape *polyShape = new b2CircleShape;
    polyShape->m_p.SetZero();
    polyShape->m_radius = 3.0;





    //b2PolygonShape *polyShape = new b2PolygonShape;
    //b2Vec2 shapeCoords[8];
    b2FixtureDef *fixture = new b2FixtureDef;




    //IFAdapter.OrderedBody()->body_def->position.Set(touchx*IFA_box2D_factor, touchy*IFA_box2D_factor);

 //   //Go backwards from viewport to frustrum to transformation coordinates
 //   // First calculate z because we need it to calculate x and y
 //   //Since z is known and does not exist in screen coordinates we calculate it as OpenGL ES would
 //   //RQNDKUtils::SRangeScale2 zRange(IFEUtils::zvnear, IFEUtils::zvfar);
 //   float zCoord = -zDefaultLayer;
 //   //zCoord = zRange.ScaleAndClip(zCoord, IFEUtils::znear, IFEUtils::zfar);
 //   //      then frustum matrix   
 //   zCoord = (zCoord*(IFEUtils::zfar + IFEUtils::znear)) / (IFEUtils::zfar - IFEUtils::znear) + (zCoord*(IFEUtils::zfar* IFEUtils::znear)) / (IFEUtils::zfar - IFEUtils::znear);
 //   //      perspective division
 //   //zCoord /= -zCoord;
 ////   zCoord = ((IFEUtils::zvfar - IFEUtils::zvnear) / 2.0)*zCoord + (IFEUtils::zvnear + IFEUtils::zvfar) / 2.0;
 //
 // 
 //   //               this is result of viewport matrix for x, backwards
 //   float screenx;
 //   //                find center of viewport
 //   float ox = (float)engine.width * 0.5;
 //   //                viewport size in pixels
 //   float px = engine.width;
 //   //                send back through window
 //   screenx = 2.0*(ox- touchx)/px;
 //   //                this is result of frustum matrix for x, backwards
 //   screenx = (IFEUtils::left*screenx+ IFEUtils::left*(zCoord)-IFEUtils::right*screenx+ IFEUtils::right*(zCoord))/(2* IFEUtils::znear);
 //   //                this is reverse transformation matrix
 //   screenx  = (screenx * -zDefaultLayer) / IFA_box2D_factor;
 //   //Same for y
 //   float screeny;// = ((float)engine.height - 2.0f* (float)touchy) / (-(float)engine.height);
 //   float oy = (float)engine.height * 0.5;
 //   float py = engine.height;
 //   screeny = 2.0*(oy - touchy) / py;
 //   //                this is reversed frustum matrix
 //   screeny = (IFEUtils::bottom*screeny + IFEUtils::bottom * (zCoord)-IFEUtils::top * screeny + IFEUtils::top * (zCoord)) / (2 * IFEUtils::znear);
 //   //                this is reverse transformation matrix
 //   screeny = (screeny*zDefaultLayer)/IFA_box2D_factor;
    float screenx = touchx;
    float screeny = touchy;
    Window2ObjectCoordinates(screenx, screeny, zDefaultLayer, engine.width, engine.height );
    IFAdapter.OrderedBody()->body_def->position.Set(screenx/ IFA_box2D_factor, screeny/ IFA_box2D_factor);
    //shapeCoords[0] = { -1,  -1 };
    //shapeCoords[1] = { 16,  -1 };
    //shapeCoords[2] = { 16,   6 };
    //shapeCoords[3] = { -1,   6 };
    //polyShape->Set(shapeCoords, 4);
    fixture->shape = polyShape;
    fixture->density = 1.1;
    fixture->friction = 0.3;
    fixture->restitution = 0.001;
    IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape, fixture);
   }

   ifCB2Body *first_body = IFAdapter.OrderedBody();
   if (!IFAdapter.MakeBody())
    return;
   //Additional work on body  
   if (drand48() > 0.5) {
    first_body->OGL_body->texture_ID = TEST_textid;
    size_t UVsize = first_body->OGL_body->UVmapping_cnt;
    for (int cntuv = 0; cntuv < UVsize; cntuv++) {
     first_body->OGL_body->UVmapping[cntuv] *= TEST_text_ut;
     cntuv++;
     first_body->OGL_body->UVmapping[cntuv] *= TEST_text_vt;
    }

   }
   else {
    first_body->OGL_body->texture_ID = User_Data.CubeTexture; //DrawText(outstring, 4, 0);
   }
   return;
  }
 
 

  double xxxx = drand48();
  if((xxxx *200)<80)
   return;


  IFAudioSLES::EngineServiceBufferMutex.EnterAndLock();
  if(IFAudioSLES::audio_engine_created &&
     !IFAudioSLES::EngineServiceBuffer.empty()){
 /////////////////////////////////////AUDIO PART
   //IFAudioSLES::sample_buf *new_buf = nullptr;
   unsigned int selected_buffer = -1;
   if(IFAudioSLES::EngineServiceBuffer.size()){
    selected_buffer = IFAudioSLES::EngineServiceBuffer.front();
    //while(IFAudioSLES::EngineServiceBuffer.size())
    // IFAudioSLES::EngineServiceBuffer.pop_back();
    while(!IFAudioSLES::EngineServiceBuffer.empty()) IFAudioSLES::EngineServiceBuffer.pop();
   }  
   if(selected_buffer==-1){
    IFAudioSLES::EngineServiceBufferMutex.LeaveAndUnlock();
    return;
   }
   uint32_t allocSize = (IFAudioSLES::engine.record_buffer[selected_buffer].size_ + 3) & ~3;

   double average_value = 0;
   static double prev_average_value = 0;
   static unsigned int nMake = 0;
   bool nMake_added=false;  
   short sample_valueL, sample_valueR;
   bool two_channels = false, LR_switch=true;
   std::vector<float> timevec;

   #define TRESHOLD_COUNT 4
   static std::deque<float> treshold_history[TRESHOLD_COUNT];
   float treshold[TRESHOLD_COUNT];

   //when audio_db starts dropping from constant rise trigger set rising to false
   //when audio_db starts rising set rising to true
   float audio_db;
   if (IFAudioSLES::engine.sampleChannels_ == 2) {
    two_channels = true;
   } 
   for (int cnt = 0; cnt < allocSize; cnt++) {
    if(LR_switch){
     sample_valueL = IFAudioSLES::engine.record_buffer[selected_buffer].buf_[cnt];
    }else{
     sample_valueR = IFAudioSLES::engine.record_buffer[selected_buffer].buf_[cnt];
     timevec.push_back((((float)IFAudioSLES::engine.record_buffer[selected_buffer].buf_[cnt] + sample_valueL) ) / 65535.0);
    }      
    if (two_channels){
     LR_switch = !LR_switch;
    }else{
     timevec.push_back((((float)sample_valueL)) / 32767.0);
    }
    //audio_db = 20 * (log10(abs(sample_valueL ) / 32767.0));
    //average_value+= audio_db;
   }
   //dBFS = 20 * log([sample level] / [max level])
   //In a 16 bit system there are 2 ^ 16 = 65536 values.So this means values from - 32768 to + 32767. Excluding 0, let's say the minimum value is 1. So plugging this into the formula gives:
   //dBFS = 20 * log(1 / 32767) = -90.3
  
   IFAudioSLES::EngineServiceBufferMutex.LeaveAndUnlock();

   if(timevec.size()){

    Eigen::FFT<float> fft;

    std::vector<std::complex<float> > freqvec;

    fft.fwd(freqvec, timevec);
    int ssize = freqvec.size();
    
    for( int cntx = 0; cntx < freqvec.size(); cntx++){
     float freq = (float)cntx*48000.0/ (float)ssize;
 //    float ampl = ((freqvec[cntx].real() >0)?20*log10(freqvec[cntx].real() / 4.0f):-1000.0);
 //    float phase = freqvec[cntx].imag();
     if(freqvec[cntx].real()<0){
      continue;
     }
     if (freq > 60 && freq < 380) {
      if (treshold[0] < freqvec[cntx].real()) {
       treshold[0] = freqvec[cntx].real();
      }
     }else if (freq > 400 && freq < 1200){
      if( treshold[1] < freqvec[cntx].real()){
       treshold[1] = freqvec[cntx].real();
      }
     } else if (freq > 1550 && freq < 4000) {
      if (treshold[2] < freqvec[cntx].real()) {
       treshold[2] = freqvec[cntx].real();
      }
     }else if (freq > 1550 && freq < 4000) {
      if (treshold[3] < freqvec[cntx].real()) {
       treshold[3] = freqvec[cntx].real();
      }
     }
    }
  
    typedef std::deque<float>::iterator TDFiter;
    for( int cntt = 0; cntt < TRESHOLD_COUNT; cntt++){
     treshold_history[cntt].push_back(treshold[cntt]);
     treshold[cntt] = 0;
     for (TDFiter iter = treshold_history[cntt].begin(); iter != treshold_history[cntt].end(); iter++) {
      treshold[cntt] += *iter;
     }
     treshold[cntt] /= treshold_history[cntt].size();
     while( treshold_history[cntt].size()*(IFAudioSLES::engine.frames_per_buffer / IFAudioSLES::engine.sample_rate)>3.0){
      treshold_history[cntt].pop_front();
     }
     treshold[cntt] = 20 * log10(treshold[cntt] / 4.0f);
    }
   

    for (int cntx = 0; cntx < freqvec.size(); cntx++) {
     float freq = (float)cntx*48000.0 / (float)ssize;
     float ampl = ((freqvec[cntx].real() > 0) ? 20 * log10(freqvec[cntx].real() / 4.0f) : -1000.0);
     float phase = freqvec[cntx].imag();
     if(freq>60 && freq<380){
      if(ampl> treshold[0]){
       for (unsigned int cntbdy = 0; cntbdy < IFAdapter.Bodies.size(); cntbdy++) {
        if (IFAdapter.Bodies[cntbdy]->body->GetType() == b2_dynamicBody) {
         IFAdapter.Bodies[cntbdy]->body->ApplyLinearImpulse(b2Vec2(0.0, abs(ampl) * 0.4), IFAdapter.Bodies[cntbdy]->body->GetPosition(), true);
        }
       }
      }
     }else if (freq > 400 && freq < 800 ) {
      if(!nMake_added && ampl > treshold[1]){
       nMake += 1;
       nMake_added = true;
      }     
     }
     else if (freq > 800 && freq < 1500) {
      if (ampl > treshold[2]) {
       for (unsigned int cntbdy = 0; cntbdy < IFAdapter.Bodies.size(); cntbdy++) {
        if (IFAdapter.Bodies[cntbdy]->body->GetType() == b2_dynamicBody) {
         IFAdapter.Bodies[cntbdy]->body->ApplyLinearImpulse(b2Vec2(abs(ampl) * 3.0, 0.0), IFAdapter.Bodies[cntbdy]->body->GetPosition(), true);
        }
       }
      }
     }
     else if (freq > 1550 && freq < 3000) {
      if (ampl > treshold[3]) {
       for (unsigned int cntbdy = 0; cntbdy < IFAdapter.Bodies.size(); cntbdy++) {
        if (IFAdapter.Bodies[cntbdy]->body->GetType() == b2_dynamicBody) {
         IFAdapter.Bodies[cntbdy]->body->ApplyLinearImpulse(b2Vec2(abs(ampl) * -3.0, 0.0), IFAdapter.Bodies[cntbdy]->body->GetPosition(), true);
        }
       }
      }
     }else if(freq>4000){
      //nMake=0;
      break;
     }
     //nthbin[Hz]=n*sampleFreq/num(DFTPoints)
    }

    // manipulate freqvec
    //fft.inv(timevec, freqvec);   
    // stored "plans" get destroyed with fft destructor
   }

   //allocSize = 2;//count of buffers to release
   //IFAudioSLES::releaseSampleBufs(new_buf, allocSize );
   /////////////////////////////////////AUDIO PART
  
   average_value/=allocSize;

   prev_average_value = average_value;



nMake = 20;
nMake_added = true;



   if(nMake==0)
    return;
   nMake--;
   if(!nMake_added)
    nMake=0;

  }else{
   IFAudioSLES::EngineServiceBufferMutex.LeaveAndUnlock();
   return;
  }




  IFAdapter.OrderBody();
  IFAdapter.OrderedBody()->body_def->type = b2_dynamicBody;
  b2PolygonShape *polyShape = new b2PolygonShape;
  b2Vec2 shapeCoords[8];
  b2FixtureDef *fixture = new b2FixtureDef;

  if(drand48()>0.3){
   IFAdapter.OrderedBody()->body_def->position.Set(drand48() * 10 - 5, drand48() * 50);
   shapeCoords[0] = { 0.8,  1.25 };
   shapeCoords[1] = { 1,   0.5 };
   shapeCoords[2] = { 2,   0.25 };
   shapeCoords[3] = { 3,   0.5 };
   shapeCoords[4] = { 4,  1.2 };
   shapeCoords[5] = { 3,  2.0 };
   shapeCoords[6] = { 2,  2.7 };
   shapeCoords[7] = { 1,  2 };
   polyShape->Set(shapeCoords, 8);
   fixture->shape = polyShape;
   fixture->density = drand48()*5.0+1.0;
   fixture->friction = drand48()*1.0;
   fixture->restitution = ((drand48()>0.2)?drand48()*0.001:drand48()*2);
  }else if(drand48()>0.6){
   IFAdapter.OrderedBody()->body_def->position.Set(drand48() * 10 - 5, drand48() * 50);
   shapeCoords[0] = { -1,  -1 };
   shapeCoords[1] = { 1,  -1 };
   shapeCoords[2] = { 1,   1 };
   shapeCoords[3] = { -1,   1 };
   polyShape->Set(shapeCoords, 4);
   fixture->shape = polyShape;
   fixture->density = 1.1;
   fixture->friction = 0.3;
   fixture->restitution = 0.001;
  }else{
   IFAdapter.OrderedBody()->body_def->position.Set(-drand48()*4.0, 0.0);
   IFAdapter.OrderedBody()->body_def->angle = drand48()*360.0;
   shapeCoords[0] = b2Vec2(0.0, 0.0);
   shapeCoords[1] = b2Vec2(static_cast<float32>(engine.width) / 100.0, 0.0);
   shapeCoords[2] = b2Vec2(static_cast<float32>(engine.width) / 100.0, 0.8);
   shapeCoords[3] = b2Vec2(0.0, 0.8);
   polyShape->Set(shapeCoords, 4);
   fixture->shape = polyShape;
   fixture->density = 0.1;
   fixture->friction = 0.0001;

  }
  //shapeCoords[4] = { 4,  1 };
  //shapeCoords[5] = { 3,  2 };
  //shapeCoords[6] = { 2,  2 };
  //shapeCoords[7] = { 1,  2 };

  //shapeCoords[0] = {  8,  12.5 };
  //shapeCoords[1] = { 10.0,   5.0 };
  //shapeCoords[2] = { 20.0,   2.5 };
  //shapeCoords[3] = { 30.0,   5.0 };
  //shapeCoords[4] = { 40.0,  12.5 };
  //shapeCoords[5] = { 30.0,  20.0 };
  //shapeCoords[6] = { 20.0,  27.0 };
  //shapeCoords[7] = { 10.0,  20.0 };
  IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape, fixture);
  ifCB2Body *first_body = IFAdapter.OrderedBody();
  if( !IFAdapter.MakeBody() )
   return; 
  //Additional work on body  
  if( drand48()>0.5 ){
   first_body->OGL_body->texture_ID = TEST_textid;
   size_t UVsize = first_body->OGL_body->UVmapping_cnt;
   for (int cntuv = 0; cntuv < UVsize; cntuv++) {
    first_body->OGL_body->UVmapping[cntuv] *= TEST_text_ut;
    cntuv++;
    first_body->OGL_body->UVmapping[cntuv] *= TEST_text_vt;
   }

  }else{
   first_body->OGL_body->texture_ID = User_Data.CubeTexture; //DrawText(outstring, 4, 0);
  }
 }
}

void Init_IFAdapter(engine &engine) {
 if (engine.EGL_initialized) {
  //static int counter = 0;
  //counter++;
  //-------------------------------------------------------     IFEngine TEST
  IFAdapter.MakeWorld(0.0f, -0.00f);
  //Smallest object box2d can deal with optimally is 0.1 in box coords, so we want smallest of elements to be 1 pixel. This factor will affect zoom in/out
  IFAdapter.screenResolutionX = engine.width;
  IFAdapter.screenResolutionY = engine.height;
  IFAdapter.CalculateBox2DSizeFactor(1000*drand48()+500);

  int twidth, theight;
  GLuint texint;

  ((TS_User_Data*)p_user_data)->CubeTexture = IFEUtilsLoadTexture::png_texture_load("testcube.png", &twidth, &theight);

  FANN_TEST_initialized = false;
  anns_body = anns_learned_body = NULL;

  return;

  IFAdapter.OrderBody();
  IFAdapter.OrderedBody()->body_def->type = b2_staticBody;
  IFAdapter.OrderedBody()->body_def->position.Set(-0.0,5.0);
  b2PolygonShape *polyShape = new b2PolygonShape;
  b2Vec2 shapeCoords[8];

  //shapeCoords[0] = { 0.8,  0.125 };
  //shapeCoords[1] = { 0.1,   0.05 };
  //shapeCoords[2] = { 0.2,   0.025 };
  //shapeCoords[3] = { 0.30,   0.05 };
  //shapeCoords[4] = { 0.40,  0.12 };
  //shapeCoords[5] = { 0.30,  0.20 };
  //shapeCoords[6] = { 0.20,  0.27 };
  //shapeCoords[7] = { 0.10,  0.2 };

  shapeCoords[0] = { -1,  -1 };
  shapeCoords[1] = {  16,  -1 };
  shapeCoords[2] = {  16,   16 };
  shapeCoords[3] = { -1,   16 };
  //shapeCoords[4] = { 4,  1 };
  //shapeCoords[5] = { 3,  2 };
  //shapeCoords[6] = { 2,  2 };
  //shapeCoords[7] = { 1,  2 };

  //shapeCoords[0] = {  8,  12.5 };
  //shapeCoords[1] = { 10.0,   5.0 };
  //shapeCoords[2] = { 20.0,   2.5 };
  //shapeCoords[3] = { 30.0,   5.0 };
  //shapeCoords[4] = { 40.0,  12.5 };
  //shapeCoords[5] = { 30.0,  20.0 };
  //shapeCoords[6] = { 20.0,  27.0 };
  //shapeCoords[7] = { 10.0,  20.0 };
  polyShape->Set(shapeCoords, 4);
  b2FixtureDef *fixture = new b2FixtureDef;
  fixture->shape = polyShape;
  fixture->density = 0.1;
  fixture->friction = 0.3;
  IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape, fixture);
  ifCB2Body *first_body = IFAdapter.OrderedBody();
  if (IFAdapter.MakeBody()){
   //Additional work on body  
   SetFaceSize(40*64, 40 * 64);
   char outstring[20] = {'R','j','b',',','g','y','\0'};
   //TEST_textid = first_body->OGL_body->texture_ID = DrawText(outstring, 5, FT_Vector()={160*64,40*64}, 3.141593*0.50, &TEST_text_ub, &TEST_text_vb, &TEST_text_ut, &TEST_text_vt);
   TEST_textid = first_body->OGL_body->texture_ID = DrawText(outstring, 5, FT_Vector() = { 160 * 64,40 * 64 }, 3.141593*0.250, &TEST_text_ub, &TEST_text_vb, &TEST_text_ut, &TEST_text_vt);
   //TEST_textid = first_body->OGL_body->texture_ID = DrawText(outstring, 5, FT_Vector() = { 40 * 64,60 * 64 }, 3.141593*0.0, &TEST_text_ub, &TEST_text_vb, &TEST_text_ut, &TEST_text_vt);
   size_t UVsize = first_body->OGL_body->UVmapping_cnt;
   for( int cntuv = 0; cntuv < UVsize; cntuv++){
    first_body->OGL_body->UVmapping[cntuv]*= TEST_text_ut;
    cntuv++;
    first_body->OGL_body->UVmapping[cntuv] *= TEST_text_vt;
   }
   //(engine.width / 20, engine.height / 20);
  }


//
//
//
////////////////////////////second body
  IFAdapter.OrderBody();
  IFAdapter.OrderedBody()->body_def->type = b2_staticBody;
  IFAdapter.OrderedBody()->body_def->position.Set(-3.0, -3.0);
  polyShape = new b2PolygonShape;
  shapeCoords[0] = b2Vec2(0.0, -0.4);
  shapeCoords[1] = b2Vec2( static_cast<float32>(engine.width) / 30.0, -0.4);
  shapeCoords[2] = b2Vec2( static_cast<float32>(engine.width) / 30.0, 0.4);
  shapeCoords[3] = b2Vec2(0.0, 0.4);
  polyShape->Set(shapeCoords, 4);
  fixture = new b2FixtureDef;
  fixture->shape = polyShape;
  fixture->density = 10.1;
  fixture->friction = 0.3;
  fixture->restitution = 0.01;
  IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape, fixture);
  first_body = IFAdapter.OrderedBody();
  if (IFAdapter.MakeBody()) {
   //Additional work on body  
   //SetFaceSize(1000, 1000);
   //char *outstring = { "#" };
   texint = ((TS_User_Data*)p_user_data)->CubeTexture;// = IFEUtilsLoadTexture::png_texture_load("testcube.png", &twidth, &theight);
   first_body->OGL_body->texture_ID = User_Data.CubeTexture; //DrawText(outstring, 4, 0);
   // first_body->body_def->position.Set(0, -100.0);
  }



/////////////////////////third body
  IFAdapter.OrderBody();
  IFAdapter.OrderedBody()->body_def->type = b2_dynamicBody;
  IFAdapter.OrderedBody()->body_def->position.Set(-2, 10.0);
  polyShape = new b2PolygonShape;
  shapeCoords[0] = b2Vec2(0.0, 0.0);
  shapeCoords[1] = b2Vec2(static_cast<float32>(engine.width) / 100.0, 0.0);
  shapeCoords[2] = b2Vec2(static_cast<float32>(engine.width) / 100.0, 0.8);
  shapeCoords[3] = b2Vec2(0.0, 0.8);
  polyShape->Set(shapeCoords, 4);
  fixture = new b2FixtureDef;
  fixture->shape = polyShape;
  fixture->density = 0.1;
  fixture->friction = 0.0001;
  IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape, fixture);
  first_body = IFAdapter.OrderedBody();
  if (IFAdapter.MakeBody()) {
   //Additional work on body  
   //SetFaceSize(1000, 1000);
   //char *outstring = { "#" };
   //int twidth, theight;
   ((TS_User_Data*)p_user_data)->CubeTexture = texint;//IFEUtilsLoadTexture::png_texture_load("testcube.png", &twidth, &theight);
   first_body->OGL_body->texture_ID = User_Data.CubeTexture; //DrawText(outstring, 4, 0);
   //first_body->body_def->position.Set(0, -100.0);
  }

  //-------------------------------------------------------     IFEngine TEST
 }
}





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

	EGLDisplay display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
	if( display == EGL_NO_DISPLAY ) {
  LOGW("EGL Init Error 1");
  return -1;
 }

 EGLint verMaj, verMin;
	if( !eglInitialize( display, &verMaj, &verMin) ) {
  LOGW("EGL Init Error 2");
  return -1;
 }

	/* Here, the application chooses the configuration it desires. In this
	* sample, we have a very simplified selection process, where we pick
	* the first EGLConfig that matches our criteria */
	if( EGL_FALSE == eglChooseConfig( display, attribs, &config, 1, &numConfigs ) ) {
  LOGW("EGL Init Error 3");
  return -1;
 }

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	* guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	* As soon as we picked a EGLConfig, we can safely reconfigure the
	* ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	if( !eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format) ) {
  LOGW("EGL Init Error 4");
  return -1;
 }

	if( ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format) < 0 ) {
  LOGW("EGL Init Error 5");
  return -1;
 }

 if( EGL_NO_SURFACE == ( surface = eglCreateWindowSurface(display, config, engine->app->window, NULL) ) ) {
  LOGW("EGL Init Error 6");
  return -1;
 }
	if( EGL_NO_CONTEXT == ( context = eglCreateContext(display, config, NULL, NULL) ) ){
  LOGW("EGL Init Error 7");
  return -1;
 }

	if( eglMakeCurrent(display, surface, surface, context) == EGL_FALSE ) {
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
 Setup_OpenGL(w,h);

 Init_IFAdapter(*engine);

 IFAudioSLES::BuildAudioEngine(User_Data.state->activity);

	return 0;
}

/**
* Just the current frame in the display.
*/

static void engine_draw_frame(struct engine* engine) {
	if (engine->display == NULL) {
		// No display.
		return;
	}


 //cleanup far bodies
 //B2BodyUtils.RemoveFarBodies();

 TESTFN_AddRandomBody(*engine);


 PrepareDraw();


 IFAdapter.UpdateSim();
 IFAdapter.UpdateGraphics();

 TESTFN_PostOperations(*engine);

	//CubeTest_draw();
 DrawBodies();

	eglSwapBuffers(engine->display, engine->surface);
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
 glDeleteTextures(1, &TEST_textid);
 TEST_textid = GL_INVALID_VALUE;
 glDeleteTextures(1, &User_Data.CubeTexture);
 User_Data.CubeTexture = GL_INVALID_VALUE; 
 glDeleteTextures(10, TEST_GUI_Tex_Ary);
 memset(TEST_GUI_Tex_Ary, GL_INVALID_VALUE,sizeof(TEST_GUI_Tex_Ary[0])*sizeof(TEST_GUI_Tex_Ary));


 IFAudioSLES::TearDownAudioEngine();

 if(FANN_TEST_initialized){
  IFFANN::Save_Cascade_FANN(&LittleBrains, IFFANN::CnTrainedFannPostscript);
 }

 last_touched_object = NULL;

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
   User_Data.Touch_Input_X = engine->state.x;
   User_Data.Touch_Input_Y = engine->state.y;
   User_Data.Event_Indicator_Touch_Input++;
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
				engine->accelerometerSensor, (1000L / 60) * 1000);
		}
  if (engine->sensor2 != NULL) {
   ASensorEventQueue_enableSensor(engine->sensorEventQueue2,
    engine->sensor2);
   // We'd like to get 60 events per second (in us).
   ASensorEventQueue_setEventRate(engine->sensorEventQueue2,
    engine->sensor2, (1000L / 60) * 1000);
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
    engine->sensor2 );
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
 if( !InitFreeType(state) ){
  return;
 }

 

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
	engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,	ASENSOR_TYPE_ACCELEROMETER);
	engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager, state->looper, LOOPER_ID_USER, NULL, NULL);

 engine.sensorManager2 = ASensorManager_getInstance();
 engine.sensor2 = ASensorManager_getDefaultSensor(engine.sensorManager2,	ASENSOR_TYPE_LIGHT);
 engine.sensorEventQueue2 = ASensorManager_createEventQueue(engine.sensorManager2, state->looper, LOOPER_ID_USER+1, NULL, NULL);


 p_user_data = &User_Data;



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
			if (ident == LOOPER_ID_USER ) {
				if (engine.accelerometerSensor != NULL) {
					ASensorEvent event;
					while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
						&event, 1) > 0) {
						//LOGI("accelerometer: x=%f y=%f z=%f",
						//	event.acceleration.x, event.acceleration.y,
						//	event.acceleration.z);
      last_x_acceleration = event.acceleration.x;
      last_y_acceleration = event.acceleration.y;
					}
				}
			} else if (ident == (LOOPER_ID_USER + 1)) {
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
    DoneFreeType();
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
