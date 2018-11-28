#pragma once


#include "fann.h"
#include "floatfann.h"

namespace IFFANN{

 const char CnCleanFannPostscript[] = {'_','f','a','n','n','_','c','l','e','a','n', '.', 'n', 'e', 't', '\0'};
 const char CnTrainedFannPostscript[] = {'_','f','a','n','n','_','t','r','a','i','n', '.', 'n', 'e', 't', '\0'};
 const char CnTrainedFannPostscript[] = {'_','f','a','n','n','.', 'n', 'e', 't', '\0'};
 const char CnFannIFANNDataPostscript[] = {'_','f','a','n','n','.', 'd', 'a', 't', '\0'};

 typedef struct {
  struct fann_train_data *train_data, *test_data;
  float desired_error;
  unsigned int max_neurons;
  unsigned int neurons_between_reports;
  //Scale factor should be such that for intended range all inputs should be scaled to range -1 to 1
  float input_scale;
  //Scale factor should be such that for intended range all outputs should be scaled to range -1 to 1
  float output_scale;
 }IFS_Cascade_FANN_Train_Struct;
 

 typedef struct {
  struct fann *ann;
  IFS_Cascade_FANN_Train_Struct *ann_train;
  char *unique_name;
 } IFS_Cascade_FANN;

 IFS_Cascade_FANN *Init_Cascade_FANN(IFS_Cascade_FANN &_pInOut) {
  _pInOut.ann = NULL;
  _pInOut.ann_train = NULL;
  _pInOut.unique_name = NULL;
  return &_pInOut;
 }

 void Cleanup_Cascade_FANN(IFS_Cascade_FANN *ifann){
  if(ifann->ann) fann_destroy(ifann->ann), ifann->ann = NULL;
  if(ifann->ann_train) free(ifann->ann_train), ifann->ann_train = NULL;
  if(ifann->unique_name) free(ifann->unique_name), ifann->unique_name = NULL;  
 }

 //unique_name parameter is used for file operations to generate multiple different file names
 IFS_Cascade_FANN *Create_Cascade_FANN(IFS_Cascade_FANN *ifann, unsigned int inputs_number, unsigned int outputs_number, char *unique_name ) {
  Cleanup_Cascade_FANN(ifann);
  ifann->ann = fann_create_shortcut(2, inputs_number, outputs_number);
  ifann->unique_name = (char*)malloc(strlen(unique_name)*sizeof(ifann->unique_name[0]) + 1);
  strcpy( ifann->unique_name, unique_name);
  return ifann;
 }
 
 IFS_Cascade_FANN *Setup_Train_Cascade_FANN(IFS_Cascade_FANN *ifann, unsigned int max_neurons, unsigned int neurons_between_reports, float desired_error, float input_scale, float output_scale ){
  if(ifann->ann_train) free(ifann->ann_train), ifann->ann_train = NULL;
  ifann->ann_train = (IFS_Cascade_FANN_Train_Struct*)malloc(sizeof(IFS_Cascade_FANN_Train_Struct));
  ifann->ann_train->max_neurons = max_neurons;
  ifann->ann_train->neurons_between_reports = neurons_between_reports;
  ifann->ann_train->desired_error = desired_error;
  ifann->ann_train->input_scale = input_scale;
  ifann->ann_train->output_scale = output_scale;
  ifann->ann_train->train_data = ifann->ann_train->test_data = NULL;

  //Setup network
  fann_set_training_algorithm(ifann->ann, FANN_TRAIN_RPROP);//Option is FANN_TRAIN_QUICKPROP
  fann_set_activation_function_hidden(ifann->ann, FANN_SIGMOID_SYMMETRIC);
  fann_set_activation_function_output(ifann->ann, FANN_LINEAR);
  fann_set_train_error_function(ifann->ann, FANN_ERRORFUNC_LINEAR);    
  fann_type steepness;
  //steepness = 0.5;
  steepness = 1;
  fann_set_cascade_activation_steepnesses(ifann->ann, &steepness, 1);
  enum fann_activationfunc_enum activation;
  //activation = FANN_SIN_SYMMETRIC;
  activation = FANN_SIGMOID_SYMMETRIC;
  fann_set_cascade_activation_functions(ifann->ann, &activation, 1);
  fann_set_cascade_num_candidate_groups(ifann->ann, 8);
  //For multi layer neworks STOP 
  if (ifann->ann->training_algorithm == FANN_TRAIN_QUICKPROP){
   fann_set_learning_rate(ifann->ann, 0.35f);
   fann_randomize_weights(ifann->ann, -2.0f, 2.0f);
  }                              
  fann_set_bit_fail_limit(ifann->ann, (fann_type)0.9);
  fann_set_train_stop_function(ifann->ann, FANN_STOPFUNC_BIT);
  //fann_print_parameters(ifann->ann);
  //Save network
  char char_buffer[1024];
  char path_name[1024];
  strcpy(char_buffer, ifann->unique_name);
  strcpy(&char_buffer[strlen(ifann->unique_name)], CnCleanFannPostscript);
  fann_save(ifann->ann, RQNDKUtils::Make_internalDataPath(path_name,1024,char_buffer));  
  //Save additional nework data
  strcpy( &char_buffer[strlen(ifann->unique_name)], CnFannIFANNDataPostscript );
  FILE *IFANNDataFile = fopen(RQNDKUtils::Make_internalDataPath(path_name,1024,char_buffer),"w+");
  size_t name_len = strlen(ifann->unique_name);
  fwrite(&name_len, sizeof(name_len), 1, IFANNDataFile );
  fwrite(ifann->unique_name, sizeof(ifann->unique_name[0]), name_len, IFANNDataFile);
  fwrite(&ifann->ann_train->max_neurons, sizeof(ifann->ann_train->max_neurons), 1, IFANNDataFile );
  fwrite(&ifann->ann_train->neurons_between_reports, sizeof(ifann->ann_train->neurons_between_reports), 1, IFANNDataFile );
  fwrite(&ifann->ann_train->desired_error, sizeof(ifann->ann_train->desired_error), 1, IFANNDataFile );
  fwrite(&ifann->ann_train->input_scale, sizeof(ifann->ann_train->input_scale), 1, IFANNDataFile );
  fwrite(&ifann->ann_train->output_scale, sizeof(ifann->ann_train->input_scale), 1, IFANNDataFile );
  fclose(IFANNDataFile);

  return ifann;
 }

 //Loads empty ann and train structure initialization 
 bool Load_Cascade_FANN_Clean(IFS_Cascade_FANN *ifann, char *unique_name){

  Cleanup_Cascade_FANN(ifann);

  char char_buffer[1024];
  char path_name[1024];
  strcpy(char_buffer, ifann->unique_name);
  strcpy(&char_buffer[strlen(ifann->unique_name)], CnCleanFannPostscript);
  FILE *IFANNDataFile = fopen(RQNDKUtils::Make_internalDataPath(path_name,1024,char_buffer),"r");
  if(IFANNDataFile == NULL){
   return false;
  }
  fclose(IFANNDataFile);
  ifann->ann = fann_create_from_file(RQNDKUtils::Make_internalDataPath(path_name,1024,char_buffer));  
  //Save additional nework data
  strcpy( &char_buffer[strlen(ifann->unique_name)], CnFannIFANNDataPostscript );
  IFANNDataFile = fopen(RQNDKUtils::Make_internalDataPath(path_name,1024,char_buffer),"r");
  if(IFANNDataFile == NULL){
   return false;
  }
  size_t name_len = strlen(ifann->unique_name);
  fread(&name_len, sizeof(name_len), 1, IFANNDataFile );
  ifann->unique_name = (char*)malloc(sizeof(ifann->unique_name[0]) * (name_len + 1));
  fread(ifann->unique_name, sizeof(ifann->unique_name[0]), name_len, IFANNDataFile);
  ifann->unique_name[name_len] = '\0';
  fread(&ifann->ann_train->max_neurons, sizeof(ifann->ann_train->max_neurons), 1, IFANNDataFile );
  fread(&ifann->ann_train->neurons_between_reports, sizeof(ifann->ann_train->neurons_between_reports), 1, IFANNDataFile );
  fread(&ifann->ann_train->desired_error, sizeof(ifann->ann_train->desired_error), 1, IFANNDataFile );
  fread(&ifann->ann_train->input_scale, sizeof(ifann->ann_train->input_scale), 1, IFANNDataFile );
  fread(&ifann->ann_train->output_scale, sizeof(ifann->ann_train->input_scale), 1, IFANNDataFile );
  fclose(IFANNDataFile);

  return true;
 }


 void Train_Cascade_FANN_Callback(unsigned int num_data, unsigned int num_input, unsigned int num_output, fann_type *input, fann_type *output){
    
 }
 void Train_Cascade_FANN(IFS_Cascade_FANN *ifann, char *ifann_unique_name) {

  Load_Cascade_FANN_Clean

  ifann->ann_train->train_data = fann_create_train_from_callback(1, ifann->ann->num_input, ifann->ann->num_output, Train_Cascade_FANN_Callback);
  fann_cascadetrain_on_data(ifann->ann, ifann->ann_train->train_data, ifann->ann_train->max_neurons, ifann->ann_train->neurons_between_reports, ifann->ann_train->desired_error );
  




























  //RQNDKUtils::Make_internalDataPath();
  unsigned int i = 0;
  fann_type *output;
  fann_type steepness;
  int multi = 0;
  enum fann_activationfunc_enum activation;
  enum fann_train_enum training_algorithm = FANN_TRAIN_RPROP;
  unsigned int bit_fail_train, bit_fail_test;
  float mse_train, mse_test;

  printf("Reading data.\n");

  train_data = fann_read_train_from_file("../datasets/parity8.train");
  test_data = fann_read_train_from_file("../datasets/parity8.test");

  fann_scale_train_data(train_data, -1, 1);
  fann_scale_train_data(test_data, -1, 1);

  printf("Creating network.\n");

  ann = fann_create_shortcut(2, fann_num_input_train_data(train_data), fann_num_output_train_data(train_data));

  fann_set_training_algorithm(ann, training_algorithm);
  fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
  fann_set_activation_function_output(ann, FANN_LINEAR);
  fann_set_train_error_function(ann, FANN_ERRORFUNC_LINEAR);

  if (!multi)
  {
   /*steepness = 0.5;*/
   steepness = 1;
   fann_set_cascade_activation_steepnesses(ann, &steepness, 1);
   /*activation = FANN_SIN_SYMMETRIC;*/
   activation = FANN_SIGMOID_SYMMETRIC;

   fann_set_cascade_activation_functions(ann, &activation, 1);
   fann_set_cascade_num_candidate_groups(ann, 8);
  }

  if (training_algorithm == FANN_TRAIN_QUICKPROP)
  {
   fann_set_learning_rate(ann, 0.35f);
   fann_randomize_weights(ann, -2.0f, 2.0f);
  }

  fann_set_bit_fail_limit(ann, (fann_type)0.9);
  fann_set_train_stop_function(ann, FANN_STOPFUNC_BIT);
  fann_print_parameters(ann);

  fann_save(ann, "cascade_train2.net");

  printf("Training network.\n");

  fann_cascadetrain_on_data(ann, train_data, max_neurons, neurons_between_reports, desired_error);

  fann_print_connections(ann);

  mse_train = fann_test_data(ann, train_data);
  bit_fail_train = fann_get_bit_fail(ann);
  mse_test = fann_test_data(ann, test_data);
  bit_fail_test = fann_get_bit_fail(ann);

  printf("\nTrain error: %f, Train bit-fail: %d, Test error: %f, Test bit-fail: %d\n\n",
   mse_train, bit_fail_train, mse_test, bit_fail_test);

  for (i = 0; i < train_data->num_data; i++)
  {
   output = fann_run(ann, train_data->input[i]);
   if ((train_data->output[i][0] >= 0 && output[0] <= 0) ||
    (train_data->output[i][0] <= 0 && output[0] >= 0))
   {
    printf("ERROR: %f does not match %f\n", train_data->output[i][0], output[0]);
   }
  }

  printf("Saving network.\n");

  fann_save(ann, "cascade_train.net");

  printf("Cleaning up.\n");
  fann_destroy_train(train_data);
  fann_destroy_train(test_data);
  fann_destroy(ann);
 }


};