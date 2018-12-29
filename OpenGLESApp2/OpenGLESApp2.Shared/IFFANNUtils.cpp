#include "IFFANNUtils.h"


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


