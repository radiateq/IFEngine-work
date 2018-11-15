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

bool SetFontPixelSize(FT_UInt _width, FT_UInt _heigth ){
 //If you want to specify the(integer) pixel sizes yourself, you can call FT_Set_Pixel_Sizes.
 FT_Error error = FT_Set_Pixel_Sizes(
  face,   /* handle to face object */
  _width,      /* pixel_width           */
  _heigth);   /* pixel_height          */
 //This example sets the character pixel sizes to 16×16 pixels.As previously, a value of 0 for one of the dimensions means ‘same as the other’.
 if (error) { return false; }
 //Note that both functions return an error code.Usually, an error occurs with a fixed - size font format(like FNT or PCF) when trying to set the pixel size to a value that is not listed in the face->fixed_sizes array.
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

// This Function Gets The First Power Of 2 >= The
// Int That We Pass It.
inline int next_p2(int a)
{
 int rval = 1;
 // rval<<=1 Is A Prettier Way Of Writing rval*=2;
 while (rval < a) rval <<= 1;
 return rval;
}

GLuint DrawText(char *_text, FT_UInt _target_height, double _angle) {

 GLuint texture = GL_INVALID_VALUE;

 slot = face->glyph;                /* a small shortcut */

 glDisable(GL_TEXTURE_2D);
 glGenTextures(1, &texture);
 glActiveTexture(GL_TEXTURE0);

 FT_Matrix matrix;
 // set up matrix 
 matrix.xx = (FT_Fixed)(cos(_angle) * 0x10000L);
 matrix.xy = (FT_Fixed)(-sin(_angle) * 0x10000L);
 matrix.yx = (FT_Fixed)(sin(_angle) * 0x10000L);
 matrix.yy = (FT_Fixed)(cos(_angle) * 0x10000L);


 

 ///* the pen position in 26.6 cartesian space coordinates 
 ///* start at (300,200)                                   
 //pen.x = 300 * 64;
 //pen.y = (_target_height - 200) * 64;
 pen.x = 0 * 64;
 pen.y = 0 * 64;
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


  //resize to pot                  slot->bitmap.
   

  unsigned int bmp_width, bmp_height;
  bmp_width = slot->bitmap.width, bmp_height = slot->bitmap.rows;









//convert bmp to png


  //unsigned int lodeerror;
  //std::vector<unsigned char> bmp((slot->bitmap.buffer), (slot->bitmap.buffer) + bmp_height * (slot->bitmap.pitch));
//lodepng::load_file(bmp, argv[1]);
  //std::vector<unsigned char> image;
  //unsigned char *PNG_image; 
  //size_t PNG_image_size;
  //lodeerror = decodeBMP(image, bmp_width, bmp_height, bmp);
  //if (lodeerror)
  //{
  // return texture;
  //}
  //lodeerror = lodepng_encode_memory(&PNG_image, &PNG_image_size, (slot->bitmap.buffer), bmp_width, bmp_height, LCT_GREY, 8);
  //lodeerror = lodepng::decode(image, bmp_width, bmp_height, bmp, LCT_GREY, 8);

// Use Our Helper Function To Get The Widths Of
// The Bitmap Data That We Will Need In Order To Create
// Our Texture.
  int expanded_width = next_p2(bmp_width);
  int expanded_height = next_p2(bmp_height);

  // Allocate Memory For The Texture Data.
  GLubyte* expanded_data = new GLubyte[2 * expanded_width * expanded_height];

  // Here We Fill In The Data For The Expanded Bitmap.
  // Notice That We Are Using A Two Channel Bitmap (One For
  // Channel Luminosity And One For Alpha), But We Assign
  // Both Luminosity And Alpha To The Value That We
  // Find In The FreeType Bitmap.
  // We Use The ?: Operator To Say That Value Which We Use
  // Will Be 0 If We Are In The Padding Zone, And Whatever
  // Is The FreeType Bitmap Otherwise.
  for (int j = 0; j < expanded_height; j++) {
   for (int i = 0; i < expanded_width; i++) {
    expanded_data[2 * (i + j * expanded_width)] = expanded_data[2 * (i + j * expanded_width) + 1] =
     (i >= bmp_width || j >= bmp_height) ?
     0 : (slot->bitmap.buffer)[i + bmp_width *j];
   }
  }


//if (error)
//{ 
// return texture;
//}

//std::vector<unsigned char> png;
//error = lodepng::encode(png, bmp, bmp_width, bmp_height);

  //if (lodeerror)
  //{ 
  // return texture;
  //}  

  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, expanded_width, expanded_height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, (GLvoid*)(expanded_data));
  GLenum glerror = glGetError();
  if (glerror)
  {
   return texture;
  }

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   

  //free(PNG_image);
  delete [] expanded_data;
  //                                    USE BITMAP CLASS TO DRAW STRING AND THEN USE CODE ABOVE TO MAP
//                                 STORE UV COORDS WHERE BORDER IS (POT)
      


  //increment pen position 
  //pen.x += slot->advance.x;
  //pen.y += slot->advance.y;
 }
 return texture;
}