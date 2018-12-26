#include <IFGameEditor.h>

#include "../OpenGLESApp2.Android.NativeActivity/IFGlobals.h"

namespace IFGameEditor {
 //Returns true if new touch event happened
//x and y are fed with latest coordinates
 unsigned long int GetTouchEvent(int *_x, int *_y){
  if (_x&&_y) *_x = User_Data.Touch_Input_X, *_y = User_Data.Touch_Input_Y;
  if(User_Data.Event_Indicator_Touch_Input != Event_Indicator_Touch_Input ){
   if (_x&&_y) Event_Indicator_Touch_Input = User_Data.Event_Indicator_Touch_Input;
   return User_Data.Touch_Time;
  }
  return 0;
 } 
}

