#pragma once

#include "MainProgram.h"

namespace Level02{

 class CRacerLevel : public CIFLevel {
 public:
  CRacerLevel() {   
  }
  void Setup_OpenGL(double width, double height);
  void Init_IFAdapter();
  void AdvanceGame();
  void PostOperations();
  void Cleanup();
 };

}