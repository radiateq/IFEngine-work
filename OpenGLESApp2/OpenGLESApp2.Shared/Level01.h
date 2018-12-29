#pragma once

#include "MainProgram.h"

namespace Level01{

 struct SFANNPong {
  int max_neurons = 30, neurons_between_reports = 0, input_neurons = 4, output_neurons = 1;
  float desired_error = 0.05, input_scale = 0.1, output_scale = 10.00;
 };




 class CPongLevel : public CIFLevel {
 public:
  CPongLevel(){
   IFLevel[0] = this;
  }
  void Setup_OpenGL(double width, double height);
  void Init_IFAdapter();
  void AdvanceGame();
  void PostOperations();
  void Cleanup();
 };

 //void Init_IFAdapter(engine &engine);
 //void TESTFN_PostOperations(engine &engine);
 //void TESTFN_AddRandomBody(engine &engine);
 //void TEST_Cleanup();
 //
}