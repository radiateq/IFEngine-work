#pragma once

#include "MainProgram.h"

namespace Level01{






 class CPongLevel : public CIFLevel {
 public:
  CPongLevel(){   
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