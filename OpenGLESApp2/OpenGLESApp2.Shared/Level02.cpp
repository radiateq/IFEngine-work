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
 void RemoveFarBodies();

 void CRacerLevel::PostOperations() {
 }
 void CRacerLevel::AdvanceGame() {
  if(game_initialized){
   
   ProcessUserInput();
   RemoveFarBodies();

   if(ifbodies[2]->body->GetPosition().y<(bottom*1.3 )){
    ifbodies[2]->body->SetTransform(b2Vec2(drand48()*(right-left)-right, top*1.3 ), 0.0);
    ifbodies[2]->body->SetLinearVelocity(b2Vec2(0.0, -9.0));
   }





   return;
  }
  game_initialized = true;  
  {
   thickness = RQNDKUtils::getDensityDpi(User_Data.state) / 25.4;
   left = 0; bottom = game_engine->height; right = game_engine->width; top = 0;
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



   //Rock
   IFAdapter.OrderBody();
   IFAdapter.OrderedBody()->body_def->type = b2_dynamicBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :    b2_kinematicBody
   polyShape2 = new b2PolygonShape;
   shapeCoords[0].x = 0.0, shapeCoords[0].y = 0.0;
   shapeCoords[1].x = -7.9, shapeCoords[1].y = -7.9;
   shapeCoords[2].x = -10.0, shapeCoords[2].y = -12.6;
   shapeCoords[3].x = -10.0, shapeCoords[3].y = -15.3;
   shapeCoords[4].x = 13.9, shapeCoords[4].y = -15.3;
   shapeCoords[5].x = 11, shapeCoords[5].y = -10.0;
   shapeCoords[6].x = 9.8, shapeCoords[6].y = -6.2;
   shapeCoords[7].x = 3.8, shapeCoords[7].y = -1.4;
   zoom_factor = 0.8;
   for (unsigned int cnt = 0; cnt < 8; cnt++) {
    shapeCoords[cnt].x *= zoom_factor, shapeCoords[cnt].y *= zoom_factor;
   }
   polyShape2->Set(shapeCoords, 8);
   fixture = new b2FixtureDef;
   fixture->shape = polyShape2;
   fixture->density = 1.0;
   fixture->friction = 0.0;
   fixture->restitution = 1.010;
   //fixture->filter.categoryBits = 0x0008;
   //fixture->filter.maskBits = 0x0010;
   IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape2, fixture);
   ifbodies[2] = IFAdapter.OrderedBody();
   if (!IFAdapter.MakeBody())
    return;
   ifbodies[2]->body->SetFixedRotation(true);
   ifbodies[2]->body->SetTransform(b2Vec2(0.0, 0.0), 0.0);
   ifbodies[2]->OGL_body->texture_ID = IFEUtilsLoadTexture::png_texture_load("rock.png", &twidth, &theight);



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

  for( unsigned int cnt = 0; cnt < 3; cnt++){
   glDeleteTextures(1, &ifbodies[2]->OGL_body->texture_ID);
  }
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
     float desired_y = (screeny) / IFA_box2D_factor - ifbodies[0]->body->GetPosition().y + 30.0;
     ifbodies[0]->body->SetLinearVelocity(
      b2Vec2(desired_x*desired_x*desired_x, desired_y*desired_y*desired_y));
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

 void RemoveFarBodies(){
  float maxx = User_Data.screenWidth;
  float maxy = User_Data.screenHeight;
  Window2ObjectCoordinates(maxx, maxy, zDefaultLayer, maxx, maxy);
  float cut_off_distance = b2Distance(b2Vec2(0, 0), b2Vec2(maxx / IFA_box2D_factor, maxy / IFA_box2D_factor)) * 2.0;

  for (typename std::list<ifCB2Body*>::iterator iter = IFAdapter.Bodies.begin(); iter != IFAdapter.Bodies.end(); iter++) {
   b2Vec2 position = (*iter)->body->GetPosition();
   if(b2Distance(b2Vec2(position.x, position.y), b2Vec2(0,0))> cut_off_distance){
    //Reset the ball to the center
    (*iter)->body->SetTransform(b2Vec2(0, 0), (*iter)->body->GetAngle());
    (*iter)->body->SetLinearVelocity(b2Vec2(0, 0));
    (*iter)->body->SetAngularVelocity(0);
   }
  }
 }

}

