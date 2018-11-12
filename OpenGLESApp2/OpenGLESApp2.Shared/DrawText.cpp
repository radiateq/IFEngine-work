#include <DrawText.h>


FT_Library  library;


bool InitFreeType(struct android_app* state){
 FT_Library  library;   
 FT_Face     face;      

 size_t size;
 FT_Byte *buffer  = (FT_Byte*)getAssetFileToBuffer( state, "RobotoMono-Regular.ttf", size);
 
 if (buffer == NULL){
  return false;
 }

 FT_Error error = FT_Init_FreeType(&library);
 if (error) { return false; }

 error = FT_New_Memory_Face(library,
  buffer,    
  size,      
  0,         
  &face);
 if (error) { return false; }

 return true;
}