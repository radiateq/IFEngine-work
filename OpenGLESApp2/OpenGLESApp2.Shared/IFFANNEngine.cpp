#include "IFFANNEngine.h"

namespace IFFANNEngine {
 IFGeneralUtils::CFileSystem IFFileSystem;


 bool CNode::LoadCore(char const * const core_name) {
  UnloadCore();  
  if( !IFFANN::Load_Cascade_FANN(&ifann, core_name, IFFANN::CnFinalFannPostscript) ) return false;
  core_loaded = true;
  outputs = NULL;
  free(inputs);
  inputs = (fann_type*)malloc(sizeof(fann_type)*ifann.ann->num_input);
  CPin *temp_pin;
  for (unsigned int cnt = 0; cnt < ifann.ann->num_input; cnt++) {
   temp_pin = new CPin;
   temp_pin->data = &inputs[cnt];
   temp_pin->fann_index = cnt;
   NodeRegister->PinToNode.Register(temp_pin, ID);
   NodeRegister->InputPinRegister.input_pins.pAdd(temp_pin->ID, &inputs[cnt]);
   input_pins.pAdd(cnt, temp_pin);      
  }
  for (unsigned int cnt = 0; cnt < ifann.ann->num_output; cnt++) {
   temp_pin = new CPin;
   temp_pin->data = NULL;//data is set as part of execution
   temp_pin->fann_index = cnt;
   NodeRegister->PinToNode.Register(temp_pin, ID);
   NodeRegister->OutputPinRegister.output_pins.pAdd(temp_pin->ID, outputs);
   output_pins.pAdd(cnt, temp_pin);
  }  
  return true;
 }

 bool CNode::UnloadCore() {
  
  if(!core_loaded) return false;

  if(ifann.ann==NULL) return false;

  core_loaded = false;

  CPin *temp_pin;

  for (unsigned int cnt = 0; cnt < ifann.ann->num_input; cnt++) {
   input_pins.Get(cnt, temp_pin);
   NodeRegister->InputPinRegister.input_pins.Remove(temp_pin->ID);

   unsigned int pin_out_ID;
   for (temp_pin->connected_pins.Iter_Set = temp_pin->connected_pins.Set.begin(); temp_pin->connected_pins.Iter_Set != temp_pin->connected_pins.Set.end(); temp_pin->connected_pins.Iter_Set++) {
    pin_out_ID = *temp_pin->connected_pins.Iter_Set;
    NodeRegister->Network->DisconnectPins(pin_out_ID, temp_pin->ID);
   }
   temp_pin->connected_pins.Set.clear();

   input_pins.Remove(cnt);
   NodeRegister->PinToNode.Unregister(temp_pin);
   delete temp_pin;
  }
  for (unsigned int cnt = 0; cnt < ifann.ann->num_output; cnt++) {
   output_pins.Get(cnt, temp_pin);
   NodeRegister->OutputPinRegister.output_pins.Remove(temp_pin->ID);

   unsigned int pin_in_ID;
   for (temp_pin->connected_pins.Iter_Set = temp_pin->connected_pins.Set.begin(); temp_pin->connected_pins.Iter_Set != temp_pin->connected_pins.Set.end(); temp_pin->connected_pins.Iter_Set++) {
    pin_in_ID = *temp_pin->connected_pins.Iter_Set;
    NodeRegister->Network->DisconnectPins(temp_pin->ID, pin_in_ID);
   }
   temp_pin->connected_pins.Set.clear();

   output_pins.Remove(cnt);
   temp_pin->connected_pins.Set.clear();
   NodeRegister->PinToNode.Unregister(temp_pin);
   delete temp_pin;
  }

  outputs = NULL;
  free(inputs);
  inputs = NULL;

  Cleanup_Cascade_FANN(&ifann);

  return true;
 }






























 CNetwork Network;


 void TestNetwork() {
  //Create Node
  CNode Node1;
  CNode Node2;

  Network.NodeRegister.Register(&Node1);
  Node1.LoadCore("forces01");

  Network.NodeRegister.Register(&Node2);
  Node2.LoadCore("forces01");

  Node1.inputs[0] = 0.1;
  //OR
  *Node1.GetInPinByIndex(1)->data = 0.2;
  *Node1.GetInPinByIndex(2)->data = 0.3;
  

  Network.ConnectPins(Node1.GetOutPinByIndex(0)->ID, Node2.GetInPinByIndex(1)->ID);
  //OR
  Network.ConnectPins(Node1.GetOutPinByIndex(1), Node2.GetInPinByIndex(2));

  unsigned int ID;
  fann_type *pin_value;
  //Set all inputs to some value
  Network.NodeRegister.InputPinRegister.input_pins.ResetIterator();
  while(0<=Network.NodeRegister.InputPinRegister.input_pins.GetNextIterator(ID, pin_value)){
   *pin_value = 0.6;
  }
  Network.Run();
  //get output values
  Network.NodeRegister.OutputPinRegister.output_pins.ResetIterator();
  while (0 <= Network.NodeRegister.OutputPinRegister.output_pins.GetNextIterator(ID, pin_value)) {
   if(pin_value){
    fann_type val = *pin_value;
    val = val;
   }
  }


  Network.DisconnectPins(Node1.GetOutPinByIndex(1), Node2.GetInPinByIndex(2));
  Network.DisconnectPins(Node1.GetOutPinByIndex(0), Node2.GetInPinByIndex(1));


  Network.NodeRegister.Unregister(&Node1);
  Network.NodeRegister.Unregister(&Node2);

 }


}

