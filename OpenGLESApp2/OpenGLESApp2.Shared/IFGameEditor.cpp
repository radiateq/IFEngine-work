#include <IFGameEditor.h>

#include "../OpenGLESApp2.Android.NativeActivity/IFGlobals.h"

namespace IFGameEditor {
 //Returns true if new touch event happened
//x and y are fed with latest coordinates
 unsigned long int GetTouchEvent(int *_x, int *_y){
  if (_x&&_y) *_x = ((TS_User_Data*)p_user_data)->Touch_Input_X, *_y = ((TS_User_Data*)p_user_data)->Touch_Input_Y;
  if(((TS_User_Data*)p_user_data)->Event_Indicator_Touch_Input != Event_Indicator_Touch_Input ){
   if (_x&&_y) Event_Indicator_Touch_Input = ((TS_User_Data*)p_user_data)->Event_Indicator_Touch_Input;
   return ((TS_User_Data*)p_user_data)->Touch_Time;
  }
  return 0;
 } 
}

