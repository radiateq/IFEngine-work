#include "MainProgram.h"









class CPongBallStateMachine;
class CNetworkNode;


ifCB2Adapter IFAdapter;

ifCB2BodyUtils B2BodyUtils(&IFAdapter);

extern TS_User_Data User_Data;


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
//std::vector<fann_type> input_data, output_data;
unsigned int input_data_sets = 0;
static float last_pos_x = FLT_MAX, last_pos_y = FLT_MAX;
int max_neurons = 20, neurons_between_reports = 0, input_neurons = 3, output_neurons = 2;
float desired_error = 0.001, input_scale = 0.01, output_scale = 0.01;
int touchx, touchy;
//DEMO 3 - BODY + GUI 
bool BODY_DEMO_initialized = false;
ifCB2Body *last_touched_object = NULL;
GLuint TEST_GUI_Tex_Ary[10] = { 0 };
ifCB2Body *buttons_body[10];
//DEMO 4 - Simple Game
//struct timespec Last_GUI_Click_Time;

unsigned long int Last_GUI_Click_Time;
bool AutoPlayer = false;
char char_buffer[BUFSIZ];
unsigned int const Cn_Max_Bodies = 10;
ifCB2Body *game_body[Cn_Max_Bodies];
bool DEMO4_initialized = false;
float thickness;
float left, bottom, right, top;
float radius, paddle_width, dummy;
IFFANNEngine::CNetwork Network;
unsigned int Last_AI_Paddle_Network_Active = -1;
bool LastAIPaddle = false;
//NODE 3 -------   TEMPLATE FOR CLASS START
//Selects what node to use for paddle x output based on 
typedef std::vector<fann_type> TFannVector;
//SFANNPong Select1_FANNPong;
//IFFANNEngine::CNode *Select1_Node;
//TFannVector select1_input_data, select1_output_data;
bool select1_train_trigger = false;
float ball_to_pad_prev_distance = FLT_MAX;
//NODE 5 -------  STOP
int score = 0;
int prev_score = -10000;
//unsigned int BounceBrainTrainSizeLimit = 30, PaddleBrainsTrainSizeLimit = 2000;
//unsigned int Node2_1_train_size_limit = 60;
bool trained[5] = { 0 }; 
//fann_type Node1_train_error = 1;
//fann_type Node2_train_error = 1;
//fann_type Node2_1_train_error = 1;
//fann_type Select1_train_error = 1;
fann_type AI_paddle_desired_position_x = 0.0;
//unsigned int BounceBrainTrainSizeLimit = 500, PaddleBrainsTrainSizeLimit = 300;
//unsigned int Node2_1_train_size_limit = 500, select2_data_counter_limit = 500;
//unsigned int select1_data_counter_limit = 1000;
//unsigned int select1_data_counter_limit = 1500;
CIFTextRender TextRender;














typedef IFFANNEngine::CNode::ENodeStates TNodeStates;
typedef std::vector<fann_type> TFannVector;
TFannVector *train_input_vector, *train_output_vector;
struct fann *train_ann;
void Setup_Train_Cascade_FANN_Callback(fann *_train_ann, TFannVector *_train_input_vector, TFannVector *_train_output_vector) {
 train_input_vector = _train_input_vector;
 train_output_vector = _train_output_vector;
 train_ann = _train_ann;
}
void Train_Cascade_FANN_Callback(unsigned int num_data, unsigned int num_input, unsigned int num_output, fann_type *input, fann_type *output) {
 for (unsigned int cnt = 0; cnt < train_ann->num_input; cnt++) {
  input[cnt] = (*train_input_vector)[num_data * train_ann->num_input + cnt];
 }
 for (unsigned int cnt = 0; cnt < train_ann->num_output; cnt++) {
  output[cnt] = (*train_output_vector)[num_data * train_ann->num_output + cnt];
 }
}


class CNetworkNode {
public:
 ~CNetworkNode(){
  Free();
 }
 void Free(){
  if (Node) {
   Node->NodeRegister->Unregister(Node);
   delete Node;
  }
 }
 SFANNPong FANNOptions;
 IFFANNEngine::CNode *Node = NULL;
 TFannVector node_input_data, node_output_data;
 bool node_train_trigger = false, node_can_train = false;
 unsigned int node_data_counter = 0;
 char node_unique_name[1024];
 unsigned int node_train_samples = 50, node_train_pos = 0, node_last_save_index = 0, node_new_samples = 0;
 fann_type node_train_error = 1, epoch_train_desired_eror = 1e-6;
 unsigned int node_data_counter_limit = 1000;

 size_t NumTrainData(){
  return node_output_data.size()/FANNOptions.output_neurons;
 }

 void CheckNodeTrainDataState(){
  if (node_output_data.size() == 0) {
   Node->AddFlag(TNodeStates::E_NoData);
   Node->ClearFlag(TNodeStates::E_FullData);
  }
  else {
   Node->ClearFlag(TNodeStates::E_NoData);
   {//Limit train size    
    node_new_samples++;
    if ((node_output_data.size()/FANNOptions.output_neurons) > node_data_counter_limit) {
     Node->AddFlag(TNodeStates::E_FullData);
     node_input_data.erase(node_input_data.begin(), node_input_data.begin() + node_input_data.size() - node_data_counter_limit * FANNOptions.input_neurons);
     node_output_data.erase(node_output_data.begin(), node_output_data.begin() + node_output_data.size() - node_data_counter_limit * FANNOptions.output_neurons);
    }
    else {
     Node->NodeStates = (IFFANNEngine::CNode::ENodeStates)(Node->NodeStates & (~IFFANNEngine::CNode::ENodeStates::E_FullData));
    }
   }
  }
 }

 void Load_Node(char const * const unique_name, bool is_running = true) { 
  Free();
  Node = new IFFANNEngine::CNode;
  Node->IsRunning = is_running;
  Network.NodeRegister.Register(Node);

  if (IFFANN::Check_Save_Cascade_FANN(unique_name, IFFANN::CnFinalFannPostscript)) {
   IFFANN::Load_Cascade_FANN(&Node->ifann, unique_name, IFFANN::CnFinalFannPostscript);
   Node->NodeStates = IFFANNEngine::CNode::ENodeStates::E_Trained;
  }
  if (!Node->ifann.ann) {
   IFFANN::Setup_Train_Cascade_FANN(IFFANN::Create_Cascade_FANN(IFFANN::Init_Cascade_FANN(&Node->ifann), FANNOptions.input_neurons, FANNOptions.output_neurons, unique_name), FANNOptions.max_neurons, FANNOptions.neurons_between_reports, FANNOptions.desired_error, FANNOptions.input_scale, FANNOptions.output_scale);
   IFFANN::Save_Cascade_FANN(&Node->ifann, IFFANN::CnFinalFannPostscript);
   Node->NodeStates = IFFANNEngine::CNode::ENodeStates::E_FreshTrain;
  }
  CheckNodeTrainDataState();

  Node->LoadCore(unique_name);
 }

 bool Add_Data(fann_type *input, fann_type *output) {
  if ((Node->NodeStates&IFFANNEngine::CNode::ENodeStates::E_FullData) && !(Node->NodeStates&IFFANNEngine::CNode::ENodeStates::E_OverwriteData))
   return false;

  for (unsigned int paramno = 0; paramno < FANNOptions.input_neurons; paramno++) {
   node_input_data.push_back(input[paramno]);
  }
  for (unsigned int paramno = 0; paramno < FANNOptions.output_neurons; paramno++) {
   node_output_data.push_back(output[paramno]);
  }
  CheckNodeTrainDataState();

  return true;
 }

 bool Epoch_Train(bool SaveNewData = true, unsigned int _node_train_samples = 5, unsigned long int timeToRunus = 3000) {
  if(!(Node->NodeStates&IFFANNEngine::CNode::ENodeStates::E_ContTraining))
   return false;
//  if (!Node->GetFlag(TNodeStates::E_FullData))
  if(node_train_samples==0)
   return false;
  //if (false)
  {/////////////////////////////////   TRAIN node
   /////////////////////////////////////NC02START
   if ((node_train_error > epoch_train_desired_eror) && ((node_train_error != std::numeric_limits<fann_type>::max())) && trained[3] == false)
   {
    //Save new chunk of data
    if (SaveNewData && (node_new_samples%node_train_samples) > node_last_save_index) {
     node_last_save_index = (node_new_samples%node_train_samples);
     node_new_samples = 0;
     Setup_Train_Cascade_FANN_Callback(Node->ifann.ann, &node_input_data, &node_output_data);
     IFFANN::Train_Cascade_FANN(&Node->ifann, Train_Cascade_FANN_Callback, NumTrainData(), 2, false);
    }

    if (!SaveNewData) {
     node_train_samples = _node_train_samples;
    }

    if (Node->ifann.ann_train->train_data) {
     struct fann_train_data *train_subset;

     //Get time
     struct timespec temp_timespec;
     clock_gettime(CLOCK_MONOTONIC, &temp_timespec);
     unsigned long int start_time = RQNDKUtils::timespec2us64(&temp_timespec), end_time = start_time + timeToRunus;
     //Create subset to train on
     train_subset = fann_subset_train_data(Node->ifann.ann_train->train_data, Node->ifann.ann_train->train_data->num_data - node_train_samples - node_train_pos, node_train_samples);
     while ((end_time > start_time) && (node_train_samples <= Node->ifann.ann_train->train_data->num_data)) {
      node_train_error = fann_train_epoch(Node->ifann.ann, train_subset);
      clock_gettime(CLOCK_MONOTONIC, &temp_timespec);
      start_time = RQNDKUtils::timespec2us64(&temp_timespec);
     }
     fann_destroy_train(train_subset);
     IFFANN::Save_Cascade_FANN(&Node->ifann, IFFANN::CnFinalFannPostscript);
    }
    else {
     Setup_Train_Cascade_FANN_Callback(Node->ifann.ann, &node_input_data, &node_output_data);
     IFFANN::Train_Cascade_FANN(&Node->ifann, Train_Cascade_FANN_Callback, NumTrainData(), 1, false);
    }
   }
   else {
    if (node_train_error != std::numeric_limits<fann_type>::max()) {
     if ((node_train_error <= epoch_train_desired_eror) && Node->ifann.ann_train->train_data->num_data >= node_data_counter_limit) {
      Setup_Train_Cascade_FANN_Callback(Node->ifann.ann, &node_input_data, &node_output_data);
      IFFANN::Train_Cascade_FANN(&Node->ifann, Train_Cascade_FANN_Callback, NumTrainData(), 2, false);
      IFFANN::Save_Cascade_FANN(&Node->ifann, IFFANN::CnFinalFannPostscript);
      node_train_error = std::numeric_limits<fann_type>::max();      
     }
    }
    return true;
   }
  }
  return false;
  /////////////////////////////////////NC02STOP
 }

 void Load_Train_Data() {
  if (IFFANN::Train_Cascade_FANN(&(Node->ifann), NULL, 0, 1, false)) {
   node_input_data.clear();
   node_output_data.clear();
   for(unsigned int cnt = 0; cnt < Node->ifann.ann_train->train_data->num_data; cnt++) {
    for (unsigned int inputcnt = 0; inputcnt < Node->ifann.ann_train->train_data->num_input; inputcnt++) {
     node_input_data.push_back(Node->ifann.ann_train->train_data->input[cnt][inputcnt]);
    }
    for (unsigned int outputcnt = 0; outputcnt < Node->ifann.ann_train->train_data->num_output; outputcnt++) {
     node_output_data.push_back(Node->ifann.ann_train->train_data->output[cnt][outputcnt]);
    }
   }
   //NetworkNodes[1].NumTrainData() = NetworkNodes[2].Node->ifann.ann_train->train_data->num_data;
  }
  CheckNodeTrainDataState();
 }
 void Save_Train_Data() {
  Setup_Train_Cascade_FANN_Callback(Node->ifann.ann, &node_input_data, &node_output_data);
  IFFANN::Train_Cascade_FANN(&(Node->ifann), Train_Cascade_FANN_Callback, NumTrainData(), 2, false);
 }

 bool Train_Node() {
  if ( !Node->GetFlag(TNodeStates::E_Training))
   return false;
  if ( !Node->GetFlag(TNodeStates::E_FullData ))
   return false;
  if (Node->GetFlag(TNodeStates::E_ContTraining))
   return false;
  char unique_name[BUFSIZ+1];
  strcpy(unique_name, Node->ifann.unique_name);
  IFFANN::Setup_Train_Cascade_FANN(IFFANN::Create_Cascade_FANN(IFFANN::Init_Cascade_FANN(&(Node->ifann)), FANNOptions.input_neurons, FANNOptions.output_neurons, unique_name), FANNOptions.max_neurons, FANNOptions.neurons_between_reports, FANNOptions.desired_error, FANNOptions.input_scale, FANNOptions.output_scale);
  Setup_Train_Cascade_FANN_Callback(Node->ifann.ann, &node_input_data, &node_output_data);
  IFFANN::Train_Cascade_FANN(&(Node->ifann), Train_Cascade_FANN_Callback, NumTrainData(), 2);
  IFFANN::Load_Cascade_FANN(&(Node->ifann), Node->ifann.unique_name, IFFANN::CnTrainedFannPostscript);
  IFFANN::Save_Cascade_FANN(&(Node->ifann), IFFANN::CnFinalFannPostscript);
  Node->LoadCore(Node->ifann.unique_name);
  return true;
 }
 //Retrived by Load_Node or any function using Load_Cascade_FANN
 bool Save_Fann(){
  return IFFANN::Save_Cascade_FANN(&Node->ifann, IFFANN::CnFinalFannPostscript);
 }
};

const unsigned int Max_Network_Nodes=10;
CNetworkNode NetworkNodes[Max_Network_Nodes];
















class CPongBallStateMachine {
public:
 enum EBallStates {
  E_Start = 0,
  E_TravelUp = 1,
  E_TravelDown,
  E_NoContact = 1,
  E_InContact,
  E_NewContact = 2 << 1,
  E_BrokeContact = 2 << 2,
  E_ContactPlayer = 1,
  E_ContactAI,
  E_ContactWall = 2 << 1,
  E_Center = 1,
  E_PlayerMiss,
  E_AIMiss = 2 << 1,
  E_Repeating = 2 << 2
 };

 struct SBallStates {
  EBallStates TravelDirection = E_Start;
  EBallStates ContactState = E_Start;
  EBallStates ContactObjects = E_Start;
  EBallStates NewContactObjects = E_Start;//After ball is not in contact with body any more its flag is set in this variable and cleared in ContactObjects, objects must be present in ContactObjects as well
  EBallStates BrokenContactObjects = E_Start;//After ball is not in contact with body any more its flag is set in this variable and cleared in ContactObjects, objects must be present in ContactObjects as well
  EBallStates PlayfieldLocation = E_Start;
  EBallStates LastPlayfieldLocation = E_Start;
 };
public:
 SBallStates BallState;
 void ResetBallState(){
  SBallStates BallState1;
  BallState = BallState1;
 }
 void DetermineBallState(ifCB2Body **game_body) {
  SBallStates CurrentBallState;//State that is going to be compared to old state and then saved as new state at the end
  b2Vec2 balllinvel = game_body[2]->body->GetLinearVelocity();
  b2Vec2 ballposition = game_body[2]->body->GetPosition();

  CurrentBallState.TravelDirection = ((balllinvel.y > 0) ? E_TravelUp : E_TravelDown);
  if (CurrentBallState.TravelDirection == E_TravelUp) {
   //CONTACTS START
   for (b2ContactEdge* ce = game_body[2]->body->GetContactList(); ce; ce = ce->next)
   {
    CurrentBallState.ContactState = (EBallStates)(CurrentBallState.ContactState | E_InContact);
    b2Contact* c = ce->contact;
    b2Fixture *hit_body_fix;
    if (c->GetFixtureA()->GetBody() == game_body[2]->body) {
     hit_body_fix = c->GetFixtureB();
    }
    else {
     hit_body_fix = c->GetFixtureA();
    }
    if (hit_body_fix->GetBody() == game_body[0]->body || hit_body_fix->GetBody() == game_body[1]->body) {//Walls
     CurrentBallState.ContactObjects = (EBallStates)((CurrentBallState.ContactObjects) | (E_ContactWall));
     if (!(BallState.ContactObjects & E_ContactWall)) {
      CurrentBallState.NewContactObjects = (EBallStates)(CurrentBallState.NewContactObjects | E_ContactWall);
      CurrentBallState.ContactState = (EBallStates)(CurrentBallState.ContactState | E_NewContact);
     }
    }
    else if (hit_body_fix->GetBody() == game_body[3]->body) {//Player paddle
     CurrentBallState.ContactObjects = (EBallStates)(CurrentBallState.ContactObjects | E_ContactPlayer);
     if (!(BallState.ContactObjects & E_ContactPlayer)) {
      CurrentBallState.NewContactObjects = (EBallStates)(CurrentBallState.NewContactObjects | E_ContactPlayer);
      CurrentBallState.ContactState = (EBallStates)(CurrentBallState.ContactState | E_NewContact);
     }
    }
    else if (hit_body_fix->GetBody() == game_body[4]->body) {//AI paddle
     CurrentBallState.ContactObjects = (EBallStates)(CurrentBallState.ContactObjects | E_ContactAI);
     if (!(BallState.ContactObjects & E_ContactAI)) {
      CurrentBallState.NewContactObjects = (EBallStates)(CurrentBallState.NewContactObjects | E_ContactAI);
      CurrentBallState.ContactState = (EBallStates)(CurrentBallState.ContactState | E_NewContact);
     }
    }
   }
   if ((BallState.ContactObjects & E_ContactWall) && !(CurrentBallState.ContactObjects & E_ContactWall)) {
    CurrentBallState.BrokenContactObjects = (EBallStates)(CurrentBallState.BrokenContactObjects | E_ContactWall);
    CurrentBallState.ContactState = (EBallStates)(CurrentBallState.ContactState | E_BrokeContact);
   }
   if ((BallState.ContactObjects & E_ContactPlayer) && !(CurrentBallState.ContactObjects & E_ContactPlayer)) {
    CurrentBallState.BrokenContactObjects = (EBallStates)(CurrentBallState.BrokenContactObjects | E_ContactPlayer);
    CurrentBallState.ContactState = (EBallStates)(CurrentBallState.ContactState | E_BrokeContact);
   }
   if ((BallState.ContactObjects & E_ContactAI) && !(CurrentBallState.ContactObjects & E_ContactAI)) {
    CurrentBallState.BrokenContactObjects = (EBallStates)(CurrentBallState.BrokenContactObjects | E_ContactAI);
    CurrentBallState.ContactState = (EBallStates)(CurrentBallState.ContactState | E_BrokeContact);
   }
   if (CurrentBallState.ContactState == E_Start) {//No contact
    CurrentBallState.ContactState = E_NoContact;
   }
   //CONTACTS STOP
   CurrentBallState.LastPlayfieldLocation = BallState.PlayfieldLocation;
   //Position on playfield START
   if (ballposition.y > game_body[4]->body->GetPosition().y) {
    CurrentBallState.PlayfieldLocation = E_AIMiss;
   }
   else {
    CurrentBallState.PlayfieldLocation = E_Center;
   }
   if (CurrentBallState.PlayfieldLocation == (BallState.PlayfieldLocation&~E_Repeating)) {
    CurrentBallState.PlayfieldLocation = (EBallStates)(CurrentBallState.PlayfieldLocation | E_Repeating);
   }
   //Position on playfield STOP
   BallState = CurrentBallState;
  }
  else {
   //Position on playfield START
   BallState.LastPlayfieldLocation = BallState.PlayfieldLocation;
   if (ballposition.y < game_body[3]->body->GetPosition().y) {
    CurrentBallState.PlayfieldLocation = E_PlayerMiss;
   }
   else {
    CurrentBallState.PlayfieldLocation = E_Center;
   }
   if (CurrentBallState.PlayfieldLocation == (BallState.PlayfieldLocation&~E_Repeating)) {
    CurrentBallState.PlayfieldLocation = (EBallStates)(CurrentBallState.PlayfieldLocation | E_Repeating);
   }   
   //Position on playfield STOP
   BallState.TravelDirection = CurrentBallState.TravelDirection;
   BallState.PlayfieldLocation = CurrentBallState.PlayfieldLocation;
  }
 }
 void ProcessBallState(ifCB2Body **game_body) {
  b2Vec2 balllinvel = game_body[2]->body->GetLinearVelocity();
  b2Vec2 ballposition = game_body[2]->body->GetPosition();

  if (BallState.NewContactObjects&E_ContactAI) {//leaving AI paddle        
   NetworkNodes[4].Node->inputs[0] = game_body[4]->body->GetPosition().x;
   NetworkNodes[4].Node->inputs[1] = game_body[4]->body->GetLinearVelocity().x;

   NetworkNodes[4].Node->inputs[2] = ballposition.x;
   NetworkNodes[4].Node->inputs[3] = atan2(balllinvel.y, balllinvel.x);
   NetworkNodes[4].Node->inputs[4] = sqrt(balllinvel.x*balllinvel.x + balllinvel.y*balllinvel.y)* NetworkNodes[3].Node->ifann.output_scale;

   NetworkNodes[4].Node->outputs = IFFANNEngine::Run_Cascade_FANN(&NetworkNodes[4].Node->ifann, NetworkNodes[4].Node->inputs);
   NetworkNodes[0].Node->IsRunning = NetworkNodes[4].Node->outputs[0]>0.5;
   NetworkNodes[1].Node->IsRunning = NetworkNodes[4].Node->outputs[1] > 0.5;
   NetworkNodes[2].Node->IsRunning = NetworkNodes[1].Node->IsRunning;
   NetworkNodes[3].Node->IsRunning = NetworkNodes[4].Node->outputs[2] > 0.5;

   if( Last_AI_Paddle_Network_Active == 0 )
    NetworkNodes[4].Node->inputs[5] = 1;
   else
    NetworkNodes[4].Node->inputs[5] = 0;

   if (Last_AI_Paddle_Network_Active == 1)
    NetworkNodes[4].Node->inputs[6] = 1;
   else
    NetworkNodes[4].Node->inputs[6] = 0;

   if (Last_AI_Paddle_Network_Active == 2)
    NetworkNodes[4].Node->inputs[7] = 1;
   else
    NetworkNodes[4].Node->inputs[7] = 0;
   NetworkNodes[4].node_new_samples = 1;
  }

  if((BallState.PlayfieldLocation&E_PlayerMiss)&&!(BallState.PlayfieldLocation&E_Repeating)){
   score--;
  }
  if ((BallState.PlayfieldLocation&E_AIMiss)&&!(BallState.PlayfieldLocation&E_Repeating)) {
   score++;
  }
  if ((BallState.PlayfieldLocation&E_PlayerMiss) && !(BallState.PlayfieldLocation&E_Repeating)) {
   if(NetworkNodes[4].node_new_samples == 1){
    NetworkNodes[4].node_new_samples = 0;
    for(unsigned int cnt = 0; cnt < NetworkNodes[4].Node->ifann.ann->num_input; cnt++){
     NetworkNodes[4].node_input_data.push_back(NetworkNodes[4].Node->inputs[cnt]);
    }

    for (unsigned int cnt = 0; cnt < NetworkNodes[4].Node->ifann.ann->num_output; cnt++) {
     NetworkNodes[4].node_output_data.push_back(NetworkNodes[4].Node->outputs[cnt]);
    }
    //NetworkNodes[4].Save_Train_Data();
   }
  }
  if (BallState.BrokenContactObjects&E_ContactPlayer) {//leaving player paddle        
   NetworkNodes[3].Node->inputs[0] = ballposition.x * NetworkNodes[3].Node->ifann.output_scale;
   NetworkNodes[3].Node->inputs[1] = atan2(balllinvel.y, balllinvel.x);
   NetworkNodes[3].Node->inputs[2] = sqrt(balllinvel.x*balllinvel.x + balllinvel.y*balllinvel.y)* NetworkNodes[3].Node->ifann.output_scale;
   NetworkNodes[3].Node->inputs[3] = 0.0f;
   NetworkNodes[3].Node->IsRunning = true;
   select1_train_trigger = true;
  }

  if ((BallState.NewContactObjects & E_ContactWall) && (BallState.TravelDirection&E_TravelUp)) {//Hitting the wall
   NetworkNodes[2].Node->inputs[0] = NetworkNodes[1].Node->inputs[0] = (ballposition.x) * NetworkNodes[1].Node->ifann.input_scale;
   NetworkNodes[2].Node->inputs[1] = NetworkNodes[1].Node->inputs[1] = (ballposition.y) * NetworkNodes[1].Node->ifann.input_scale;
   NetworkNodes[2].Node->inputs[2] = NetworkNodes[1].Node->inputs[2] = atan2(balllinvel.y, balllinvel.x) * NetworkNodes[1].Node->ifann.input_scale;
   //select1_train_trigger = false;
  }
  
  if ((BallState.BrokenContactObjects & E_ContactWall)&&(BallState.TravelDirection&E_TravelUp)) {//Leaving the wall
   //if (b2Distance(ballposition, b2Vec2(NetworkNodes[1].Node->inputs[0], NetworkNodes[1].Node->inputs[1])) > ((right - left)*0.2)) {
    NetworkNodes[1].node_new_samples++;
    NetworkNodes[2].Node->inputs[3] = NetworkNodes[1].Node->inputs[3] = (ballposition.x) * NetworkNodes[1].Node->ifann.input_scale;
    NetworkNodes[2].Node->inputs[4] = NetworkNodes[1].Node->inputs[4] = (ballposition.y) * NetworkNodes[1].Node->ifann.input_scale;
    NetworkNodes[2].Node->inputs[5] = NetworkNodes[1].Node->inputs[5] = atan2(balllinvel.y, balllinvel.x) * NetworkNodes[1].Node->ifann.input_scale;
    NetworkNodes[1].Node->outputs = IFFANNEngine::Run_Cascade_FANN(&NetworkNodes[1].Node->ifann, NetworkNodes[1].Node->inputs);
    NetworkNodes[2].Node->inputs[6] = NetworkNodes[1].Node->outputs[0];
    //NetworkNodes[1].Node->ClearFlag(TNodeStates::E_Training);
    //train_input_bounce_set = false;
    NetworkNodes[1].Node->IsRunning = true;
    NetworkNodes[2].Node->IsRunning = true;
    NetworkNodes[0].Node->IsRunning = false;
    //NetworkNodes[3].Node->IsRunning = true;
    //ball_state = 4;
   //}
  }

  if (NetworkNodes[1].node_new_samples
      &&
     (BallState.NewContactObjects & E_ContactAI)&&!(BallState.PlayfieldLocation & E_AIMiss)
     ||
   (((BallState.PlayfieldLocation & E_AIMiss) && !(BallState.PlayfieldLocation&E_Repeating)))
   ) {//The moment ball hits or passes AI Pad
   //NetworkNodes[1].node_input_data.push_back(NetworkNodes[1].Node->inputs[0]);
   //NetworkNodes[1].node_input_data.push_back(NetworkNodes[1].Node->inputs[1]);
   //NetworkNodes[1].node_input_data.push_back(NetworkNodes[1].Node->inputs[2]);
   //NetworkNodes[1].node_input_data.push_back(NetworkNodes[1].Node->inputs[3]);
   //NetworkNodes[1].node_input_data.push_back(NetworkNodes[1].Node->inputs[4]);
   //NetworkNodes[1].node_input_data.push_back(NetworkNodes[1].Node->inputs[5]);
   //NetworkNodes[1].node_output_data.push_back(((ballposition.x))*NetworkNodes[1].Node->ifann.output_scale);
   //NetworkNodes[1].NumTrainData() = NetworkNodes[1].node_output_data.size();
   
   

   fann_type outputs[1];
   outputs[0] = (((ballposition.x))*NetworkNodes[1].Node->ifann.output_scale);
   NetworkNodes[1].Add_Data( NetworkNodes[1].Node->inputs, outputs);
   
   //outputs[0] = ((ballposition.x)*NetworkNodes[1].Node->ifann.output_scale);
   //NetworkNodes[1].Node->outputs = IFFANNEngine::Run_Cascade_FANN(&NetworkNodes[1].Node->ifann, NetworkNodes[1].Node->inputs);
   NetworkNodes[2].Add_Data( NetworkNodes[2].Node->inputs, outputs);
   
   NetworkNodes[1].node_new_samples = 0;
  }

 }
};
CPongBallStateMachine PongBallStateMachine;


void TESTFN_SaveAllTrainData(){
 NetworkNodes[0].Save_Train_Data();
 NetworkNodes[1].Save_Train_Data();
 NetworkNodes[2].Save_Train_Data(); 
 NetworkNodes[3].Save_Train_Data();
 NetworkNodes[4].Save_Train_Data();
}

void ConnectNetwork01(){
 Network.ConnectPins(NetworkNodes[1].Node->GetOutPinByIndex(0), NetworkNodes[2].Node->GetInPinByIndex(6)); 
// Network.ConnectPins(Node1->GetOutPinByIndex(0), Select1_Node->GetInPinByIndex(4));
 //Network.ConnectPins(NetworkNodes[1].Node->GetOutPinByIndex(0), Select1_Node->GetInPinByIndex(5));
 //Network.ConnectPins(NetworkNodes[2].Node->GetOutPinByIndex(0), Select1_Node->GetInPinByIndex(6));

}
void DisonnectNetwork01() {
 Network.DisconnectPins(NetworkNodes[1].Node->GetOutPinByIndex(0), NetworkNodes[2].Node->GetInPinByIndex(6));
 //Network.DisconnectPins(Node1->GetOutPinByIndex(0), Select1_Node->GetInPinByIndex(4));
 //Network.DisconnectPins(NetworkNodes[1].Node->GetOutPinByIndex(0), Select1_Node->GetInPinByIndex(5));
 //Network.DisconnectPins(NetworkNodes[2].Node->GetOutPinByIndex(0), Select1_Node->GetInPinByIndex(6));
}
void TESTFN_PostOperations(engine &engine) {
 if (engine.EGL_initialized) {
  if(DEMO4_initialized){   

   
   if(score == prev_score )
    return;
   prev_score = score;
   char outtext[BUFSIZ+1];
   if((score<10)&&(score >= 0)){
    sprintf(outtext," %i ", score);   
   }else{
    sprintf(outtext, "%i", abs(score));
   }
   if(score==0){
    TextRender.SetForegroundColor(10, 250, 10, 255);
   }else if(score>0){
    TextRender.SetForegroundColor(250, 150 + ((abs(score) < 11) ? (score * 10) : 100), 10, 255);
   }else{
    TextRender.SetForegroundColor(250, 10, 10, 255);
   }
   //TextRender.SetForegroundColor(250, 250, 10, 255);


   glDeleteTextures(1, &TEST_GUI_Tex_Ary[5]);
   game_body[5]->OGL_body->texture_ID = TextRender.DrawText(outtext, 256);
   if (true) {
    char print_char = '\0';//null box does not encompass whole text
    SFloatRect *char_rect;
    char_rect = TextRender.CharMap.GetRef(print_char);
    size_t UVsize = game_body[5]->OGL_body->UVmapping_cnt;
    game_body[5]->OGL_body->UVmapping[0] = char_rect->xMax*1.1 / (float)TextRender.expanded_width;
    game_body[5]->OGL_body->UVmapping[1] = char_rect->yMin / (float)TextRender.expanded_height;

    game_body[5]->OGL_body->UVmapping[2] = char_rect->xMax*1.1 / (float)TextRender.expanded_width;
    game_body[5]->OGL_body->UVmapping[3] = char_rect->yMax / (float)TextRender.expanded_height;

    game_body[5]->OGL_body->UVmapping[4] = char_rect->xMin / (float)TextRender.expanded_width;
    game_body[5]->OGL_body->UVmapping[5] = char_rect->yMax*1.1 / (float)TextRender.expanded_height;

    game_body[5]->OGL_body->UVmapping[6] = char_rect->xMin / (float)TextRender.expanded_width;
    game_body[5]->OGL_body->UVmapping[7] = char_rect->yMin / (float)TextRender.expanded_height;

   }

  }

  //DEMO 3 - USER INTERFACE - START
  if (!BODY_DEMO_initialized) {
  }
  else {
   if (last_touched_object) {

    Eigen::Transform<float, 3, Eigen::TransformTraits::Affine> t;
    Eigen::Matrix<float, 4, 4> mScale;
    //void CopyFloat16ToMatrix(MatrixXf &mf, float *mfa);
    //void CopyMatrix16ToFloat(MatrixXf &mf, float *mfa);
    IFGeneralUtils::CopyFloat16ToMatrix(mScale, last_touched_object->OGL_body->modelview_matrix);
    t.matrix() = mScale;

    GLuint texture_index;

    if (buttons_body[0] == last_touched_object) {
     texture_index = 0;
    }
    else {
     texture_index = 1;
    }

    glDeleteTextures(1, &TEST_GUI_Tex_Ary[texture_index]);
    char outstring[120];
    strcpy(outstring, "touched");


    CIFTextRender TextRender;
  //  TextRender.InitTextRender("Roboto-Thin.ttf", User_Data.state);
    TextRender.InitTextRender("RobotoMono-Regular.ttf", User_Data.state);
    TextRender.SetBackgroundColor(20, 80, 20, 255);
    TextRender.SetForegroundColor(230, 230, 230, 255);
    TextRender.SetCharSize_px(60, 30);
    



    TEST_GUI_Tex_Ary[texture_index] = last_touched_object->OGL_body->texture_ID = TextRender.DrawText(outstring, 128);

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
void TESTFN_AddRandomBody(engine &engine) {
 if (engine.EGL_initialized) {

  if (!DEMO4_initialized) {
   DEMO4_initialized = true;





   ///////////////////////////////////////////////////////////////////////    LOAD GRAPHICS START










   thickness = RQNDKUtils::getDensityDpi(User_Data.state) / 25.4;
   left = -thickness*0.5; bottom = 0; right = engine.width; top = engine.height;
   Window2ObjectCoordinates(left, bottom, zDefaultLayer, engine.width, engine.height);
   left /= IFA_box2D_factor; bottom /= IFA_box2D_factor;
   Window2ObjectCoordinates(right, top, zDefaultLayer, engine.width, engine.height);
   right /= IFA_box2D_factor; top /= IFA_box2D_factor;









   b2EdgeShape *edgeShape;
   b2FixtureDef *fixture;
   b2PolygonShape *polyShape2;
   b2Vec2 shapeCoords[8];
   float zoom_factor;













   //Left Wall
   IFAdapter.OrderBody();
   IFAdapter.OrderedBody()->body_def->type = b2_staticBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :    
   edgeShape = new b2EdgeShape;
   edgeShape->Set(b2Vec2(left, bottom), b2Vec2(left, top));
   fixture = new b2FixtureDef;
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
   game_body[0]->OGL_body->texture_ID = TEST_GUI_Tex_Ary[0] = User_Data.CubeTexture;
   game_body[0]->OGL_body->line_thickness = thickness;


   //Right Wall
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
   game_body[1]->OGL_body->texture_ID = TEST_GUI_Tex_Ary[1] = User_Data.CubeTexture;
   game_body[1]->OGL_body->line_thickness = thickness;





   //Ball
   IFAdapter.OrderBody();
   IFAdapter.OrderedBody()->body_def->type = b2_dynamicBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody : 
   b2CircleShape *polyShape = new b2CircleShape;
   polyShape->m_p.SetZero();
   radius = engine.width > engine.height ? engine.width : engine.height, dummy = 0.0;
   radius *= 0.02;
   radius += engine.width*0.5;
   Window2ObjectCoordinates(radius, dummy, zDefaultLayer, engine.width, engine.height);
   radius /= IFA_box2D_factor;
   polyShape->m_radius = radius;
   fixture = new b2FixtureDef;
   fixture->shape = polyShape;
   fixture->density = 0.9;
   fixture->friction = 0.0;
   fixture->restitution = 1.0500;
   //fixture->filter.categoryBits = 0x0008;
   //fixture->filter.maskBits = 0x0010;
   IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape, fixture);
   game_body[2] = IFAdapter.OrderedBody();
   if (!IFAdapter.MakeBody())
    return;
   game_body[2]->OGL_body->texture_ID = TEST_GUI_Tex_Ary[2] = User_Data.Textures[2];






   //Player/lower paddle
   IFAdapter.OrderBody();
   IFAdapter.OrderedBody()->body_def->type = b2_kinematicBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :    b2_kinematicBody
   polyShape2 = new b2PolygonShape;   
   
   //zoom_factor = engine.width < engine.height ? engine.width : engine.height, dummy = 0.0;
   //zoom_factor /= 12.0 * 24.0;
   //zoom_factor += engine.width*0.5;
   //Window2ObjectCoordinates(zoom_factor, dummy, zDefaultLayer, engine.width, engine.height);
   //shapeCoords[0] = { zoom_factor *-5.0, zoom_factor * 0.0 };
   //shapeCoords[1] = { zoom_factor *-3, zoom_factor *  -2 };
   //shapeCoords[2] = { zoom_factor * 0,zoom_factor *-3};
   //shapeCoords[3] = { zoom_factor * 3,zoom_factor *-2};
   //shapeCoords[4] = { zoom_factor * 5,zoom_factor * 0};
   //shapeCoords[5] = { zoom_factor * 2,zoom_factor * 2};
   //shapeCoords[6] = { zoom_factor * 0,zoom_factor * 3 };
   //shapeCoords[7] = { zoom_factor *-2, zoom_factor * 2 };
   zoom_factor = (right-left)/(12.0 * 8.0);
   paddle_width = zoom_factor * 12.0 * 2.0;
   shapeCoords[0].x = zoom_factor * -12, shapeCoords[0].y = zoom_factor * 0;
   shapeCoords[1].x = zoom_factor * 12, shapeCoords[1].y = zoom_factor * 0;
   shapeCoords[2].x = zoom_factor * 12, shapeCoords[2].y = zoom_factor * 0;
   shapeCoords[3].x = 0,                shapeCoords[3].y = zoom_factor * 3.5;
   shapeCoords[4].x = zoom_factor * -12, shapeCoords[4].y = zoom_factor * 0;
   polyShape2->Set(shapeCoords, 5);
   fixture = new b2FixtureDef;
   fixture->shape = polyShape2;
   fixture->density = 1.0;
   fixture->friction = 0.0;
   fixture->restitution = 1.010;
   //fixture->filter.categoryBits = 0x0008;
   //fixture->filter.maskBits = 0x0010;
   IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape2, fixture);
   game_body[3] = IFAdapter.OrderedBody();
   if (!IFAdapter.MakeBody())
    return;
   game_body[3]->body->SetTransform(b2Vec2(0.0, -bottom * 0.6), 0.0);
   game_body[3]->OGL_body->texture_ID = TEST_GUI_Tex_Ary[3] = User_Data.CubeTexture;
   game_body[3]->body->SetFixedRotation(true);








   //AI/upper paddle
   IFAdapter.OrderBody();
   IFAdapter.OrderedBody()->body_def->type = b2_kinematicBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :    
   polyShape2 = new b2PolygonShape;
   //zoom_factor = engine.width < engine.height ? engine.width : engine.height, dummy = 0.0;
   //zoom_factor /= 12.0 * 24.0;
   //zoom_factor += engine.width*0.5;
   //Window2ObjectCoordinates(zoom_factor, dummy, zDefaultLayer, engine.width, engine.height);
   shapeCoords[0].x = zoom_factor * -12, shapeCoords[0].y = zoom_factor * 0.0;
   shapeCoords[1].x = 0, shapeCoords[1].y = zoom_factor * -3.5;
   shapeCoords[2].x = zoom_factor * 12, shapeCoords[2].y = zoom_factor * 0.0;
   shapeCoords[3].x = zoom_factor * 12, shapeCoords[3].y = zoom_factor * 0;
   shapeCoords[4].x = zoom_factor * -12, shapeCoords[4].y = zoom_factor * 0;
   polyShape2->Set(shapeCoords, 5);
   fixture = new b2FixtureDef;
   fixture->shape = polyShape2;
   fixture->density = 1.0;
   fixture->friction = 0.0;
   fixture->restitution = 1.010;
   //fixture->filter.categoryBits = 0x0008;
   //fixture->filter.maskBits = 0x0010;
   IFAdapter.OrderedBody()->AddShapeAndFixture(polyShape2, fixture);
   game_body[4] = IFAdapter.OrderedBody();
   if (!IFAdapter.MakeBody())
    return;
   game_body[4]->body->SetTransform(b2Vec2(0.0, bottom * 0.6), 0.0);
   game_body[4]->OGL_body->texture_ID = TEST_GUI_Tex_Ary[4] = User_Data.CubeTexture;





   //GUI Train button
   //Button Background
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
   shapeCoords[0] = { zoom_factor *-5, zoom_factor *-2.5 };
   shapeCoords[1] = { zoom_factor * 5, zoom_factor * -2.5 };
   shapeCoords[2] = { zoom_factor * 5, zoom_factor * 2.5 };
   shapeCoords[3] = { zoom_factor *-5, zoom_factor * 2.5 };
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
   game_body[6] = IFAdapter.OrderedBody();
   if (!IFAdapter.MakeBody())
    return;
   game_body[6]->body->SetTransform(b2Vec2(0.0, bottom - zoom_factor * 5), 0.0);
   game_body[6]->OGL_body->z_pos -= 0.1;
   TEST_GUI_Tex_Ary[6] = game_body[6]->OGL_body->texture_ID = TextRender.DrawText(" ", 128);
   //////////////////////BUTTON
   IFAdapter.OrderBody();
   IFAdapter.OrderedBody()->body_def->type = b2_staticBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :    
   polyShape2 = new b2PolygonShape;
#define  zoom_factor 1.45
   //shapeCoords[0] = { zoom_factor *-5.0, zoom_factor * 0.0 };
   //shapeCoords[1] = { zoom_factor *-3, zoom_factor *  -2 };
   //shapeCoords[2] = { zoom_factor * 0,zoom_factor *-3};
   //shapeCoords[3] = { zoom_factor * 3,zoom_factor *-2};
   //shapeCoords[4] = { zoom_factor * 5,zoom_factor * 0};
   //shapeCoords[5] = { zoom_factor * 2,zoom_factor * 2};
   //shapeCoords[6] = { zoom_factor * 0,zoom_factor * 3 };
   //shapeCoords[7] = { zoom_factor *-2, zoom_factor * 2 };
   shapeCoords[0] = { zoom_factor *-5, zoom_factor *-2.5 };
   shapeCoords[1] = { zoom_factor * 5, zoom_factor * -2.5 };
   shapeCoords[2] = { zoom_factor * 5, zoom_factor * 2.5 };
   shapeCoords[3] = { zoom_factor *-5, zoom_factor * 2.5 };
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
   game_body[5]->body->SetTransform(b2Vec2(0.0, bottom - zoom_factor * 5), 0.0);
   game_body[5]->OGL_body->z_pos -= 0.1;   
   char outstring[1200];
   strcpy(outstring, "train");
   //TEST_textid = first_body->OGL_body->texture_ID = DrawText(outstring, 5, FT_Vector()={160*64,40*64}, 3.141593*0.50, &TEST_text_ub, &TEST_text_vb, &TEST_text_ut, &TEST_text_vt);
   TEST_GUI_Tex_Ary[5] = game_body[5]->OGL_body->texture_ID = TextRender.DrawText("0", 128);

//("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",512);
   // (FT_Pos)(( 0.0 * (( 64.0 * 72.0 ) / (float)RQNDKUtils::getDensityDpi(User_Data.state))) * 64 ),
   // (FT_Pos)(( 1.0 * (( 64.0 * 72.0 ) / (float)RQNDKUtils::getDensityDpi(User_Data.state) ) ) * 64 )
   // },
   //128);
   //TEST_GUI_Tex_Ary[0] = game_body[5]->OGL_body->texture_ID = DrawText(outstring, 15, FT_Vector() = { 160 * 64,40 * 64 }, 0.0, &TEST_text_ub, &TEST_text_vb, &TEST_text_ut, &TEST_text_vt);
   //TEST_textid = first_body->OGL_body->texture_ID = DrawText(outstring, 5, FT_Vector() = { 40 * 64,60 * 64 }, 3.141593*0.0, &TEST_text_ub, &TEST_text_vb, &TEST_text_ut, &TEST_text_vt);
   //////////////////////////////////////////////////////////////////////////////////////
   //{
   // char print_char = 'a';
   // SFloatRect *char_rect;
   // char_rect = TextRender.CharMap.GetRef(print_char);
   // float deltabbx, deltabby;
   // deltabbx = char_rect->xMax - char_rect->xMin;
   // deltabby = char_rect->yMax - char_rect->yMin;
   // size_t UVsize = game_body[5]->OGL_body->UVmapping_cnt;
   // for (int cntuv = 0; cntuv < UVsize; cntuv++) {
   //  game_body[5]->OGL_body->UVmapping[cntuv] *= deltabbx;
   //  game_body[5]->OGL_body->UVmapping[cntuv] += char_rect->xMin;
   //  cntuv++;
   //  game_body[5]->OGL_body->UVmapping[cntuv] *= deltabby;
   //  game_body[5]->OGL_body->UVmapping[cntuv] +=char_rect->yMin;
   //  game_body[5]->OGL_body->z_pos = zDefaultLayer*1.1;
   // }
   //}
   if(false){
    char print_char = 'i';//null box does not encompass whole text
    SFloatRect *char_rect;
    char_rect = TextRender.CharMap.GetRef(print_char);
    size_t UVsize = game_body[5]->OGL_body->UVmapping_cnt;
    game_body[5]->OGL_body->UVmapping[0] = char_rect->xMax / TextRender.expanded_width;
    game_body[5]->OGL_body->UVmapping[1] = char_rect->yMin / TextRender.expanded_height;

    game_body[5]->OGL_body->UVmapping[2] = char_rect->xMax / TextRender.expanded_width;
    game_body[5]->OGL_body->UVmapping[3] = char_rect->yMax / TextRender.expanded_height;

    game_body[5]->OGL_body->UVmapping[4] = char_rect->xMin / TextRender.expanded_width;
    game_body[5]->OGL_body->UVmapping[5] = char_rect->yMax / TextRender.expanded_height;

    game_body[5]->OGL_body->UVmapping[6] = char_rect->xMin / TextRender.expanded_width;
    game_body[5]->OGL_body->UVmapping[7] = char_rect->yMin / TextRender.expanded_height;

   }



   //

   ////Left Wall
   //IFAdapter.OrderBody();
   //IFAdapter.OrderedBody()->body_def->type = b2_staticBody;//b2_dynamicBody;//((drand48() > 0.5) ? b2_staticBody :    
   //edgeShape = new b2EdgeShape;
   //edgeShape->Set(b2Vec2(left, bottom), b2Vec2(left, top));
   //fixture = new b2FixtureDef;
   //fixture->shape = edgeShape;
   //fixture->density = 1.0;
   //fixture->density = 1.0;
   //fixture->friction = 0.0;
   //fixture->restitution = 1.000;
   ////fixture->filter.categoryBits = 0x0008;
   ////fixture->filter.maskBits = 0x0010;
   //IFAdapter.OrderedBody()->AddShapeAndFixture(edgeShape, fixture);
   //game_body[0] = IFAdapter.OrderedBody();
   //if (!IFAdapter.MakeBody())
   // return;
   //game_body[0]->body->SetTransform(b2Vec2(0.0, 0.0), 0.0);
   //game_body[0]->OGL_body->texture_ID = User_Data.CubeTexture;
   //game_body[0]->OGL_body->line_thickness = thickness;



   //Reduce image to occupy only part of the scren
   IFAdapter.CalculateBox2DSizeFactor(25);

   ///////////////////////////////////////////////////////////////////////    LOAD GRAPHICS STOP




   PongBallStateMachine.ResetBallState();



   //Node1 = new IFFANNEngine::CNode;
   //Node1->IsRunning = true;
   //if (IFFANN::Check_Save_Cascade_FANN("pongpaddle", IFFANN::CnFinalFannPostscript)) {
   // IFFANN::Load_Cascade_FANN(&Node1->ifann, "pongpaddle", IFFANN::CnFinalFannPostscript);
   //}   
   //FANNPong.input_neurons = 4;
   //FANNPong.max_neurons = 30;
   //FANNPong.desired_error = 0.00000;
   //FANNPong.input_scale = 10.0;
   //FANNPong.output_scale = 10.0;
   //if (!Node1->ifann.ann){
   // IFFANN::Setup_Train_Cascade_FANN(IFFANN::Create_Cascade_FANN(IFFANN::Init_Cascade_FANN(&Node1->ifann), FANNPong.input_neurons, FANNPong.output_neurons, "pongpaddle"), FANNPong.max_neurons, FANNPong.neurons_between_reports, FANNPong.desired_error, FANNPong.input_scale, FANNPong.output_scale);
   // IFFANN::Save_Cascade_FANN(&Node1->ifann, IFFANN::CnFinalFannPostscript);
   //}
   //Node1->LoadCore("pongpaddle");
   /////////////////////////////////////



  static float train_size_factor = 0.3;
  static float neuron_count_factor = 0.6;
      ////////////////////////////  Load Nodes START
   NetworkNodes[0].FANNOptions.input_neurons = 3;
   NetworkNodes[0].FANNOptions.max_neurons = 10 * neuron_count_factor;
   NetworkNodes[0].FANNOptions.desired_error = 0.000;
   NetworkNodes[0].FANNOptions.input_scale = 0.1;
   NetworkNodes[0].FANNOptions.output_scale = 0.1;
   NetworkNodes[0].node_data_counter_limit = 500*train_size_factor;
   NetworkNodes[0].Load_Node( "pongpaddle");
   NetworkNodes[0].Load_Train_Data();      
   NetworkNodes[0].Node->AddFlag(TNodeStates::E_Training);
   NetworkNodes[0].Node->AddFlag(TNodeStates::E_ContTraining);


   NetworkNodes[1].FANNOptions.input_neurons = 6;//x, y, linear atan2 - paddle bounce off, x, y of impact, atan2 on impact
   NetworkNodes[1].FANNOptions.max_neurons = 30 * neuron_count_factor;
   NetworkNodes[1].FANNOptions.desired_error = 0.000;
   NetworkNodes[1].FANNOptions.input_scale = 0.1;
   NetworkNodes[1].FANNOptions.output_scale = 0.1;
   NetworkNodes[1].node_data_counter_limit = 250*train_size_factor;
   NetworkNodes[1].Load_Node("pongpaddlebounce", false);
   NetworkNodes[1].Load_Train_Data();
   NetworkNodes[1].Node->AddFlag(TNodeStates::E_Training);
   NetworkNodes[1].Node->AddFlag(TNodeStates::E_ContTraining);

   NetworkNodes[2].FANNOptions.input_neurons = 7;//
   NetworkNodes[2].FANNOptions.max_neurons = 30 * neuron_count_factor;
   NetworkNodes[2].FANNOptions.desired_error = 0.000;
   NetworkNodes[2].FANNOptions.input_scale = NetworkNodes[1].FANNOptions.input_scale;
   NetworkNodes[2].FANNOptions.output_scale = NetworkNodes[1].FANNOptions.output_scale;
   NetworkNodes[2].node_data_counter_limit = 250*train_size_factor;
   NetworkNodes[2].Load_Node("pongpaddlebouncetune");
   NetworkNodes[2].Load_Train_Data();
   NetworkNodes[2].Node->AddFlag(TNodeStates::E_Training);
   NetworkNodes[2].Node->AddFlag(TNodeStates::E_ContTraining);

   //Inputs x start, arctan2(speed vec y, ), magnitude(speedvec), ball end x, input 1, input 2, input 3
   NetworkNodes[3].FANNOptions.input_neurons = 7;//
   NetworkNodes[3].FANNOptions.max_neurons = 40 * neuron_count_factor;
   NetworkNodes[3].FANNOptions.desired_error = 0.00000;
   NetworkNodes[3].FANNOptions.input_scale = 0.1;
   NetworkNodes[3].FANNOptions.output_scale = 0.1;
   NetworkNodes[3].node_data_counter_limit= 1000*train_size_factor;
   NetworkNodes[3].Load_Node("pongpaddleselect");
   NetworkNodes[3].Load_Train_Data();
   NetworkNodes[3].Node->AddFlag(TNodeStates::E_Training);
   NetworkNodes[3].Node->AddFlag(TNodeStates::E_ContTraining);

   //Network node MAIN BRAIN
   //On inputs it receives:
   //PAD POSITION X at the moment of ball leaving pad
   //PAD HORIZONTAL LINEAR VELOCITY MAGNITUDE at the moment ball leaving pad
   //BALL Position X, leaving pad
   //BALL linear velocity
   //BALL linear velocity magnitude
   //1/0 - network 1 was running
   //1/0 - networks 2 and 2_1 were running
   //1/0 - network 3 was running
   //PLAYER PAD POSITION X AT THE MOMENT OF BALL LEAVING PAD
   
   //output 1 - last net running was 1
   //output 2 - last net running was 2
   //output 3 - last net running was 3

   NetworkNodes[4].FANNOptions.input_neurons = 8;//
   NetworkNodes[4].FANNOptions.output_neurons = 3;//
   NetworkNodes[4].FANNOptions.max_neurons = 30 * neuron_count_factor;
   NetworkNodes[4].FANNOptions.desired_error = 0.00000;
   NetworkNodes[4].FANNOptions.input_scale = 0.1;
   NetworkNodes[4].FANNOptions.output_scale = 0.1;
   NetworkNodes[4].node_data_counter_limit = 1000 * train_size_factor;
   NetworkNodes[4].Load_Node("pongmainbrain");
   NetworkNodes[4].Load_Train_Data();
   NetworkNodes[4].Node->AddFlag(TNodeStates::E_Training);
   NetworkNodes[4].Node->AddFlag(TNodeStates::E_ContTraining);


   ////////////////////////////  Load Nodes STOP

   User_Data.TrainInProgress = false;


   //Launch the ball
   //float launch_y = (drand48()) * 500.0 - 250.0;
   float launch_y = (drand48()) * 150.0+100;
   game_body[2]->body->ApplyLinearImpulse(b2Vec2(drand48()*500.0-500.0, launch_y), game_body[2]->body->GetPosition(), true);


//   trained[0] = trained[1] = trained[2] = trained[3] =true;
   
   //if ((trained[0] == true) && (trained[1] == true) && (trained[2] == true)) {
   // NetworkNodes[0].Node->IsRunning = true;
   // NetworkNodes[1].Node->IsRunning = true;
   // NetworkNodes[2].Node->IsRunning = true;
   // Select1_Node->IsRunning = true;
   //}
   //else {
   // NetworkNodes[0].Node->IsRunning = true;
   // NetworkNodes[1].Node->IsRunning = true;
   // NetworkNodes[2].Node->IsRunning = true;
   // Select1_Node->IsRunning = true;
   //}


  }else {

   float maxx = engine.width;
   float maxy = engine.height;
   Window2ObjectCoordinates(maxx, maxy, zDefaultLayer, maxx, maxy);
   float cut_off_distance = b2Distance(b2Vec2(0, 0), b2Vec2(maxx / IFA_box2D_factor, maxy / IFA_box2D_factor)) * 1;
   {
//    b2Vec2 position = game_body[3]->body->GetPosition();
//    game_body[3]->body->SetTransform(b2Vec2(-last_x_acceleration * 2.9, -bottom*0.6), game_body[3]->body->GetAngle());
    //If ball is traveling too slowly towards player (any) accelerate it
    float ball_travel_angle = atan2(game_body[2]->body->GetLinearVelocity().y, game_body[2]->body->GetLinearVelocity().x);
    if (ball_travel_angle < 0.0) {     
     IFA_World->SetGravity(b2Vec2(0.0, -1.0));
    }
    else {
     IFA_World->SetGravity(b2Vec2(0.0, 1.0));
    }
   }
   //Is ball outside of the screen
   float balldistancecenter = b2Distance(b2Vec2(game_body[2]->body->GetPosition().x, game_body[2]->body->GetPosition().y), b2Vec2(0, 0));
   if (balldistancecenter > cut_off_distance) {    

    //Delete last paddle node1 data
    //Limit_Node1_data_size(true);
    NetworkNodes[0].CheckNodeTrainDataState();
    ball_to_pad_prev_distance = FLT_MAX;
    //
    //ball_state = -2;

    //Reset the ball to the center
    game_body[2]->body->SetTransform(b2Vec2(0, 0), game_body[2]->body->GetAngle());
    game_body[2]->body->SetLinearVelocity(b2Vec2(0, 0));
    game_body[2]->body->SetAngularVelocity(0);
    //static float direction_x = -1.0;
    //game_body[2]->body->ApplyLinearImpulse((b2Vec2((drand48()*2.0-1.0)* 550.0, (drand48() * -1.0) * 300.0)), game_body[2]->body->GetPosition(), true);
    //float launch_y = drand48()*1000.0 - 500.0;
    float launch_y = (drand48()) * 150.0 + 100;
    game_body[2]->body->ApplyLinearImpulse(b2Vec2(drand48()*500.0 - 500.0, launch_y), game_body[2]->body->GetPosition(), true);
    NetworkNodes[0].Node->IsRunning = false;
    NetworkNodes[1].Node->IsRunning = false;
    NetworkNodes[2].Node->IsRunning = false;
    NetworkNodes[3].Node->IsRunning = false;
    NetworkNodes[4].Node->IsRunning = true;
    //direction_x+=0.1;
    //if(direction_x>1.0)direction_x = -1.0;

    //for(unsigned int cnt = 0; cnt < sizeof(ndelta_x)/ sizeof(ndelta_x[0]); cnt++){
    // ndelta_x[cnt] = ndelta_y[cnt] = 0;
    //}

   }  else {

    User_Data.TrainInProgress = AutoPlayer;
    
    game_body[3]->body->SetLinearVelocity(b2Vec2(0.0,0.0));
    if (IFGameEditor::GetTouchEvent()) {
     
     //while ((IFGameEditor::GetTouchEvent(&touchx, &touchy)));
     
     struct timespec temp_timespec;
     clock_gettime(CLOCK_MONOTONIC, &temp_timespec);
     //temp_int64 = timespec2ms64(&temp_timespec) - timespec2ms64(&game_time_0);
     //unsigned long int temp_int64 = RQNDKUtils::timespec2ms64(&temp_timespec) - RQNDKUtils::timespec2ms64(&Last_GUI_Click_Time);
     unsigned long int temp_int64 = RQNDKUtils::timespec2ms64(&temp_timespec);     
     unsigned long int input_event_time_stamp;
     //if (temp_int64 > 0) 
     {
      input_event_time_stamp = IFGameEditor::GetTouchEvent(&touchx, &touchy);
      {
       //IFGameEditor::GetTouchEvent(&touchx, &touchy);

       //Move players paddle
       float screenx = touchx;
       float screeny = touchy;
       Window2ObjectCoordinates(screenx, screeny, zDefaultLayer, engine.width, engine.height);
       //game_body[3]->body->SetTransform(b2Vec2(screenx / IFA_box2D_factor, game_body[3]->body->GetPosition().y), game_body[3]->body->GetAngle());
       float desired_x = (screenx) / IFA_box2D_factor - game_body[3]->body->GetPosition().x;
       game_body[3]->body->SetLinearVelocity(
        b2Vec2(desired_x*desired_x*desired_x, 0.0));
       //Player may press train button but not more often than once per second       
       if ((input_event_time_stamp - Last_GUI_Click_Time) > 1000000000) {
        Last_GUI_Click_Time = input_event_time_stamp;
        //while (IFGameEditor::GetTouchEvent(&touchx, &touchy));
        typename std::list<ifCB2Body*>::iterator iter;
        for (iter = IFAdapter.Bodies.begin(); iter != IFAdapter.Bodies.end(); iter++) {
         if (game_body[5] == *iter) {
          if (B2BodyUtils.RayTestHitpoint(touchx, touchy, *iter)) {                      

           //if (input_data_sets > 20) {
           //Pong_Learning_Phase = true;
           AutoPlayer = !AutoPlayer;
           if(AutoPlayer){
            trained[0] = trained[1] = trained[2] = false;
            trained[3] = false;

            //if((trained[0]==true)&&(trained[1] == true)&&(trained[2] == true)){
            // trained[3] = false;
            //}else{
            // trained[3] = true;
            //}
           }else{
            if (!trained[0]){
             NetworkNodes[0].Train_Node();
            }
            if (!trained[1]) {
             NetworkNodes[1].Train_Node();
            }
            if (!trained[2]) {
             NetworkNodes[2].Train_Node();
            }
            if(!trained[3]){
             NetworkNodes[3].Train_Node();
             //Train_Node(Select1_Node,&Select1_FANNPong, Select1_Node->ifann.unique_name,&select1_input_data, &select1_output_data);
            }
            if (!trained[4]) {
             NetworkNodes[4].Train_Node();
             //Train_Node(Select1_Node,&Select1_FANNPong, Select1_Node->ifann.unique_name,&select1_input_data, &select1_output_data);
            }
            //NetworkNodes[3].Save_Train_Data();
            //Setup_Train_Cascade_FANN_Train_Callback(NetworkNodes[3].Node->ifann.ann, &select1_input_data, &select1_output_data);
            //IFFANN::Train_Cascade_FANN(&NetworkNodes[3].Node->ifann, Train_Cascade_FANN_Train_Callback, select1_output_data.size(), 2, false);

            game_body[3]->body->SetLinearVelocity(b2Vec2(0,0));
            if(trained[0]){
             NetworkNodes[0].Save_Fann();
             //IFFANN::Save_Cascade_FANN(&NetworkNodes[0].Node->ifann, IFFANN::CnFinalFannPostscript);              
            }
            if (trained[1]) {
             NetworkNodes[1].Save_Fann();
             //IFFANN::Save_Cascade_FANN(&NetworkNodes[1].Node->ifann, IFFANN::CnFinalFannPostscript);
            }
            if (trained[2]) {
             NetworkNodes[2].Save_Fann();
             //IFFANN::Save_Cascade_FANN(&NetworkNodes[2].Node->ifann, IFFANN::CnFinalFannPostscript);
            }
            if (trained[3]) {
             NetworkNodes[3].Save_Fann();
             //IFFANN::Save_Cascade_FANN(&NetworkNodes[3].Node->ifann, IFFANN::CnFinalFannPostscript);
            }
            if (trained[4]) {
             NetworkNodes[4].Save_Fann();
             //IFFANN::Save_Cascade_FANN(&NetworkNodes[3].Node->ifann, IFFANN::CnFinalFannPostscript);
            }
           }
           //}

           //clock_gettime(CLOCK_MONOTONIC, &Last_GUI_Click_Time);
           break;
          }
         }
        }        

       }

      }      
     }
    }else{
     if(AutoPlayer){
      if(b2Distance(game_body[3]->body->GetPosition(), game_body[2]->body->GetPosition())>(radius*3.0)){
       game_body[3]->body->SetLinearVelocity(b2Vec2(((game_body[2]->body->GetPosition().x - game_body[3]->body->GetPosition().x
        ) + (drand48()*2.0 - 1.0))*10.0, 0.0));
       //game_body[3]->body->SetTransform(b2Vec2(game_body[2]->body->GetPosition().x + (drand48()-0.5)*2.0, game_body[3]->body->GetPosition().y), game_body[3]->body->GetAngle());
       //game_body[3]->body->ApplyLinearImpulse(b2Vec2((game_body[2]->body->GetPosition().x - game_body[3]->body->GetPosition().x + (drand48() - 0.5)*2.0)*100.0, 0.0), game_body[3]->body->GetWorldCenter(), true);      
      }
     }
    }

    //game_body[3]->body->SetTransform(b2Vec2(game_body[3]->body->GetPosition().x, -game_body[3]->body->GetLinearVelocity().y), game_body[3]->body->GetAngle());


    {/////////////////////////////////////  TRAINIG START

      //Train network
     for (unsigned int cnt = 0; cnt < 5; cnt++) {
      //if (!trained[cnt]) {
       if (NetworkNodes[cnt].Node->GetFlag(TNodeStates::E_ContTraining)) {
        trained[cnt] = NetworkNodes[cnt].Epoch_Train();
       //}
       //else {
        //trained[cnt] = NetworkNodes[cnt].Train_Node();
      // }
      }
     }





     if(AutoPlayer){

      for (unsigned int cnt = 0; cnt < 4; cnt++) {
       if (!trained[cnt]) {
        if (NetworkNodes[cnt].Node->GetFlag(TNodeStates::E_ContTraining)) {
         //trained[cnt] = NetworkNodes[cnt].Epoch_Train();
        }
        else {
         trained[cnt] = NetworkNodes[cnt].Train_Node();
        }
       }
      }

      if((trained[0] == true)&&
      (trained[1] == true)&&
      (trained[2] == true)&&
       (trained[3] == true) &&
       (trained[4] == true)
      ){
       AutoPlayer = false;
       game_body[3]->body->SetLinearVelocity(b2Vec2(0, 0));       
      }
     
     }else{
      // CONSTANT TRAINIG UNTIL ERROR REACHED
      NetworkNodes[3].Epoch_Train(false, NetworkNodes[3].NumTrainData(), 2000);
     }

     //input_data_sets = 0;
     //input_data.clear();
     //output_data.clear();




     //float ball_angle_to_pad;
     b2Vec2 balllinvel = game_body[2]->body->GetLinearVelocity();
     b2Vec2 ballposition = game_body[2]->body->GetPosition();
     b2Vec2 paddleposition;
     bool AIPaddle = false;
     
     if (ballposition.y > 0) {
      AIPaddle = true;
     }
     if(AIPaddle != LastAIPaddle ){
      LastAIPaddle = AIPaddle;
      //Node1_data_head = input_data_sets;
      if(!AIPaddle){
       NetworkNodes[0].Node->IsRunning = false;
       NetworkNodes[1].Node->IsRunning = false;
       NetworkNodes[2].Node->IsRunning = false;
      }else{
      }
      //for (unsigned int cnt = 0; cnt < sizeof(train_ndelta_x)/ sizeof(train_ndelta_x[0]); cnt++) {
      // train_ndelta_x[cnt] = train_ndelta_y[cnt] = 0;
      //}


     }

     {
      if (ballposition.y>0) {
       paddleposition = game_body[4]->body->GetPosition();
      }else{
       paddleposition = game_body[3]->body->GetPosition();
      }
//if (AutoPlayer)
{
      //Store data to train
      //if(abs(paddleposition.y) > abs(ballposition.y)) {

      PongBallStateMachine.DetermineBallState(game_body);
      PongBallStateMachine.ProcessBallState(game_body);      

      if( balllinvel.y > 0 ){ //PongBallStateMachine.BallState.PlayfieldLocation == CPongBallStateMachine::E_TravelUp ){
       if( ballposition.y > ( bottom * 0.6 * 0.6 )){
        NetworkNodes[3].Node->IsRunning = false;
        NetworkNodes[0].Node->IsRunning = true;
       }else{
        NetworkNodes[0].Node->IsRunning = false;
       }
      }
     
      
      ////////////////////////////////////////////////////////////////////////////////
      static int skipCnt = 0;
      static float prev_dist = 0;
      static float prev_padx = 0;
      static float skipnum = 0;
      ballposition = game_body[2]->body->GetPosition();
      if (ballposition.y > 0) {
       //paddleposition = game_body[4]->body->GetPosition();
       paddleposition = b2Vec2(right*1000.0, top*1000.0);
      }
      else {
       paddleposition = game_body[3]->body->GetPosition();
      }
      float bal_pad_distance = b2Distance(paddleposition, ballposition);

      //if (bal_pad_distance > 6.0){
      // skipnum = bal_pad_distance*2;
      //}else{
      // skipnum = 0;
      //}
      //if (bal_pad_distance < 6.0)
      // skipnum = 0; 
      //else
      // skipnum = 2;

      if ((skipCnt++ >= skipnum)) {
       skipCnt = 0;        
       if(bal_pad_distance < (radius*8.0)){//Ball close to pad
        if(//(abs(abs(paddleposition.x)- abs(ballposition.x))<(paddle_width*0.5))&&//Ball is not close but to the side
           (abs(ballposition.y)<(abs(paddleposition.y) + radius))//Ball is not behind pad
           //&&   (((abs(balllinvel.y / balllinvel.x)))>1.0)//Ball is traveling in y direction two times faster than in x direction
        // &&(abs(abs(ballposition.x) - abs(paddleposition.x))<radius)
          )
        {
         static float prev_val01 = 0.0f;
         if(AIPaddle){ 
          if (abs(b2Distance(paddleposition, ballposition)) < ball_to_pad_prev_distance){
           ball_to_pad_prev_distance = abs(b2Distance(paddleposition, ballposition));
           if ((prev_val01 - b2Distance(ballposition, paddleposition))) {
            NetworkNodes[0].node_input_data.push_back((prev_val01 - b2Distance(ballposition, paddleposition)) * NetworkNodes[0].Node->ifann.input_scale);
           }
           else {
            NetworkNodes[0].node_input_data.push_back(0);
           }
           prev_val01 = b2Distance(ballposition, paddleposition);

           if (balllinvel.y && balllinvel.x) {
            NetworkNodes[0].node_input_data.push_back((atan2(-balllinvel.y, -balllinvel.x) - b2_pi));
           }
           else {
            NetworkNodes[0].node_input_data.push_back(0);
           }

           NetworkNodes[0].node_input_data.push_back(-ballposition.x * NetworkNodes[0].Node->ifann.input_scale);


           //if (ballposition.x < 0) {
           // input_data.push_back(-(left - ballposition.x)/ NetworkNodes[0].Node->ifann.input_scale);
           //}
           //else {
           // input_data.push_back((right - ballposition.x) / NetworkNodes[0].Node->ifann.input_scale);
           //}
           //input_data.push_back(b2Distance(ballposition, paddleposition) / NetworkNodes[0].Node->ifann.input_scale);
           

           //train_ndelta_x[0] = -ballposition.x;
           //train_ndelta_y[0] = -ballposition.y;
           //for(unsigned int cnt = 0; cnt < sizeof(train_ndelta_x)/ sizeof(train_ndelta_x[0]); cnt++){
           // input_data.push_back((train_ndelta_x[cnt] = train_ndelta_x[cnt+1] - train_ndelta_x[cnt]));
           //}
           //for (unsigned int cnt = 0; cnt < sizeof(train_ndelta_y)/ sizeof(train_ndelta_y[0]); cnt++) {
           // input_data.push_back((train_ndelta_y[cnt] = train_ndelta_y[cnt + 1] - train_ndelta_y[cnt]));
           //}
           if (ballposition.x) {
            NetworkNodes[0].node_output_data.push_back((-ballposition.x) * NetworkNodes[0].Node->ifann.output_scale);
           }
           else {
            NetworkNodes[0].node_output_data.push_back(0);
           }

           

           prev_padx = -paddleposition.x;
          }else{
           ball_to_pad_prev_distance = FLT_MAX;
          }
         }else{

          //train_ndelta_x[0] = ballposition.x;
          //train_ndelta_y[0] = ballposition.y;
          //for (unsigned int cnt = 0; cnt < sizeof(train_ndelta_x)/ sizeof(train_ndelta_x[0]); cnt++) {
          // input_data.push_back((train_ndelta_x[cnt] = train_ndelta_x[cnt + 1] - train_ndelta_x[cnt]));
          //}
          //for (unsigned int cnt = 0; cnt < sizeof(train_ndelta_y)/sizeof(train_ndelta_y[0]); cnt++) {
          // input_data.push_back((train_ndelta_y[cnt] = train_ndelta_y[cnt + 1] - train_ndelta_y[cnt]));
          //}
           
          if((prev_val01 - b2Distance(paddleposition, ballposition))){
           NetworkNodes[0].node_input_data.push_back((prev_val01 - b2Distance(paddleposition, ballposition)) * NetworkNodes[0].Node->ifann.input_scale);
          }else{
           NetworkNodes[0].node_input_data.push_back(0);
          }
          prev_val01 = b2Distance(paddleposition, ballposition);
          if (balllinvel.y && balllinvel.x) {
           NetworkNodes[0].node_input_data.push_back(atan2(balllinvel.y, balllinvel.x ));
          }else{
           NetworkNodes[0].node_input_data.push_back(0);
          }
          //prev_dist = b2Distance(paddleposition, ballposition);
          NetworkNodes[0].node_input_data.push_back(ballposition.x * NetworkNodes[0].Node->ifann.input_scale);

          //if (ballposition.x < 0) {
          // input_data.push_back((left - ballposition.x)/ NetworkNodes[0].Node->ifann.input_scale);
          //}
          //else {
          // input_data.push_back(-(right - ballposition.x)/ NetworkNodes[0].Node->ifann.input_scale);
          //}
          //input_data.push_back(b2Distance(paddleposition, ballposition) / NetworkNodes[0].Node->ifann.input_scale);

         // input_data.push_back(game_body[3]->body->GetPosition().x);
          //input_data.push_back((paddleposition.y - ballposition.y) );
          //input_data.push_back((paddleposition.x) / NetworkNodes[0].Node->ifann.input_scale);
          //input_data.push_back((paddleposition.y) / NetworkNodes[0].Node->ifann.input_scale);
          //output_data.push_back(paddleposition.x / NetworkNodes[0].Node->ifann.output_scale);
          //input_data.push_back((paddleposition.x) / NetworkNodes[0].Node->ifann.input_scale);
          //input_data.push_back((ballposition.x) / NetworkNodes[0].Node->ifann.input_scale);
          if(ballposition.x){
           NetworkNodes[0].node_output_data.push_back((ballposition.x) * NetworkNodes[0].Node->ifann.output_scale);
          }else{
           NetworkNodes[0].node_output_data.push_back(0);
          }
          prev_padx = paddleposition.x;
         }
         
         //Limit size of training set
         //Limit_Node1_data_size();
         NetworkNodes[0].CheckNodeTrainDataState();
         NetworkNodes[0].Node->AddFlag(TNodeStates::E_Training);
         //IFFANN::Train_Cascade_FANN(&NetworkNodes[0].Node->ifann, Train_Cascade_FANN_PaddleBrains_Callback, input_data_sets, 2, false);
        }else if(abs(ballposition.y) < abs(paddleposition.y)){//Ball is behind pad
        }
       }
      }
      //}
}

      ////////////////////////////////////////////////////////////////////////////// NETWORK EXECUTION START
      if (true) { 

       if(AI_paddle_desired_position_x != AI_paddle_desired_position_x)
        AI_paddle_desired_position_x = right * 2.0;
       b2Vec2 position = game_body[4]->body->GetPosition();
       float ball_pad_distance = AI_paddle_desired_position_x - game_body[4]->body->GetPosition().x;
       game_body[4]->body->SetLinearVelocity(
        b2Vec2(ball_pad_distance*ball_pad_distance*ball_pad_distance, 0.0));//+2.0 is to make sure result is larger than 1
       if (game_body[4]->body->GetPosition().x < left*2.0)game_body[4]->body->SetTransform(b2Vec2(left*2.0, position.y), game_body[4]->body->GetAngle());
       if (game_body[4]->body->GetPosition().x > right*2.0)game_body[4]->body->SetTransform(b2Vec2(right*2.0, position.y), game_body[4]->body->GetAngle());


       ConnectNetwork01();       
       paddleposition = game_body[4]->body->GetPosition();
       unsigned int ID;
       fann_type *pin_value;
       //Set all inputs to some value
       Network.NodeRegister.InputPinRegister.input_pins.ResetIterator();
       //ndelta_x[0] = -ballposition.x;
       //ndelta_y[0] = -ballposition.y;
       //if((trained[0]==true)&&(trained[1]==true)&&(trained[2]==true)){
       // NetworkNodes[0].Node->IsRunning = true;
       // NetworkNodes[1].Node->IsRunning = true;
       // NetworkNodes[2].Node->IsRunning = true;
       // //Select1_Node->IsRunning = true;
       //}


       
       if(AutoPlayer){
        if(NetworkNodes[3].Node->IsRunning){
         NetworkNodes[0].Node->IsRunning = true;
         NetworkNodes[1].Node->IsRunning = true;
         NetworkNodes[2].Node->IsRunning = true;
        }else{
         if(trained[0])
          NetworkNodes[0].Node->IsRunning = false;
         else
          NetworkNodes[0].Node->IsRunning = true;
         NetworkNodes[1].Node->IsRunning = false;
         NetworkNodes[2].Node->IsRunning = false;
        }
       }

//NetworkNodes[0].Node->IsRunning = true;//ERR01

       while (0 <= Network.NodeRegister.InputPinRegister.input_pins.GetNextIterator(ID, pin_value)) {
        //if() 
        {         
         if(NetworkNodes[0].Node == Network.GetNodeByPinID(ID)){         
          unsigned int pinindex = Network.GetNodeByPinID(ID)->GetInPinByID(ID)->fann_index;
          //if(pinindex <20){
          // *pin_value = ndelta_x[pinindex] = ndelta_x[pinindex +1]- ndelta_x[pinindex];
          //}else if (pinindex < 40) {
          // *pin_value = ndelta_y[pinindex -20] = ndelta_y[pinindex + 1-20] - ndelta_y[pinindex -20];
          //}
          switch (pinindex) {
          case 0:
           static float prev_val01 = 0.0f;
           if((prev_val01 - b2Distance(ballposition, paddleposition))){
            *pin_value = (prev_val01 - b2Distance(ballposition, paddleposition)) * NetworkNodes[0].Node->ifann.input_scale;
           }else{
            *pin_value = 0;
           }
           prev_val01 = b2Distance(ballposition, paddleposition);
           break;
          case 1:
           //         *pin_value = -ballposition.x / NetworkNodes[0].Node->ifann.input_scale;
           if(balllinvel.y && balllinvel.x){
            *pin_value = (atan2(-balllinvel.y, -balllinvel.x ) - b2_pi);
           }else{
            *pin_value = 0;
           }
           break;
          case 2:
           *pin_value = -ballposition.x * NetworkNodes[0].Node->ifann.input_scale;
           break;
          case 3:
           if(ballposition.x< 0){
            *pin_value = -(left- ballposition.x);
           }else{
            *pin_value = (right - ballposition.x);
           }
           *pin_value*= NetworkNodes[0].Node->ifann.input_scale;
           break;
          case 4:
           *pin_value = b2Distance(ballposition, paddleposition) * NetworkNodes[0].Node->ifann.input_scale;
           break;
          };
         }else if (NetworkNodes[1].Node == Network.GetNodeByPinID(ID)) {
         }else if (NetworkNodes[2].Node == Network.GetNodeByPinID(ID)) {          
          unsigned int pinindex = Network.GetNodeByPinID(ID)->GetInPinByID(ID)->fann_index;
          if(pinindex<5){
           NetworkNodes[2].Node->inputs[pinindex] = NetworkNodes[1].Node->inputs[pinindex];
          }
         }
         else if (NetworkNodes[3].Node == Network.GetNodeByPinID(ID)) {          
          if(pin_value == &NetworkNodes[3].Node->inputs[3]){
           *pin_value = 0.0f;//(ballposition.x)*NetworkNodes[3].Node->ifann.output_scale;
          }else
          if (pin_value == &NetworkNodes[3].Node->inputs[4]) {
           *pin_value = 0.0f;//(ballposition.x)*NetworkNodes[3].Node->ifann.output_scale;
          }else
          if (pin_value == &NetworkNodes[3].Node->inputs[5]) {
           *pin_value = 0.0f;//(ballposition.x)*NetworkNodes[3].Node->ifann.output_scale;
          }
          //}else if(Select2_Node == Network.GetNodeByPinID(ID)){

          //select2_input_data.push_back(Select2_Node->inputs[0]);
          //select2_input_data.push_back(Select2_Node->inputs[1]);
          //select2_input_data.push_back(Select2_Node->inputs[2]);
          //select2_input_data.push_back(Select2_Node->inputs[3]);
          //select2_input_data.push_back(Select2_Node->inputs[4]);
          //select2_input_data.push_back(Select2_Node->inputs[5]);

          //if(Select2_Node->inputs[5]==1.0){
          // select2_output_data.push_back(1.0);
          //}
          //else{
          //select2_output_data.push_back(1.0);
          //}

          //select2_can_train = true;
          //{//Limit train size
          // if (select2_output_data.size() > select2_data_counter_limit) {
          //  select2_input_data.erase(select2_input_data.begin(), select2_input_data.begin() + select2_input_data.size() - select2_data_counter_limit * Select2_FANNPong.input_neurons);
          //  select2_output_data.erase(select2_output_data.begin(), select2_output_data.begin() + select2_output_data.size() - select2_data_counter_limit * Select2_FANNPong.output_neurons);
          // }

         }
        }
       }
       if(NetworkNodes[0].Node->IsRunning)
        fann_scale_input(NetworkNodes[0].Node->ifann.ann, NetworkNodes[0].Node->inputs);
       Network.Run(); 
       if(NetworkNodes[0].Node->IsRunning)
        fann_descale_output(NetworkNodes[0].Node->ifann.ann, NetworkNodes[0].Node->outputs);
       //get output values
       Network.NodeRegister.OutputPinRegister.output_pins.ResetIterator();
       while (0 <= Network.NodeRegister.OutputPinRegister.output_pins.GetNextIterator(ID, pin_value)) {
        if (true)
        {
         if(NetworkNodes[0].Node->IsRunning && NetworkNodes[0].Node == Network.GetNodeByPinID(ID)){
          if (pin_value) {
           fann_type val = *pin_value / NetworkNodes[0].Node->ifann.output_scale;
           AI_paddle_desired_position_x = -val;
           Last_AI_Paddle_Network_Active = 0;
          }
         }else if(NetworkNodes[1].Node->IsRunning && NetworkNodes[1].Node == Network.GetNodeByPinID(ID)){
          NetworkNodes[1].Node->IsRunning = false;         
         }else if (NetworkNodes[2].Node->IsRunning && NetworkNodes[2].Node == Network.GetNodeByPinID(ID)) {
          //get executed data and train with current
          NetworkNodes[2].Node->IsRunning = false;
          b2Vec2 position = game_body[4]->body->GetPosition();
          AI_paddle_desired_position_x = NetworkNodes[2].Node->outputs[0] / NetworkNodes[2].Node->ifann.output_scale;
          Last_AI_Paddle_Network_Active = 1;
         }else if (NetworkNodes[3].Node->IsRunning && NetworkNodes[3].Node == Network.GetNodeByPinID(ID)) {
          NetworkNodes[3].Node->IsRunning = false;

          //Inputs x start, arctan2(speed vec y, ), magnitude(speedvec), end posx, input 1, input 2, input 3
          if(select1_train_trigger){
           

           //if (select1_output_data.size() < select1_data_counter_limit) 
           {
            NetworkNodes[3].Node->inputs[3] = NetworkNodes[3].Node->inputs[4] = NetworkNodes[3].Node->inputs[5] = NetworkNodes[3].Node->inputs[6] = 0.0f;
            fann_type outputs[1];
            outputs[0] = ((ballposition.x)*NetworkNodes[3].Node->ifann.output_scale);
            NetworkNodes[3].Add_Data( NetworkNodes[3].Node->inputs, outputs);
            NetworkNodes[3].Node->AddFlag(TNodeStates::E_Training);




            
           }
           //else{
           // select1_train_trigger = false;
           //}
          }
          if(select1_train_trigger){
           select1_train_trigger = false;
           if(AutoPlayer){
            AI_paddle_desired_position_x = left * 1.3;
           }else{
            AI_paddle_desired_position_x = NetworkNodes[3].Node->outputs[0] / NetworkNodes[3].Node->ifann.output_scale;
            Last_AI_Paddle_Network_Active = 2;
           }
          }else{ ////////////////////              DOUBLED LINE
           AI_paddle_desired_position_x = NetworkNodes[3].Node->outputs[0] / NetworkNodes[3].Node->ifann.output_scale;
           Last_AI_Paddle_Network_Active = 2;
          }
         }
        }else{
         //b2Vec2 position = game_body[4]->body->GetPosition();
         //AI_paddle_desired_position_x = NetworkNodes[3].Node->outputs[0] / Select1_Node->ifann.output_scale;
        }
       }
       DisonnectNetwork01();
      }
      ////////////////////////////////////////////////////////////////////////////// NETWORK EXECUTION STOP
     }
    }
   }
  }
  //DEMO 4 - Simple Game - START

  return;
  //DEMO 4 - Simple Game - STOP
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
  IFAdapter.CalculateBox2DSizeFactor(30);

  int twidth, theight;
  GLuint texint;

  User_Data.CubeTexture = IFEUtilsLoadTexture::png_texture_load("testcube.png", &twidth, &theight);
  User_Data.Textures[2] = IFEUtilsLoadTexture::png_texture_load("ball.png", &twidth, &theight);

  FANN_TEST_initialized = false;
  anns_body = anns_learned_body = NULL;

  //TextRender.InitTextRender("Roboto-Thin.ttf", User_Data.state);
  TextRender.InitTextRender("RobotoMono-Regular.ttf", User_Data.state);
  TextRender.SetBackgroundColor(19, 187, 236, 255);
  TextRender.SetForegroundColor(255, 128, 0, 120);
  TextRender.SetCharSize_px(50, 80);

  prev_score = -10000;

  for (unsigned int cnt = 0; cnt < 10; cnt++) {
   TEST_GUI_Tex_Ary[cnt] = GL_INVALID_VALUE;
  }

  prev_score = -10000;


  return;
 }
}
void TEST_Cleanup(){
 //if (FANN_TEST_initialized) {
 // IFFANN::Save_Cascade_FANN(&LittleBrains, IFFANN::CnTrainedFannPostscript);
 //}

 //IFFANN::Save_Cascade_FANN(&NetworkNodes[0].Node->ifann, IFFANN::CnTrainedFannPostscript);

 //Network.NodeRegister.Unregister(NetworkNodes[0].Node);
 TESTFN_SaveAllTrainData();
 User_Data.TrainInProgress = false;

 last_touched_object = NULL;
 DEMO4_initialized = false;

 glDeleteTextures(10, TEST_GUI_Tex_Ary);
 for( unsigned int cnt = 0; cnt < 10; cnt++){
  TEST_GUI_Tex_Ary[cnt] = GL_INVALID_VALUE;
 }
}








