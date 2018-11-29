#pragma once


//#include "fann.h"
#include "floatfann.h"
#include "../OpenGLESApp2.Android.NativeActivity/RQ_NDK_Utils.h"

namespace IFFANN{

 extern const char CnCleanFannPostscript[];
 extern const char CnTrainedFannPostscript[];
 extern const char CnFinalFannPostscript[];
 extern const char CnFannIFANNDataPostscript[];

 typedef struct {
  struct fann_train_data *train_data, *test_data;
  //unsigned int num_data;
  fann_type desired_error;
  unsigned int max_neurons;
  unsigned int neurons_between_reports;
 }IFS_Cascade_FANN_Train_Struct;
 

 typedef struct {
  struct fann *ann;
  IFS_Cascade_FANN_Train_Struct *ann_train;
  char *unique_name;
  //Scale factor should be such that for intended range all inputs should be scaled to range -1 to 1
  fann_type input_scale;
  //Scale factor should be such that for intended range all outputs should be scaled to range -1 to 1
  fann_type output_scale;
 } IFS_Cascade_FANN;

 typedef void TTrain_Cascade_FANN_Callback(unsigned int, unsigned int, unsigned int, fann_type *, fann_type *);
 //void Train_Cascade_FANN_Callback(unsigned int num_data, unsigned int num_input, unsigned int num_output, fann_type *input, fann_type *output);


 IFS_Cascade_FANN *Init_Cascade_FANN(IFS_Cascade_FANN *_pInOut);

 void Cleanup_Cascade_FANN(IFS_Cascade_FANN  * const ifann);

 //unique_name parameter is used for file operations to generate multiple different file names
 IFS_Cascade_FANN *Create_Cascade_FANN(IFS_Cascade_FANN *ifann, unsigned int inputs_number, unsigned int outputs_number, char *unique_name );
 
 IFS_Cascade_FANN *Setup_Train_Cascade_FANN(IFS_Cascade_FANN *ifann, unsigned int max_neurons, unsigned int neurons_between_reports, fann_type desired_error, fann_type input_scale, fann_type output_scale );

 IFS_Cascade_FANN *Finish_Train_Cascade_FANN(IFS_Cascade_FANN *ifann);

 //Loads empty ann and train structure initialization 
 bool Load_Cascade_FANN(IFS_Cascade_FANN * const ifann, char const * const unique_name, char const * const state_name);

 bool const Save_Cascade_FANN(IFS_Cascade_FANN const * const ifann, char const * const state_name);

//Returns true if save exists
 bool const Check_Save_Cascade_FANN(char const * const unique_name, char const * const state_name);

 void Train_Cascade_FANN(IFS_Cascade_FANN *ifann, TTrain_Cascade_FANN_Callback *train_callback, unsigned int num_data);

 fann_type *Run_Cascade_FANN(IFS_Cascade_FANN *ifann, fann_type *inputs);
};