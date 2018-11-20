#pragma once

#include "../OpenGLESApp2.Android.NativeActivity/IFGlobals.h"

namespace IFGameEditor{
 static unsigned int Event_Indicator_Touch_Input;
 //Returns true if new touch event happened
 //x and y are fed with latest coordinates
 bool GetTouchEvent(int *_x = NULL, int *_y = NULL);
}