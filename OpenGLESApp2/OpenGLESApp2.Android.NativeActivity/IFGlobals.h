#ifndef __Cube_Test_Interface__
#define __Cube_Test_Interface__




//Include this file in graphics engine and client software to define data types used for passing data
//typedef struct S_Cube_Test_Update_User_Data;
typedef struct {
 //Current shader global program object (glCreateProgram ret val)
 GLuint program_object; 
 struct android_app* state;
 double Touch_Input_X;
 double Touch_Input_Y;
 unsigned int Event_Indicator_Touch_Input;
 bool pointer_down;
 unsigned long int Touch_Time;
 double screenWidth;
 double screenHeight;
 //GLuint CubeTexture;
 //GLuint Textures[10];
 bool TrainInProgress;
 std::vector<float> accel[2];
}TS_User_Data;
extern TS_User_Data User_Data;



typedef int(*Tp_Cube_Test_Init)(const void *);

// - pointer for passing data from file including .h file, 
// - usage varies on function being called
// - bare in mind when chaining function calls using this variable
// - function must check if pointer is NULL before using this variable (treat as user variable ie perform checks)
// - function accepts this parameter as optional parameter with default value of NULL
//List of functions using p_user_data
//This pointer is casted to any type defined in CubeTest_Test_Interface.h
//01 - CubeTest_update
//extern void *p_user_data;

#endif
