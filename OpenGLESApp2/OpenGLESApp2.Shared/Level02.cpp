#include "Level02.h"
#include "Level01.h"


Level02::CRacerLevel RacerLevel;

extern ifCB2Adapter IFAdapter;
extern ifCB2BodyUtils B2BodyUtils;
ifCB2Body *ifbodies[100];


namespace Level02{

 bool game_initialized = false;

 float thickness;
 float left, bottom, right, top;

 int touchx, touchy;
 unsigned long int Last_GUI_Click_Time = 0;
 void ProcessUserInput();
 
 void CRacerLevel::PostOperations() {
 }
 void CRacerLevel::AdvanceGame() {
  if(game_initialized){
   
   ProcessUserInput();












   return;
  }
  game_initialized = true;  
  {
   thickness = RQNDKUtils::getDensityDpi(User_Data.state) / 25.4;
   left = -thickness * 0.5; bottom = 0; right = game_engine->width; top = game_engine->height;
   Window2ObjectCoordinates(left, bottom, zDefaultLayer, game_engine->width, game_engine->height);
   left /= IFA_box2D_factor; bottom /= IFA_box2D_factor;
   Window2ObjectCoordinates(right, top, zDefaultLayer, game_engine->width, game_engine->height);
   right /= IFA_box2D_factor; top /= IFA_box2D_factor;

   int twidth, theight;

   b2EdgeShape *edgeShape;
   b2FixtureDef *fixture;
   b2PolygonShape *polyShape2;
   b2Vec2 shapeCoords[8];
   float zoom_factor = 2.0;

   //Player Boat
   IFAdapter.OrderBody();
   IFAdapter.OrderedBody()->body_def->type = b2_kinematicBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :    
   polyShape2 = new b2PolygonShape;
   shapeCoords[0].x = 0.0, shapeCoords[0].y = 0.0;
   shapeCoords[1].x = -1.7, shapeCoords[1].y = -2.3;
   shapeCoords[2].x = -1.2, shapeCoords[2].y = -7.8;
   shapeCoords[3].x = 1.2, shapeCoords[3].y = -7.8;
   shapeCoords[4].x = 1.7, shapeCoords[4].y = -2.3;
   for( unsigned int cnt = 0; cnt < 5; cnt++){
    shapeCoords[cnt].x *= zoom_factor, shapeCoords[cnt].y *= zoom_factor;
   }
   polyShape2->Set(shapeCoords, 5);
   fixture = new b2FixtureDef;
   fixture->shape = polyShape2;
   fixture->density = 1.0;
   fixture->friction = 0.0;
   fixture->restitution = 1.010;
   //fixture->filter.categoryBits = 0x0008;
   //fixture->filter.maskBits = 0x0010;
   IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape2, fixture);
   ifbodies[0] = IFAdapter.OrderedBody();
   if (!IFAdapter.MakeBody())
    return;
   ifbodies[0]->body->SetTransform(b2Vec2(0.0, -bottom * 0.4), 0.0);
   ifbodies[0]->OGL_body->texture_ID = IFEUtilsLoadTexture::png_texture_load("boat.png", &twidth, &theight);



   //AI Boat
   IFAdapter.OrderBody();
   IFAdapter.OrderedBody()->body_def->type = b2_dynamicBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :   b2_kinematicBody 
   polyShape2 = new b2PolygonShape;
   polyShape2->Set(shapeCoords, 5);
   fixture = new b2FixtureDef;
   fixture->shape = polyShape2;
   fixture->density = 1.0;
   fixture->friction = 0.0;
   fixture->restitution = 1.010;
   //fixture->filter.categoryBits = 0x0008;
   //fixture->filter.maskBits = 0x0010;
   IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape2, fixture);
   ifbodies[1] = IFAdapter.OrderedBody();
   if (!IFAdapter.MakeBody())
    return;
   ifbodies[1]->body->SetTransform(b2Vec2(0.0, -bottom * 0.4), 0.0);
   ifbodies[1]->OGL_body->texture_ID = IFEUtilsLoadTexture::png_texture_load("boat.png", &twidth, &theight);




  }  
 }

 void CRacerLevel::Init_IFAdapter() {
  if (game_engine->EGL_initialized) {
   IFAdapter.MakeWorld(0.0f, -0.00f);
   //Smallest object box2d can deal with optimally is 0.1 in box coords, so we want smallest of elements to be 1 pixel. This factor will affect zoom in/out
   IFAdapter.screenResolutionX = game_engine->width;
   IFAdapter.screenResolutionY = game_engine->height;
   IFAdapter.CalculateBox2DSizeFactor(30);
   //IFEUtilsLoadTexture::png_texture_load("testcube.png", &twidth, &theight);
   //IFEUtilsLoadTexture::png_texture_load("ball.png", &twidth, &theight);
  }
 }

 void CRacerLevel::Setup_OpenGL(double width, double height) {
  User_Data.screenWidth = width;
  User_Data.screenHeight = height;
  //// Initialize GL state for 3D
  //glDisable(GL_DITHER);
  //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  //glClearColor(1.0f, 0.41f, 0.71f, 1.0f);
  //glEnable(GL_CULL_FACE);
  //glShadeModel(GL_SMOOTH);
  //glEnable(GL_DEPTH_TEST);

  //glViewport(0, 0, width, height);
  //GLfloat ratio = (GLfloat)width / height;
  //glMatrixMode(GL_PROJECTION);
  //glLoadIdentity();
  //glFrustumf(-ratio, ratio, -1, 1, 1, 10);





  // Initialize GL state for 2D
  //glDisable(GL_TEXTURE_2D);
  //glDisable(GL_DITHER);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glClearColor(0.600f, 0.600, 0.600, 1.0f);
  //glDisable(GL_CULL_FACE); //     ORIGINAL VALUE glEnable(GL_CULL_FACE);
  glShadeModel(GL_SMOOTH);
  //glDisable(GL_DEPTH_TEST); //     ORIGINAL VALUE glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glDisable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
  glFrontFace(GL_CCW);

  //glEnable(GL_FOG); 
  //glFogf(GL_FOG_MODE, GL_LINEAR);
  //glFogf(GL_FOG_DENSITY, 0.1);
  //float fog_color[4];
  //fog_color[0] = 1.0f;
  //fog_color[1] = 1.0f;
  //fog_color[2] = 1.0f;
  //fog_color[3] = 0.5f;
  //glFogfv(GL_FOG_COLOR, fog_color);

  IFEUtils::zvnear = 0.0;
  IFEUtils::zvfar = 1.0;
  glDepthRangef(IFEUtils::zvnear, IFEUtils::zvfar);
  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  IFEUtils::ratio = (GLfloat)width / (GLfloat)height;
  if (IFEUtils::ratio > 1.0f) {
   IFEUtils::ratio = (GLfloat)height / (GLfloat)width;
   IFEUtils::left = -1.0;
   IFEUtils::right = 1.0;
   IFEUtils::bottom = -IFEUtils::ratio;
   IFEUtils::top = IFEUtils::ratio;
  }
  else {
   IFEUtils::left = -IFEUtils::ratio;
   IFEUtils::right = IFEUtils::ratio;
   IFEUtils::bottom = -1.0;
   IFEUtils::top = 1.0;
  }
  IFEUtils::znear = 0.1;
  IFEUtils::zfar = 100;

  glFrustumf(IFEUtils::left, IFEUtils::right, IFEUtils::bottom, IFEUtils::top, IFEUtils::znear, IFEUtils::zfar);


  glMatrixMode(GL_MODELVIEW);
 }


 void CRacerLevel::Cleanup() {
  game_initialized = false;
 }


 void ProcessUserInput(){


  ifbodies[0]->body->SetLinearVelocity(b2Vec2(0, 0));
  if (IFGameEditor::GetTouchEvent()) {
   struct timespec temp_timespec;
   clock_gettime(CLOCK_MONOTONIC, &temp_timespec);
   unsigned long int input_event_time_stamp;
   {
    input_event_time_stamp = IFGameEditor::GetTouchEvent(&touchx, &touchy);
    {
     //Move players paddle
     float screenx = touchx;
     float screeny = touchy;
     Window2ObjectCoordinates(screenx, screeny, zDefaultLayer, User_Data.screenWidth, User_Data.screenHeight);
     float desired_x = (screenx) / IFA_box2D_factor - ifbodies[0]->body->GetPosition().x;
     ifbodies[0]->body->SetLinearVelocity(
      b2Vec2(desired_x*desired_x*desired_x, 0.0));
     //Player may press train button but not more often than once per second       
     if ((input_event_time_stamp - Last_GUI_Click_Time) > 1000000000) {
      Last_GUI_Click_Time = input_event_time_stamp;
      typename std::list<ifCB2Body*>::iterator iter;
      for (iter = IFAdapter.Bodies.begin(); iter != IFAdapter.Bodies.end(); iter++) {
       if (ifbodies[0] == *iter) {
        if (B2BodyUtils.RayTestHitpoint(touchx, touchy, *iter)) {
        }
       }
      }
     }
    }
   }
  }
 }


}

