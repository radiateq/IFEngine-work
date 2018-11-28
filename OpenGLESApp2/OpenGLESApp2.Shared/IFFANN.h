#pragma once


#include "fann.h"
#include "floatfann.h"

namespace IFFANN{


 typedef struct {
  struct fann_train_data *train_data, *test_data;
  float desired_error;
  unsigned int max_neurons;
  unsigned int neurons_between_reports;
  unsigned int bit_fail_train, bit_fail_test;
  float mse_train, mse_test;
 }IFS_Cascade_FANN_Train_Struct;
 

 typedef struct {
  struct fann *ann;
  IFS_Cascade_FANN_Train_Struct *ann_train;
 } IFS_Cascade_FANN;

 IFS_Cascade_FANN *Init_Cascade_FANN(IFS_Cascade_FANN &_pInOut) {
  _pInOut.ann = NULL;
  _pInOut.ann_train = NULL;
  return &_pInOut;
 }

 IFS_Cascade_FANN *Create_Cascade_FANN(IFS_Cascade_FANN *_pInOut, unsigned int _inputs_number, unsigned int _outputs_number) {
  if(_pInOut->ann) fann_destroy(_pInOut->ann), _pInOut->ann = NULL;
  if(_pInOut->ann_train) free(_pInOut->ann_train), _pInOut->ann_train = NULL;
  _pInOut->ann = fann_create_shortcut(2, _inputs_number, _outputs_number);
 }

 void Train_Cascade_FANN_Callback(unsigned int num_data, unsigned int num_input, unsigned int num_output, fann_type *input, fann_type *output){
 }

 void Train_Cascade_FANN(IFS_Cascade_FANN *cfann) {


  cfann->ann_train->train_data = fann_create_train_from_callback(1, cfann->ann->num_input, cfann->ann->num_output, Train_Cascade_FANN_Callback);

 //fann_scale_input

  //fann_create_train_from_callback
  // FANN_EXTERNAL struct fann_train_data * FANN_API fann_create_train_from_callback(
  //  unsigned 	int 	num_data,
  //  unsigned 	int 	num_input,
  //  unsigned 	int 	num_output,
  //  void 	(FANN_API *user_function)(unsigned int, unsigned int, unsigned int, fann_type *, fann_type *)
  // )
   //Creates the training data struct from a user supplied function.As the training data are numerable(data 1, data 2...), the user must write a function that receives the number of the training data set(input, output) and returns the set.fann_create_train_from_callback will call the user supplied function ‘num_data’ times, one input - output pair each time.Each time the user supplied function is called, the time of the call will be passed as the ‘num’ parameter and the user supplied function must write the input and output to the corresponding parameters.

   //Parameters
   //num_data	The number of training data
   //num_input	The number of inputs per training data
   //num_output	The number of ouputs per training data
   //user_function	The user supplied function
   //Parameters for the user function
   //num	The number of the training data set
   //num_input	The number of inputs per training data
   //num_output	The number of ouputs per training data
   //input	The set of inputs
   //output	The set of desired outputs
   //See also
   //fann_read_train_from_file, fann_train_on_data, fann_destroy_train, fann_save_train

  




























  //RQNDKUtils::Make_internalDataPath();
  unsigned int i = 0;
  fann_type *output;
  fann_type steepness;
  int multi = 0;
  enum fann_activationfunc_enum activation;
  enum fann_train_enum training_algorithm = FANN_TRAIN_RPROP;

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