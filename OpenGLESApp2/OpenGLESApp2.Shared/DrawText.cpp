#include <DrawText.h>

#include <android/bitmap.h>

FT_Library  library;
FT_Face     face;
int32_t deviceDPI;
FT_GlyphSlot  slot;
FT_Matrix     matrix;              /* transformation matrix */
FT_UInt       glyph_index;
FT_Vector     pen;                 /* untransformed origin */
int           n;

bool InitFreeType(struct android_app* state){

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

 deviceDPI = getDensityDpi(state);

 return true;
}

bool SetFaceSize(FT_F26Dot6  char_width, FT_F26Dot6 char_height ) {
 if (FT_Set_Char_Size(
  face,    /* handle to face object           */
  char_width,       /* char_width in 1/64th of points  */
  char_height,   /* char_height in 1/64th of points */
  deviceDPI,     /* horizontal device resolution    */
  deviceDPI)) {  /* vertical device resolution      */
  return false;
 }
 return true;
}


bool DrawText(char *_text, FT_UInt _target_height, double _angle) {

 // slot = face->glyph;                /* a small shortcut */
 FT_Matrix matrix;
 // set up matrix 
 matrix.xx = (FT_Fixed)(cos(_angle) * 0x10000L);
 matrix.xy = (FT_Fixed)(-sin(_angle) * 0x10000L);
 matrix.yx = (FT_Fixed)(sin(_angle) * 0x10000L);
 matrix.yy = (FT_Fixed)(cos(_angle) * 0x10000L);

 ///* the pen position in 26.6 cartesian space coordinates 
 ///* start at (300,200)                                   
 pen.x = 300 * 64;
 pen.y = (_target_height - 200) * 64;
 size_t num_chars = strlen(_text);

 for (size_t n = 0; n < num_chars; n++) {
  // set transformation 
  FT_Set_Transform(face, &matrix, &pen);

  //load glyph image into the slot (erase previous one) 
  FT_Error error = FT_Load_Char(face, _text[n], FT_LOAD_RENDER);
  if (error)
   continue;  /* ignore errors */

///* now, draw to our target surface (convert position) */
// my_draw_bitmap(&slot->bitmap,
//  slot->bitmap_left,
//  my_target_height - slot->bitmap_top);


   

  GLuint texture;
  glGenTextures(1, &texture);
  glActiveTexture(GL_TEXTURE0 + 1);

  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, slot->bitmap.width, slot->bitmap.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)(slot->bitmap.buffer));

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   

      


  //increment pen position 
  pen.x += slot->advance.x;
  pen.y += slot->advance.y;
 }
}