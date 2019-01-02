#pragma once

#include "IFFANNEngine.h"

class CNetworkNode;


typedef IFFANNEngine::CNode::ENodeStates TNodeStates;
typedef std::vector<fann_type> TFannVector;
extern TFannVector *train_input_vector, *train_output_vector;
extern struct fann *train_ann;
void Setup_Train_Cascade_FANN_Callback(fann *_train_ann, TFannVector *_train_input_vector, TFannVector *_train_output_vector);
void Train_Cascade_FANN_Callback(unsigned int num_data, unsigned int num_input, unsigned int num_output, fann_type *input, fann_type *output);

struct SFANNOptions {
 int max_neurons = 30, neurons_between_reports = 0, input_neurons = 4, output_neurons = 1;
 float desired_error = 0.05, input_scale = 0.1, output_scale = 10.00;
};

class CNetworkNode {
public:
 ~CNetworkNode() {
  Free();
 }
 void Free() {
  if (Node) {
   Node->NodeRegister->Unregister(Node);
   delete Node;
  }
 }
 SFANNOptions FANNOptions;
 IFFANNEngine::CNode *Node = NULL;
 TFannVector node_input_data, node_output_data;
 bool node_train_trigger = false, node_can_train = false;
 unsigned int node_data_counter = 0;
 char node_unique_name[1024];
 unsigned int node_train_samples = 50, node_train_pos = 0, node_last_save_index = 0, node_new_samples = 0;
 fann_type node_train_error = 1, epoch_train_desired_eror = 1e-6;
 unsigned int node_data_counter_limit = 1000;

 size_t NumTrainData() {
  return node_output_data.size() / FANNOptions.output_neurons;
 }

 void CheckNodeTrainDataState() {
  if (node_output_data.size() == 0) {
   Node->AddFlag(TNodeStates::E_NoData);
   Node->ClearFlag(TNodeStates::E_FullData);
  }
  else {
   Node->ClearFlag(TNodeStates::E_NoData);
   {//Limit train size    
    node_new_samples++;
    if ((node_output_data.size() / FANNOptions.output_neurons) > node_data_counter_limit) {
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

 void Load_Node(IFFANNEngine::CNetwork &Network, char const * const unique_name, bool is_running = true) {
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
  if (!(Node->NodeStates&IFFANNEngine::CNode::ENodeStates::E_ContTraining))
   return false;
  //  if (!Node->GetFlag(TNodeStates::E_FullData))
  if (node_train_samples == 0)
   return false;
  //if (false)
  {/////////////////////////////////   TRAIN node
   /////////////////////////////////////NC02START
   if ((node_train_error > epoch_train_desired_eror) && ((node_train_error != std::numeric_limits<fann_type>::max())))
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
     unsigned int subset_node_train_samples = node_train_samples;
     unsigned int subset_start = Node->ifann.ann_train->train_data->num_data - node_train_samples - node_train_pos;
     if((node_train_samples - node_train_pos)> Node->ifann.ann_train->train_data->num_data){
      subset_start = 0;
      subset_node_train_samples = Node->ifann.ann_train->train_data->num_data;      
     }
     train_subset = fann_subset_train_data(Node->ifann.ann_train->train_data, subset_start, subset_node_train_samples);
     while ((end_time > start_time) && (subset_node_train_samples <= Node->ifann.ann_train->train_data->num_data)) {
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
   for (unsigned int cnt = 0; cnt < Node->ifann.ann_train->train_data->num_data; cnt++) {
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
  if (!Node->GetFlag(TNodeStates::E_Training))
   return false;
  if (!Node->GetFlag(TNodeStates::E_FullData))
   return false;
  if (Node->GetFlag(TNodeStates::E_ContTraining))
   return false;
  char unique_name[BUFSIZ + 1];
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
 bool Save_Fann() {
  return IFFANN::Save_Cascade_FANN(&Node->ifann, IFFANN::CnFinalFannPostscript);
 }
};

