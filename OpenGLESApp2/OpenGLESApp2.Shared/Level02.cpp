#include "Level02.h"

#include "./Common/External/AudioFile/AudioFile.h"



Level02::CRacerLevel RacerLevel;

extern ifCB2Adapter IFAdapter;
extern ifCB2BodyUtils B2BodyUtils;
ifCB2Body *ifbodies[100];





namespace Level02{


 class CSensorWav {
 public:
  AudioFile<double> audio_file;
  unsigned int num_channels, num_samples, sample_rate;
  AudioFile<double>::AudioBuffer buffer;
  CSensorWav() {
   sample_rate = 44100;
   num_channels = 2;
   buffer.resize(num_channels);
   num_samples = 0;
  }
  void SetBufferSize(const unsigned int samples_count) {
   /////////////////////////
   // // // Buffer Size 
   /////////////////////////////
   num_samples = samples_count;
   buffer[0].resize(num_samples);
   buffer[1].resize(num_samples);
  }
  //////////////////////////////////////
  // // //          Test Signal
  //////////////////////////////////////
  void TestTone() {
   float _sample_rate = sample_rate;
   float frequency = 440.f;
   for (int i = 0; i < num_samples; i++)
   {
    float sample = sinf(2. * M_PI * ((float)i / _sample_rate) * frequency);

    for (int channel = 0; channel < num_channels; channel++)
     buffer[channel][i] = sample * 0.5;
   }
  }
  void AddSample(float sampleL, float sampleR) {
   if (buffer[0].size() <= num_samples) {
    buffer[0].push_back(sampleL);
    buffer[1].push_back(sampleR);
   }
  }
  void SaveFile(char const * const _file_name) {
   audio_file.setAudioBuffer(buffer);
   // Set both the number of channels and number of samples per channel
   audio_file.setAudioBufferSize(num_channels, num_samples);
   // Set the number of samples per channel
   audio_file.setNumSamplesPerChannel(num_samples);
   // Set the number of channels
   audio_file.setNumChannels(num_channels);
   audio_file.setBitDepth(24);
   audio_file.setSampleRate(sample_rate);
   char file_name[BUFSIZ + 1];
   RQNDKUtils::Make_storageDataPath(file_name, BUFSIZ, _file_name);
   audio_file.save(file_name);
  }
 };
 CSensorWav SensorWav;





 bool game_initialized = false;

 int touchx, touchy;
 unsigned long int Last_GUI_Click_Time = 0;
 void ProcessUserInput();
 float DeltaTime();//Function returns delta time between calls in milliseconds


 void CRacerLevel::PostOperations() {
  if (game_initialized) {
  }
 }






 void CRacerLevel::AdvanceGame() {
  if(game_initialized){
   //User_Data.accel[0].push_back(1.1 / 8.f);
   //User_Data.accel[1].push_back(1.1 / 8.f);

   unsigned int repeat_count = 10;
   SensorWav.SetBufferSize(SensorWav.buffer[0].size() + User_Data.accel[0].size() * repeat_count);
   for( size_t cnt = 0; cnt < User_Data.accel[0].size(); cnt++){       
    for(unsigned int rep_cnt = 0; rep_cnt < repeat_count; rep_cnt++){
     SensorWav.AddSample(User_Data.accel[0].at(cnt), User_Data.accel[1].at(cnt));
    }
   }
   if(User_Data.accel[0].size()){
    User_Data.accel[0].clear();
    User_Data.accel[1].clear();
   }


   ProcessUserInput();
   return;
  }
  game_initialized = true;  
  if (User_Data.accel[0].size()) {
   User_Data.accel[0].clear();
   User_Data.accel[1].clear();
  }
 }

 void CRacerLevel::Init_IFAdapter() {
  if (game_engine->EGL_initialized) {
   IFAdapter.MakeWorld(0.0f, 0.00f);
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
  SensorWav.SaveFile("shake.wav");
 }


 void ProcessUserInput(){
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
     unsigned int n = 0;
     //float desired_x = (screenx) / IFA_box2D_factor - ifbodies[n]->body->GetPosition().x;
     //float desired_y = (screeny) / IFA_box2D_factor - ifbodies[n]->body->GetPosition().y + 20.0;
     //ifbodies[n]->body->SetTransform(b2Vec2(desired_x, desired_y),0);
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


 unsigned long int last_DeltaTime_time;
 float DeltaTime(){
  timespec temp_timespec;
  unsigned long int ret_val;
  unsigned long int temp_time;
  clock_gettime(CLOCK_MONOTONIC, &temp_timespec);  
  temp_time = RQNDKUtils::timespec2us64(&temp_timespec);
  ret_val = temp_time - last_DeltaTime_time;
  last_DeltaTime_time = temp_time;
  return (float)ret_val/1000.0;
 }


}