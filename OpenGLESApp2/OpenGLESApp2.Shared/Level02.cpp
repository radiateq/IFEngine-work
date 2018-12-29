#include "Level02.h"
#include "Level01.h"


Level02::CRacerLevel RacerLevel;

extern ifCB2Adapter IFAdapter;
extern ifCB2BodyUtils B2BodyUtils;


namespace Level02{


 
 
 void CRacerLevel::PostOperations() {
 }
 void CRacerLevel::AdvanceGame() {
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
 void CRacerLevel::Cleanup() {
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
}
