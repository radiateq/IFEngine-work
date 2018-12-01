#include "IFFANN.h"

namespace IFFANN {

 const char CnCleanFannPostscript[] = { 'f','a','n','n','c','l','e','a','n','\0' };
 const char CnTrainedFannPostscript[] = { 'f','a','n','n','t','r','a','i','n', '\0' };
 const char CnFinalFannPostscript[] = { 'f','a','n','n','\0' };
 const char CnFannIFANNDataPostscript[] = { 'd', 'a', 't', '\0' };

 IFS_Cascade_FANN *Init_Cascade_FANN(IFS_Cascade_FANN *_pInOut) {
  _pInOut->ann = NULL;
  _pInOut->ann_train = NULL;
  _pInOut->unique_name = NULL;
  return _pInOut;
 }

 void Cleanup_Cascade_FANN(IFS_Cascade_FANN  * const ifann) {
  if (ifann->ann) fann_destroy(ifann->ann), ifann->ann = NULL;
  if (ifann->ann_train) free(ifann->ann_train), ifann->ann_train = NULL;
  if (ifann->unique_name) free(ifann->unique_name), ifann->unique_name = NULL;
 }

 //unique_name parameter is used for file operations to generate multiple different file names
 IFS_Cascade_FANN *Create_Cascade_FANN(IFS_Cascade_FANN *ifann, unsigned int inputs_number, unsigned int outputs_number, char *unique_name) {
  Cleanup_Cascade_FANN(ifann);
  ifann->ann = fann_create_shortcut(2, inputs_number, outputs_number);
  ifann->unique_name = (char*)malloc(strlen(unique_name) * sizeof(ifann->unique_name[0]) + 1);
  strcpy(ifann->unique_name, unique_name);
  return ifann;
 }

 IFS_Cascade_FANN *Setup_Train_Cascade_FANN(IFS_Cascade_FANN *ifann, unsigned int max_neurons, unsigned int neurons_between_reports, fann_type desired_error, fann_type input_scale, fann_type output_scale) {
  if (ifann->ann_train) free(ifann->ann_train), ifann->ann_train = NULL;
  ifann->ann_train = (IFS_Cascade_FANN_Train_Struct*)malloc(sizeof(IFS_Cascade_FANN_Train_Struct));
  //ifann->ann_train->num_data = 0;
  ifann->ann_train->max_neurons = max_neurons;
  ifann->ann_train->neurons_between_reports = neurons_between_reports;
  ifann->ann_train->desired_error = desired_error;
  ifann->input_scale = input_scale;
  ifann->output_scale = output_scale;
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
  if (ifann->ann->training_algorithm == FANN_TRAIN_QUICKPROP) {
   fann_set_learning_rate(ifann->ann, 0.35f);
   fann_randomize_weights(ifann->ann, -2.0f, 2.0f);
  }
  fann_set_bit_fail_limit(ifann->ann, (fann_type)0.9);
  fann_set_train_stop_function(ifann->ann, FANN_STOPFUNC_BIT);
  //fann_print_parameters(ifann->ann);
  Save_Cascade_FANN(ifann, CnCleanFannPostscript);
  Save_Cascade_FANN(ifann, CnTrainedFannPostscript);


  return ifann;
 }

 IFS_Cascade_FANN *Finish_Train_Cascade_FANN(IFS_Cascade_FANN *ifann) {
  free(ifann->ann_train);
  ifann->ann_train = NULL;
  Save_Cascade_FANN(ifann, CnFinalFannPostscript);
 }

 //Loads empty ann and train structure initialization 
 bool Load_Cascade_FANN(IFS_Cascade_FANN * const ifann, char const * const unique_name, char const * const state_name) {

  Cleanup_Cascade_FANN(ifann);

  char char_buffer[1024];
  char path_name[1024];
  strcpy(char_buffer, unique_name);
  size_t name_len = strlen(unique_name);
  strcpy(&char_buffer[name_len], state_name);
  FILE *IFANNDataFile = fopen(RQNDKUtils::Make_storageDataPath(path_name, 1024, char_buffer), "r");
  if (IFANNDataFile == NULL) {
   return false;
  }
  fclose(IFANNDataFile);
  ifann->ann = fann_create_from_file(RQNDKUtils::Make_storageDataPath(path_name, 1024, char_buffer));
  //Save additional nework data
  strcpy(&char_buffer[strlen(char_buffer)], CnFannIFANNDataPostscript);
  IFANNDataFile = fopen(RQNDKUtils::Make_storageDataPath(path_name, 1024, char_buffer), "r");
  if (IFANNDataFile == NULL) {
   return false;
  }
  //Check if we have train data saved
  unsigned int marker;
  fread(&marker, sizeof(marker), 1, IFANNDataFile);
  if (marker) {
   fread(&name_len, sizeof(name_len), 1, IFANNDataFile);
   ifann->unique_name = (char*)malloc(sizeof(ifann->unique_name[0]) * (name_len + 1));
   fread(ifann->unique_name, sizeof(ifann->unique_name[0]), name_len, IFANNDataFile);
   ifann->unique_name[name_len] = '\0';
   ifann->ann_train = (IFS_Cascade_FANN_Train_Struct*)malloc(sizeof(IFS_Cascade_FANN_Train_Struct));
   //fread(&ifann->ann_train->num_data, sizeof(ifann->ann_train->num_data), 1, IFANNDataFile);
   fread(&ifann->ann_train->max_neurons, sizeof(ifann->ann_train->max_neurons), 1, IFANNDataFile);
   fread(&ifann->ann_train->neurons_between_reports, sizeof(ifann->ann_train->neurons_between_reports), 1, IFANNDataFile);
   fread(&ifann->ann_train->desired_error, sizeof(ifann->ann_train->desired_error), 1, IFANNDataFile);
   fread(&ifann->input_scale, sizeof(ifann->input_scale), 1, IFANNDataFile);
   fread(&ifann->output_scale, sizeof(ifann->input_scale), 1, IFANNDataFile);
  }
  fclose(IFANNDataFile);

  return true;
 }
 bool const Save_Cascade_FANN(IFS_Cascade_FANN const * const ifann, char const * const state_name) {
  char char_buffer[1024];
  char path_name[1024];
  strcpy(char_buffer, ifann->unique_name);
  size_t name_len = strlen(ifann->unique_name);
  strcpy(&char_buffer[name_len], state_name);
  if( -1 == fann_save(ifann->ann, RQNDKUtils::Make_storageDataPath(path_name, 1024, char_buffer)) ){
   return false;
  }
  //Save additional network data
  strcpy(&char_buffer[strlen(char_buffer)], CnFannIFANNDataPostscript);
  FILE *IFANNDataFile = fopen(RQNDKUtils::Make_storageDataPath(path_name, 1024, char_buffer), "w+");
  if( IFANNDataFile == NULL){
   return false;
  }
  if (ifann->ann_train) {
   unsigned int marker = 1;
   fwrite(&marker, sizeof(marker), 1, IFANNDataFile);
   fwrite(&name_len, sizeof(name_len), 1, IFANNDataFile);
   fwrite(ifann->unique_name, sizeof(ifann->unique_name[0]), name_len, IFANNDataFile);
   //fwrite(&ifann->ann_train->num_data, sizeof(ifann->ann_train->num_data), 1, IFANNDataFile);
   fwrite(&ifann->ann_train->max_neurons, sizeof(ifann->ann_train->max_neurons), 1, IFANNDataFile);
   fwrite(&ifann->ann_train->neurons_between_reports, sizeof(ifann->ann_train->neurons_between_reports), 1, IFANNDataFile);
   fwrite(&ifann->ann_train->desired_error, sizeof(ifann->ann_train->desired_error), 1, IFANNDataFile);
   fwrite(&ifann->input_scale, sizeof(ifann->input_scale), 1, IFANNDataFile);
   fwrite(&ifann->output_scale, sizeof(ifann->input_scale), 1, IFANNDataFile);
  }
  else {
   unsigned int marker = 0;
   fwrite(&marker, sizeof(marker), 1, IFANNDataFile);
  }
  fclose(IFANNDataFile);
  return true;
 }
 bool const Check_Save_Cascade_FANN(char const * const unique_name, char const * const state_name){
  char char_buffer[1024];
  char path_name[1024];
  strcpy(char_buffer, unique_name);
  size_t name_len = strlen(unique_name);
  strcpy(&char_buffer[name_len], state_name);
  FILE *IFANNDataFile = fopen(RQNDKUtils::Make_storageDataPath(path_name, 1024, char_buffer), "r");
  if (NULL == IFANNDataFile ){
   return false;
  }
  fclose(IFANNDataFile);  
  strcpy(&char_buffer[strlen(char_buffer)], CnFannIFANNDataPostscript);
  IFANNDataFile = fopen(RQNDKUtils::Make_storageDataPath(path_name, 1024, char_buffer), "r");
  if ( NULL == IFANNDataFile ) {
   return false;
  }
  fclose(IFANNDataFile);
  return true;
 }
 void Train_Cascade_FANN(IFS_Cascade_FANN *ifann, TTrain_Cascade_FANN_Callback *train_callback, unsigned int num_data ) {

  //ifann->ann_train->num_data = num_data;
  ifann->ann_train->train_data = fann_create_train_from_callback(num_data, ifann->ann->num_input, ifann->ann->num_output, train_callback);
  fann_scale_train_data(ifann->ann_train->train_data, -1, 1);
  fann_cascadetrain_on_data(ifann->ann, ifann->ann_train->train_data, ifann->ann_train->max_neurons, ifann->ann_train->neurons_between_reports, ifann->ann_train->desired_error);



  //float mse_train = fann_test_data(ifann->ann, ifann->ann_train->train_data);
  //int bit_fail_train = fann_get_bit_fail(ifann->ann);


  Save_Cascade_FANN(ifann, CnTrainedFannPostscript);

 }

 fann_type *Run_Cascade_FANN(IFS_Cascade_FANN *ifann, fann_type *inputs) {
  return fann_run(ifann->ann, inputs);
 }

};