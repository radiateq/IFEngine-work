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
 void ProcessBoatsAndRocks();
 float DeltaTime();//Function returns delta time between calls in milliseconds

 IFFANNEngine::CNetwork Network;
 const unsigned int Max_Network_Nodes = 10;
 CNetworkNode NetworkNodes[Max_Network_Nodes];
 void ConnectAI();
 void TrainBoatAI();
 void RunBoatAI();

 void CRacerLevel::PostOperations() {
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
     
     TrainBoatAI();     

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

  last_rock = 9;
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
  static float train_size_factor = 5.0;
  static float neuron_count_factor = 7.0;
  ////////////////////////////  Load Nodes START
  int n=0;
  NetworkNodes[n].FANNOptions.input_neurons = (last_rock - first_rock + 1+1)*3;
  NetworkNodes[n].FANNOptions.output_neurons = 2;
  NetworkNodes[n].FANNOptions.max_neurons = 30 * neuron_count_factor;
  NetworkNodes[n].FANNOptions.desired_error = 0.000;
  NetworkNodes[n].FANNOptions.input_scale = 0.1;
  NetworkNodes[n].FANNOptions.output_scale = 0.1;
  NetworkNodes[n].node_train_samples = NetworkNodes[n].node_data_counter_limit = 1000 * train_size_factor;
  NetworkNodes[n].Load_Node(Network, "boatandrocks");
  NetworkNodes[n].Load_Train_Data();
  NetworkNodes[n].Node->AddFlag(TNodeStates::E_Training);
  NetworkNodes[n].Node->AddFlag(TNodeStates::E_ContTraining);
  /////////////////////////////////////////////// Y
  n=1;
  NetworkNodes[n].FANNOptions.input_neurons = (last_rock - first_rock + 1)*2;
  NetworkNodes[n].FANNOptions.max_neurons = 15 * neuron_count_factor;
  NetworkNodes[n].FANNOptions.desired_error = 0.000;
  NetworkNodes[n].FANNOptions.input_scale = 0.01;
  NetworkNodes[n].FANNOptions.output_scale = 0.01;
  NetworkNodes[n].node_train_samples = NetworkNodes[n].node_data_counter_limit = 1000 * train_size_factor;
  NetworkNodes[n].Load_Node(Network, "boatandrocksy");
  NetworkNodes[n].Load_Train_Data();
  NetworkNodes[n].Node->AddFlag(TNodeStates::E_Training);
  NetworkNodes[n].Node->AddFlag(TNodeStates::E_ContTraining);
  /////////////////////////////////////////////// Approaching Rock Avoidance (L/R)
  n = 2;
  NetworkNodes[n].FANNOptions.input_neurons = 2*(last_rock - first_rock + 1);
  NetworkNodes[n].FANNOptions.max_neurons = 15 * neuron_count_factor;
  NetworkNodes[n].FANNOptions.desired_error = 0.000;
  NetworkNodes[n].FANNOptions.input_scale = 0.01;
  NetworkNodes[n].FANNOptions.output_scale = 0.1;
  NetworkNodes[n].node_train_samples = NetworkNodes[n].node_data_counter_limit = 1000 * train_size_factor;
  NetworkNodes[n].Load_Node(Network, "boatandrocksavoid");
  NetworkNodes[n].Load_Train_Data();
  NetworkNodes[n].Node->AddFlag(TNodeStates::E_Training);
  NetworkNodes[n].Node->AddFlag(TNodeStates::E_ContTraining);

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
  for(unsigned int boat_body_index = 0; boat_body_index < 1; boat_body_index++){
   boat_hit[boat_body_index] = false;
   for (unsigned int cnt = first_rock; cnt <= last_rock; cnt++) {
    if (ifbodies[cnt]->body->GetPosition().y < (bottom*1.3)) {
     ifbodies[cnt]->body->SetTransform(b2Vec2(drand48()*(right - left) - right, top + top * (drand48() + 1.0)*1.7), 0.0);
    }
    ifbodies[cnt]->body->SetLinearVelocity(b2Vec2(0.0, current_velocity));
   }
   for (b2ContactEdge* ce = ifbodies[boat_body_index]->body->GetContactList(); ce && (boat_hit[boat_body_index] == false); ce = ce->next)
   {
    b2Contact* c = ce->contact;
    b2Fixture *hit_body_fix;
    if (c->GetFixtureA()->GetBody() == ifbodies[boat_body_index]->body) {
     hit_body_fix = c->GetFixtureB();
    }
    else {
     hit_body_fix = c->GetFixtureA();
    }
    for (unsigned int cnt = first_rock; cnt <= last_rock; cnt++) {
     if ((hit_body_fix->GetBody() == ifbodies[cnt]->body)&&(c->IsTouching())) {
      boat_hit[boat_body_index] = true;
      break;
     }
    }
    if (boat_hit[boat_body_index])
     break;
   }
   if ((boat_hit[boat_body_index] == true)&&false) {
    for (unsigned int cnt = first_rock; cnt <= last_rock; cnt++) {
     ifbodies[cnt]->body->SetLinearVelocity(b2Vec2(0.0, 0.0));
    }
   }  
   ifbodies[boat_body_index]->body->SetTransform(ifbodies[boat_body_index]->body->GetPosition(), 0.0);
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //                                                      //
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  RunBoatAI();
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
   b2Vec2 boat_position = ifbodies[1]->body->GetPosition();
   fann_type outputs[2];
   for (unsigned int cnt = first_rock; cnt <= last_rock; cnt++) {
    rock_position = ifbodies[cnt]->body->GetPosition();
    NetworkNodes[n].Node->inputs[(cnt - first_rock) * 3 + 0] = (rock_position.x) * NetworkNodes[n].FANNOptions.input_scale;
    NetworkNodes[n].Node->inputs[(cnt - first_rock) * 3 + 1] = (rock_position.y) * NetworkNodes[n].FANNOptions.input_scale;
    NetworkNodes[n].Node->inputs[(cnt - first_rock) * 3 + 2] = b2Distance(boat_position, rock_position) * NetworkNodes[n].FANNOptions.input_scale;
   }
   NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * 3 + 0] = boat_position.x * NetworkNodes[n].FANNOptions.input_scale;;
   NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * 3 + 1] = boat_position.y * NetworkNodes[n].FANNOptions.input_scale;;
   NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * 3 + 2] = 0 * NetworkNodes[n].FANNOptions.input_scale;;
   outputs[0] = ifbodies[0]->body->GetPosition().x * NetworkNodes[n].FANNOptions.output_scale;
   outputs[1] = ifbodies[0]->body->GetPosition().x * NetworkNodes[n].FANNOptions.output_scale;
   //static bool x = true;
   if(!boat_hit[0]){
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
   fann_type outputs[2];
   for (unsigned int cnt = first_rock; cnt <= last_rock; cnt++) {
    rock_position = ifbodies[cnt]->body->GetPosition();
    NetworkNodes[n].Node->inputs[(cnt - first_rock) * 3 + 0] = (rock_position.x) * NetworkNodes[n].FANNOptions.input_scale;
    NetworkNodes[n].Node->inputs[(cnt - first_rock) * 3 + 1] = (rock_position.y) * NetworkNodes[n].FANNOptions.input_scale;
    NetworkNodes[n].Node->inputs[(cnt - first_rock) * 3 + 2] = b2Distance(boat_position, rock_position) * NetworkNodes[n].FANNOptions.input_scale;
   }
   NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * 3 + 0] = boat_position.x * NetworkNodes[n].FANNOptions.input_scale;;
   NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * 3 + 1] = boat_position.y * NetworkNodes[n].FANNOptions.input_scale;;
   NetworkNodes[n].Node->inputs[(last_rock - first_rock + 1) * 3 + 2] = 0 * NetworkNodes[n].FANNOptions.input_scale;;
   NetworkNodes[n].Node->outputs = IFFANNEngine::Run_Cascade_FANN(&NetworkNodes[n].Node->ifann, NetworkNodes[n].Node->inputs);
   float desired_x = NetworkNodes[n].Node->outputs[0] / NetworkNodes[n].FANNOptions.output_scale - boat_position.x;
   float desired_y = NetworkNodes[n].Node->outputs[1] / NetworkNodes[n].FANNOptions.output_scale - boat_position.y;
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