#include "Level02.h"
#include "Level01.h"


Level02::CRacerLevel RacerLevel;

extern ifCB2Adapter IFAdapter;
extern ifCB2BodyUtils B2BodyUtils;
ifCB2Body *ifbodies[100];


namespace Level02{

 bool game_initialized = false;
 float boat_width, boat_height, rock_width, rock_height;
 void MakeBodies();

 float thickness;
 float left, bottom, right, top;

 int touchx, touchy;
 unsigned long int Last_GUI_Click_Time = 0;
 void ProcessUserInput();
 void RemoveFarBodies();

 unsigned int first_rock, last_rock;
 bool boat_hit[2] = {false};
 b2Vec2 last_boat_position[2];
 b2Vec2 last_train_boat_position[2];
 void ProcessBoatsAndRocks();
 float DeltaTime();//Function returns delta time between calls in milliseconds

 IFFANNEngine::CNetwork Network;
 const unsigned int Max_Network_Nodes = 10;
 CNetworkNode NetworkNodes[Max_Network_Nodes];
 void ConnectAI();
 void TrainBoatAI();
 void RunBoatAI();

 void CRacerLevel::PostOperations() {
  if (game_initialized) {
   last_boat_position[0] = ifbodies[1]->body->GetPosition();
   last_boat_position[1] = last_boat_position[0];
  }
 }
 void CRacerLevel::AdvanceGame() {
  if(game_initialized){
   
   ProcessUserInput();
   RemoveFarBodies();
  
   ProcessBoatsAndRocks();

   return;
  }
  game_initialized = true;  
  MakeBodies();  
  ConnectAI();
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
  NetworkNodes[0].Save_Fann();
  NetworkNodes[0].Save_Train_Data();
 }


 void ProcessUserInput(){


  ifbodies[0]->body->SetLinearVelocity(b2Vec2(0, 0));
  ifbodies[0]->body->SetAngularVelocity(0.0);
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
     float desired_x = (screenx) / IFA_box2D_factor - ifbodies[n]->body->GetPosition().x;
     float desired_y = (screeny) / IFA_box2D_factor - ifbodies[n]->body->GetPosition().y + 20.0;
//     ifbodies[n]->body->SetTransform(b2Vec2(desired_x, desired_y),0);
     ifbodies[n]->body->SetLinearVelocity(
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
  float cut_off_distance = b2Distance(b2Vec2(0, 0), b2Vec2(maxx / IFA_box2D_factor, maxy / IFA_box2D_factor)) * 2.5;

  for (typename std::list<ifCB2Body*>::iterator iter = IFAdapter.Bodies.begin(); iter != IFAdapter.Bodies.end(); iter++) {
   b2Vec2 position = (*iter)->body->GetPosition();
   if(b2Distance(b2Vec2(position.x, position.y), b2Vec2(0,0))> cut_off_distance){
    

    bool found = false;
    for (unsigned int cnt = first_rock; cnt <= last_rock; cnt++) {
     if (ifbodies[cnt] == (*iter)) {
      //In order to increase distance between bodies we don't instantiate them first time
      if(drand48()>0.987){
       ifbodies[cnt]->body->SetTransform(b2Vec2(drand48()*(right - left) - right, top + top * (drand48() + 1.0)*1.7), 0.0);
       ifbodies[cnt]->body->SetLinearVelocity(b2Vec2(0.0, -9.0));
      }
      found = true;
      //TrainBoatAI();
      break;
     }
    }
    if(!found){
     (*iter)->body->SetTransform(b2Vec2(0, 0), (*iter)->body->GetAngle());
     (*iter)->body->SetLinearVelocity(b2Vec2(0, 0));
     (*iter)->body->SetAngularVelocity(0);
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
 void MakeBodies(){
  thickness = RQNDKUtils::getDensityDpi(User_Data.state) / 25.4;
  left = 0; bottom = User_Data.screenHeight; right = User_Data.screenWidth; top = 0;
  Window2ObjectCoordinates(left, bottom, zDefaultLayer, User_Data.screenWidth, User_Data.screenHeight);
  left /= IFA_box2D_factor; bottom /= IFA_box2D_factor;
  Window2ObjectCoordinates(right, top, zDefaultLayer, User_Data.screenWidth, User_Data.screenHeight);
  right /= IFA_box2D_factor; top /= IFA_box2D_factor;

  int twidth, theight;
 
  b2EdgeShape *edgeShape;
  b2FixtureDef *fixture;
  b2PolygonShape *polyShape2;
  b2Vec2 shapeCoords[8];
  float zoom_factor = 2.0;

  //Player Boat
  IFAdapter.OrderBody();
  IFAdapter.OrderedBody()->body_def->type = b2_dynamicBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :   b2_kinematicBody
  polyShape2 = new b2PolygonShape;  
  shapeCoords[0].x = 0.0, shapeCoords[0].y = 0.0;
  shapeCoords[1].x = -1.7, shapeCoords[1].y = -2.3;
  shapeCoords[2].x = -1.2, shapeCoords[2].y = -7.8;
  shapeCoords[3].x = 1.2, shapeCoords[3].y = -7.8;
  shapeCoords[4].x = 1.7, shapeCoords[4].y = -2.3;
  for (unsigned int cnt = 0; cnt < 5; cnt++) {
   shapeCoords[cnt].x *= zoom_factor, shapeCoords[cnt].y *= zoom_factor;
  }
  boat_width = abs(shapeCoords[4].x - shapeCoords[1].x);
  boat_height = abs(shapeCoords[3].y);
  polyShape2->Set(shapeCoords, 5);  
  fixture = new b2FixtureDef;
  fixture->isSensor = true;
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
  //ifbodies[0]->body->SetFixedRotation(0.0);
  ifbodies[0]->OGL_body->texture_ID = IFEUtilsLoadTexture::png_texture_load("boat.png", &twidth, &theight);



  //AI Boat
  IFAdapter.OrderBody();
  IFAdapter.OrderedBody()->body_def->type = b2_dynamicBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :   b2_kinematicBody 
  polyShape2 = new b2PolygonShape;
  polyShape2->Set(shapeCoords, 5);
  fixture = new b2FixtureDef;
  fixture->isSensor = true;
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



  //Rocks
  IFAdapter.OrderBody();
  IFAdapter.OrderedBody()->body_def->type = b2_kinematicBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :    b2_kinematicBody
  polyShape2 = new b2PolygonShape;
  shapeCoords[0].x = 0.0, shapeCoords[0].y = 0.0;
  shapeCoords[1].x = -7.9, shapeCoords[1].y = -7.9;
  shapeCoords[2].x = -10.0, shapeCoords[2].y = -12.6;
  shapeCoords[3].x = -10.0, shapeCoords[3].y = -15.3;
  shapeCoords[4].x = 13.9, shapeCoords[4].y = -15.3;
  shapeCoords[5].x = 11, shapeCoords[5].y = -10.0;
  shapeCoords[6].x = 9.8, shapeCoords[6].y = -6.2;
  shapeCoords[7].x = 3.8, shapeCoords[7].y = -1.4;
  zoom_factor = 0.3;
  for (unsigned int cnt = 0; cnt < 8; cnt++) {
   shapeCoords[cnt].x *= zoom_factor, shapeCoords[cnt].y *= zoom_factor;
  }
  rock_width = shapeCoords[4].x - shapeCoords[3].x;
  rock_height = shapeCoords[3].y;
  polyShape2->Set(shapeCoords, 8);
  fixture = new b2FixtureDef;
  fixture->isSensor = true;
  fixture->shape = polyShape2;
  fixture->density = 1.0;
  fixture->friction = 0.0;
  fixture->restitution = 1.010;
  //fixture->filter.categoryBits = 0x0008;
  //fixture->filter.maskBits = 0x0010;
  IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape2, fixture);
  first_rock = 2;
  ifbodies[first_rock] = IFAdapter.OrderedBody();
  if (!IFAdapter.MakeBody())
   return;
  ifbodies[first_rock]->body->SetFixedRotation(true);
  ifbodies[first_rock]->body->SetTransform(b2Vec2(0.0, 0.0), 0.0);
  ifbodies[first_rock]->OGL_body->texture_ID = IFEUtilsLoadTexture::png_texture_load("rock.png", &twidth, &theight);

  last_rock = 17;
  for (unsigned int cnt = first_rock + 1; cnt <= last_rock; cnt++) {
   IFAdapter.OrderBody();
   IFAdapter.OrderedBody()->body_def->type = b2_kinematicBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :    
   IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape2, fixture);
   ifbodies[cnt] = IFAdapter.OrderedBody();
   if (!IFAdapter.MakeBody())
    return;
   ifbodies[cnt]->body->SetFixedRotation(true);
   ifbodies[cnt]->body->SetTransform(b2Vec2(0.0, 0.0), 0.0);
   ifbodies[cnt]->OGL_body->texture_ID = IFEUtilsLoadTexture::png_texture_load("rock.png", &twidth, &theight);
  }
 }
 void ConnectAI(){
  //ifbodies[1]
  static float train_size_factor = 40.0;
  static float neuron_count_factor = 50.0;
  ////////////////////////////  Load Nodes START
  int n=0;
  NetworkNodes[n].FANNOptions.input_neurons = (last_rock - first_rock + 1)*3+2;
  NetworkNodes[n].FANNOptions.output_neurons = 2;
  NetworkNodes[n].FANNOptions.max_neurons = 10 * neuron_count_factor;
  NetworkNodes[n].FANNOptions.desired_error = 0.000;
  NetworkNodes[n].epoch_train_desired_eror = 1e-30;
  NetworkNodes[n].FANNOptions.input_scale = 0.01;
  NetworkNodes[n].FANNOptions.output_scale = 0.01;
  NetworkNodes[n].node_train_samples = NetworkNodes[n].node_data_counter_limit = 1000 * train_size_factor;
  NetworkNodes[n].Load_Node(Network, "boatandrocks");
  NetworkNodes[n].Load_Train_Data();
  NetworkNodes[n].Node->AddFlag(TNodeStates::E_Training);
  NetworkNodes[n].Node->AddFlag(TNodeStates::E_ContTraining);
 }
 bool Ifbodies_hit(unsigned int body_index, unsigned int body_test_index = -1){
  bool ret_val = false;
  for (b2ContactEdge* ce = ifbodies[body_index]->body->GetContactList(); ce && (boat_hit[body_index] == false); ce = ce->next)
  {
   b2Contact* c = ce->contact;
   b2Fixture *hit_body_fix;
   if (c->GetFixtureA()->GetBody() == ifbodies[body_index]->body) {
    hit_body_fix = c->GetFixtureB();
   }
   else {
    hit_body_fix = c->GetFixtureA();
   }
   if(body_test_index ==-1){
    for (unsigned int cnt = first_rock; cnt <= last_rock; cnt++) {
     if ((hit_body_fix->GetBody() == ifbodies[cnt]->body) && (c->IsTouching())) {
      ret_val = true;
      break;
     }
    }
   }else{
    if ((hit_body_fix->GetBody() == ifbodies[body_test_index]->body) && (c->IsTouching())) {
     ret_val = true;
     break;
    }
   }
   if (ret_val)
    break;
  }
  return ret_val;
 }

 void ProcessBoatsAndRocks(){
  float delta_t_ms = DeltaTime();
  static float last_train_delta = 0;
  last_train_delta += delta_t_ms;
  //if(last_train_delta> (delta_t_ms*10.0)){
  // last_train_delta = 0;
  // TrainBoatAI();
  //}
  static float wave_t = 0.0;
  float current_velocity = -20;/*sin(wave_t) * 20.0 + 30.0;  
  current_velocity = -current_velocity;
  if (2 * M_PI < wave_t) wave_t = 0;
  wave_t += (1.0 / 30000.0)*delta_t_ms * drand48() * 10.0;*/
  //unsigned int boat_body_index = 1;//0 player 1 AI
  current_velocity -= delta_t_ms * 0.0001;
  //////////////////////////////////////////////////////////
  /////     Return rocks from bottom to top
  //////////////////////////////////////////////////////////
  for (unsigned int cnt = first_rock; cnt <= last_rock; cnt++) {
   if (ifbodies[cnt]->body->GetPosition().y < (bottom*1.3)) {
    ifbodies[cnt]->body->SetTransform(b2Vec2(drand48()*(right - left) - right, top + top * (drand48() + 1.0)*1.7), 0.0);
   }
   ifbodies[cnt]->body->SetLinearVelocity(b2Vec2(0.0, current_velocity));
  }
  //////////////////////////////////////////////////////////
  /////  //         Check if boats are hitting rocks
  //////////////////////////////////////////////////////////
  for(unsigned int boat_body_index = 0; boat_body_index < 2; boat_body_index++){
   boat_hit[boat_body_index] = Ifbodies_hit(boat_body_index);
   //////////////////////////////////////////////////////////
   //     ///  //     If rock hit stop all rocks
   //////////////////////////////////////////////////////////
   if ((boat_hit[boat_body_index] == true)&&true) {
    if((boat_body_index>0)||((boat_body_index==0)&&(User_Data.pointer_down))){
     for (unsigned int cnt = first_rock; cnt <= last_rock; cnt++) {
      ifbodies[cnt]->body->SetLinearVelocity(b2Vec2(0.0, -2.0));
     }
    }
   }  
   ifbodies[boat_body_index]->body->SetTransform(ifbodies[boat_body_index]->body->GetPosition(), 0.0);
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //                                                      //
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  RunBoatAI();
  if(User_Data.pointer_down)
    TrainBoatAI();
  static unsigned int last_touch_indicator = 0;
  //if(last_touch_indicator != User_Data.Event_Indicator_Touch_Input ){   
  // last_touch_indicator = User_Data.Event_Indicator_Touch_Input;
  //}  
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 //                                                      //
 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 }
 
 void TrainBoatAI(){
  {
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //  /  /  /  /  /  /       Train Learn From Player AI      /  /  /  /  /  /
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
   unsigned int n = 0;
   b2Vec2 rock_position;
   b2Vec2 boat_position = ifbodies[0]->body->GetPosition();
   
   fann_type outputs[2];   
   bool any_rock_too_close = false;
   unsigned int rock_data_count = 3;
   float prev_x_position = left;
   IFGeneralUtils::SMapWrap<float, unsigned int> rocks_by_x;//x, index
   for (unsigned int cnt = first_rock; cnt <= last_rock; cnt++) {
    rock_position = ifbodies[cnt]->body->GetPosition();
    rocks_by_x.Add(rock_position.x, cnt);
   }
   rocks_by_x.ResetIterator();
   float x_coord;
   unsigned int rock_index;
   unsigned int cnt = first_rock;
   while(rocks_by_x.GetNextIterator(x_coord, rock_index)>0) {
    bool rock_too_close = false;
    rock_position = ifbodies[rock_index]->body->GetPosition();
    //if ((abs(boat_position.x - rock_position.x) < (boat_width * 1.0)) || (abs(boat_position.y - rock_position.y) < (boat_height * 1.0))) {
    if(Ifbodies_hit(0, rock_index)){
     rock_too_close = true;
     any_rock_too_close = true;
    }
    prev_x_position = rock_position.x;
    fann_type aim_position_x = (prev_x_position + (rock_position.x - prev_x_position)*0.5);
    NetworkNodes[n].Node->inputs[(cnt - first_rock) * rock_data_count + 0] = aim_position_x * NetworkNodes[n].FANNOptions.input_scale;
    NetworkNodes[n].Node->inputs[(cnt - first_rock) * rock_data_count + 1] = (rock_position.y) * NetworkNodes[n].FANNOptions.input_scale;
    NetworkNodes[n].Node->inputs[(cnt - first_rock) * rock_data_count + 2] = rock_too_close;
    //NetworkNodes[n].Node->inputs[(cnt - first_rock) * rock_data_count + 2] = rock_position.x * NetworkNodes[n].FANNOptions.input_scale;
    //NetworkNodes[n].Node->inputs[(cnt - first_rock) * rock_data_count + 3] = rock_position.y * NetworkNodes[n].FANNOptions.input_scale;
    //NetworkNodes[n].Node->inputs[(cnt - first_rock) * rock_data_count + 2] = rock_too_close;
    cnt++;
   }
   NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * rock_data_count + 0] = ifbodies[1]->body->GetPosition().x * NetworkNodes[n].FANNOptions.input_scale;
   NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * rock_data_count + 1] = ifbodies[1]->body->GetPosition().y * NetworkNodes[n].FANNOptions.input_scale;
   //NetworkNodes[n].Node->inputs[(last_rock - first_rock + 2) * rock_data_count + 0] = last_train_boat_position[0].x * NetworkNodes[n].FANNOptions.input_scale;
   //NetworkNodes[n].Node->inputs[(last_rock - first_rock + 2) * rock_data_count + 1] = last_train_boat_position[0].y * NetworkNodes[n].FANNOptions.input_scale;
   //NetworkNodes[n].Node->inputs[(last_rock - first_rock + 3) * rock_data_count + 0] = last_train_boat_position[1].x * NetworkNodes[n].FANNOptions.input_scale;
   //NetworkNodes[n].Node->inputs[(last_rock - first_rock + 3) * rock_data_count + 1] = last_train_boat_position[1].y * NetworkNodes[n].FANNOptions.input_scale;
   last_train_boat_position[0] = boat_position;
   last_train_boat_position[1] = last_train_boat_position[0];
   //NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * rock_data_count + 2] = (boat_hit[0]?1:0) ;
   //NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * rock_data_count + 3] = (any_rock_too_close ? 1 : 0) ;
   //NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * rock_data_count + 4] = ((boat_hit[0] || any_rock_too_close)?1:0);
  // NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * rock_data_count + 5] = any_rock_too_close;
   outputs[0] = ifbodies[0]->body->GetPosition().x * NetworkNodes[n].FANNOptions.output_scale;
   outputs[1] = ifbodies[0]->body->GetPosition().y * NetworkNodes[n].FANNOptions.output_scale;
   //static bool x = true;
   //if(!boat_hit[0]&& any_rock_too_close){
   if (!any_rock_too_close) {
    NetworkNodes[n].Add_Data(NetworkNodes[n].Node->inputs, outputs);
    NetworkNodes[n].node_train_error = 1.0;
   }
   //x=!x;
  }
  return;
 }
 void RunBoatAI(){
  {
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //  /  /  /  /  /  /             Learn From Player AI      /  /  /  /  /  /
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
   unsigned int n = 0;
   b2Vec2 rock_position;
   b2Vec2 boat_position = ifbodies[1]->body->GetPosition();
   if((boat_position.x!= boat_position.x)|| (boat_position.y != boat_position.y)){
    boat_position.x = boat_position.y = 0;
   }
   fann_type outputs[2];
   bool any_rock_too_close = false;
   unsigned int rock_data_count = 3;
   float prev_x_position = left;
   IFGeneralUtils::SMapWrap<float, unsigned int> rocks_by_x;//x, index
   for (unsigned int cnt = first_rock; cnt <= last_rock; cnt++) {
    rock_position = ifbodies[cnt]->body->GetPosition();
    rocks_by_x.Add(rock_position.x, cnt);
   }
   rocks_by_x.ResetIterator();
   float x_coord;
   unsigned int rock_index;
   unsigned int cnt = first_rock;
   while (rocks_by_x.GetNextIterator(x_coord, rock_index) > 0) {
    bool rock_too_close = false;
    rock_position = ifbodies[rock_index]->body->GetPosition();
//    if ((abs(boat_position.x - rock_position.x) < (boat_width * 1.0)) || (abs(boat_position.y - rock_position.y) < (boat_height * 1.0))) {
    if (Ifbodies_hit(1, rock_index)) {
     rock_too_close = true;
     any_rock_too_close = true;
    }
    prev_x_position = rock_position.x;
    fann_type aim_position_x = (prev_x_position + (rock_position.x - prev_x_position)*0.5);
    NetworkNodes[n].Node->inputs[(cnt - first_rock) * rock_data_count + 0] = aim_position_x * NetworkNodes[n].FANNOptions.input_scale;
    NetworkNodes[n].Node->inputs[(cnt - first_rock) * rock_data_count + 1] = (rock_position.y) * NetworkNodes[n].FANNOptions.input_scale;
    NetworkNodes[n].Node->inputs[(cnt - first_rock) * rock_data_count + 2] = rock_too_close;
    //NetworkNodes[n].Node->inputs[(cnt - first_rock) * rock_data_count + 2] = rock_position.x * NetworkNodes[n].FANNOptions.input_scale;
    //NetworkNodes[n].Node->inputs[(cnt - first_rock) * rock_data_count + 3] = rock_position.y * NetworkNodes[n].FANNOptions.input_scale;
    //NetworkNodes[n].Node->inputs[(cnt - first_rock) * rock_data_count + 2] = rock_too_close;
    
    cnt++;
   }
   NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * rock_data_count + 0] = ifbodies[1]->body->GetPosition().x * NetworkNodes[n].FANNOptions.input_scale;
   NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * rock_data_count + 1] = ifbodies[1]->body->GetPosition().y * NetworkNodes[n].FANNOptions.input_scale;   
   //NetworkNodes[n].Node->inputs[(last_rock - first_rock + 2) * rock_data_count + 1] = last_train_boat_position[0].y * NetworkNodes[n].FANNOptions.input_scale;
   //NetworkNodes[n].Node->inputs[(last_rock - first_rock + 3) * rock_data_count + 0] = last_train_boat_position[1].x * NetworkNodes[n].FANNOptions.input_scale;
   //NetworkNodes[n].Node->inputs[(last_rock - first_rock + 3) * rock_data_count + 1] = last_train_boat_position[1].y * NetworkNodes[n].FANNOptions.input_scale;
   //NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * rock_data_count + 2] = (boat_hit[0]?1:0) ;
   //NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * rock_data_count + 3] = (any_rock_too_close ? 1 : 0) ;
   //NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * rock_data_count + 4] = ((boat_hit[0] || any_rock_too_close)?1:0);
  // NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * rock_data_count + 5] = any_rock_too_close;
   //////////////////////////////////
   //////   Run AI
   /////////////////
   NetworkNodes[n].Node->outputs = IFFANNEngine::Run_Cascade_FANN(&NetworkNodes[n].Node->ifann, NetworkNodes[n].Node->inputs);
   //////////////////////////////////
   float desired_x = NetworkNodes[n].Node->outputs[0] / NetworkNodes[n].FANNOptions.output_scale - boat_position.x;
   float desired_y = NetworkNodes[n].Node->outputs[1] / NetworkNodes[n].FANNOptions.output_scale - boat_position.y;
   if ((desired_x != desired_x) || (desired_y != desired_y)) {
    desired_x = desired_y = 0;
   }
   ifbodies[1]->body->SetLinearVelocity(b2Vec2(desired_x*desired_x*desired_x, desired_y*desired_y*desired_y));
   ifbodies[1]->body->SetAngularVelocity(0);
   ifbodies[1]->body->SetTransform(ifbodies[1]->body->GetPosition(), 0);   
   NetworkNodes[n].Epoch_Train(true, NetworkNodes[n].node_train_samples);
  }  
  return;
 
  //Limit Boat Position
  //boat_position = ifbodies[1]->body->GetPosition();
  //if(boat_position.x<left)
  // boat_position.x = left;
  //else if (boat_position.x > right)
  // boat_position.x = right;
  //if (boat_position.y < bottom)
  // boat_position.y = bottom;
  //else if (boat_position.y > top)
  // boat_position.y = top;  
  //ifbodies[1]->body->SetTransform(boat_position, 0);
 }

 

}