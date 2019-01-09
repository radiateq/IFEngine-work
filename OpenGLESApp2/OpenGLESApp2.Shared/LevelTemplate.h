#pragma once

#include "MainProgram.h"
namespace LevelTemplate{

 class CLevel : public CIFLevel {
 public:
  CLevel() {
  }
  void Setup_OpenGL(double width, double height);
  void Init_IFAdapter();
  void AdvanceGame();
  void PostOperations();
  void Cleanup();
 };

}