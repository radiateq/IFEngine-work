#ifndef __Cube_Test_Interface__
#define __Cube_Test_Interface__




//Include this file in graphics engine and client software to define data types used for passing data
//typedef struct S_Cube_Test_Update_User_Data;
typedef struct {
 //Current shader global program object (glCreateProgram ret val)
 GLuint program_object; 
 struct android_app* state;
 double Animation_Direction_X;
 double Animation_Direction_Y;
 double screenWidth;
 double screenHeight;
 GLuint CubeTexture;
}TS_Cube_Test_Update_User_Data;



typedef int(*Tp_Cube_Test_Init)(const void *);

// - pointer for passing data from file including .h file, 
// - usage varies on function being called
// - bare in mind when chaining function calls using this variable
// - function must check if pointer is NULL before using this variable (treat as user variable ie perform checks)
// - function accepts this parameter as optional parameter with default value of NULL
//List of functions using p_user_data
//This pointer is casted to any type defined in CubeTest_Test_Interface.h
//01 - CubeTest_update
extern void *p_user_data;

#endif
