#pragma once

#include <../OpenGLESApp2.Android.NativeActivity/RQ_NDK_Utils.h>

#include <lodepng.h>


#include <ft2build.h>
#include FT_FREETYPE_H

#include <IFEUtils.h>

#include <freetype/ftglyph.h>

extern FT_Library  library;
extern FT_Face     face;
extern int32_t deviceDPI;
extern FT_GlyphSlot  slot;
extern FT_Matrix     matrix;              /* transformation matrix */
extern FT_UInt       glyph_index;
extern FT_Vector     pen;                 /* untransformed origin */
extern int           n;
extern FT_Byte *buffer;
extern struct android_app* android_app_state;

//state can be passed only on a first call
bool InitFreeType(struct android_app* _state = NULL);
void DoneFreeType();
bool SetFaceSize(FT_F26Dot6  char_width = 0, FT_F26Dot6  char_height = 0);
bool SetFontPixelSize(FT_UInt _width = 0, FT_UInt _heigth = 0);
void  computeStringBBox(char *glyphs, FT_BBox  *abbox, float angle, FT_Vector _pos);
GLuint DrawText(char *_text, FT_UInt _target_height, FT_Vector start_pos, double _angle, float *ub = NULL, float *vb = NULL, float *ut = NULL, float *vt = NULL);

inline int next_p2(int a);













/*char font name (must be packed in resources)
SetFontPixelSize font size w,h
font color 
font background
blending type, rendering type (different blending functions, alpha channel, ...)
font charmap - for every character store bb coordinates of a texture in a map

potential problems
- without rendering twice, exact bb coordinates cant be known, therefore texture size is unknown
- if rotating text, characters may overlap; disable rotation when rendering to map
- must query for largest texture GPU can handle
- trim text that does not fit
*/

class CIFTextRender{
public:
 CIFTextRender(){
  strcpy(font_file_name,"Roboto-Thin.ttf");
 }
 ~CIFTextRender(){
  DoneFreeType();
 }
 //////////////////////////////////////////////////////////////////////////////////////////////////
 struct SRGBA {
  unsigned char r, g, b, a;
 };
 union URGBA {
  SRGBA rgba;
  unsigned int field : 32;
 };
private:
 URGBA workRGBA;
public:
 URGBA SetRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a ){
  workRGBA.rgba.r = r;
  workRGBA.rgba.g = g;
  workRGBA.rgba.b = b;
  workRGBA.rgba.a = a;
  return workRGBA;
 }
 //////////////////////////////////////////////////////////////////////////////////////////////////

private:
 FT_Library  library;
 FT_Face     face;
 FT_GlyphSlot  slot;
 FT_Matrix     matrix;              /* transformation matrix */
 FT_UInt       glyph_index;
 FT_Vector     pen;                 /* untransformed origin */
 FT_Byte      *buffer = NULL;
 URGBA foreground, background; 
 GLuint texture_ID;
 GLuint texture_map_max_width, texture_map_max_height;//read this from OpenGLES driver
 int char_width = 0, char_height = 0;
 int char_width_px = 0, char_height_px = 0;
 struct android_app* android_app_state = NULL;
 char font_file_name[BUFSIZ+1];


public:
 int32_t deviceDPI;



public:

 void SetBackgroundColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a ){
  background = SetRGBA(r,g,b,a);
 }
 void SetBackgroundColor(unsigned int filed){
  background.field = filed;
 }
 void SetForegroundColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a ){
  foreground = SetRGBA(r,g,b,a); 
 }
 void SetForegroundColor(unsigned int field ){
  foreground.field = field;
 }
 bool SetCharSize_px(FT_UInt _width = 0, FT_UInt _heigth = 0 ){
  if (buffer == NULL) 
   return false;

  if(_width!=0|| _heigth!=0){
   char_width_px = _width;
   char_height_px = _heigth;
  }
  char_width = char_height = 0;
  //If you want to specify the(integer) pixel sizes yourself, you can call FT_Set_Pixel_Sizes.
  FT_Error error = FT_Set_Pixel_Sizes(
   face,   /* handle to face object */
   char_width_px,      /* pixel_width           */
   char_height_px);   /* pixel_height          */
                      //This example sets the character pixel sizes to 16�16 pixels.As previously, a value of 0 for one of the dimensions means �same as the other�.
  if (error) { return false; }
  //Note that both functions return an error code.Usually, an error occurs with a fixed - size font format(like FNT or PCF) when trying to set the pixel size to a value that is not listed in the face->fixed_sizes array.
  return true;
 }
 bool SetCharSize_F26Dot6(FT_F26Dot6 _char_width = 0, FT_F26Dot6 _char_height = 0 ) {
  if (buffer == NULL) 
   return false;

  if(_char_width!=0||_char_height!=0){
   char_width = _char_width;
   char_height = _char_height;
  }
  char_width_px = char_height_px = 0;
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



 bool InitTextRender(char const * const _font_file_name, struct android_app* _state){
  android_app_state = _state;  
  strncpy(font_file_name,_font_file_name, BUFSIZ);

  return InitFreeType();
 }

 bool InitFreeType(){

  if (buffer != NULL) {
   DoneFreeType();  
  }

  size_t size;
  //buffer = (FT_Byte*)RQNDKUtils::getAssetFileToBuffer(android_app_state, "RobotoMono-Regular.ttf", size);
  buffer = (FT_Byte*)RQNDKUtils::getAssetFileToBuffer(android_app_state, font_file_name, size);//"Roboto-Thin.ttf"

  if (buffer == NULL){
   return false;
  }

  FT_Error error = FT_Init_FreeType(&library);
  if (error) { 
   free(buffer);
   return false;
  }

  error = FT_New_Memory_Face(library,
   buffer,    
   size,      
   0,         
   &face); 
  if (error) { 
   free(buffer); 
   return false; 
  }

  deviceDPI = RQNDKUtils::getDensityDpi(android_app_state);

  return true;
 }

 void DoneFreeType(){ 
  if(buffer){
   FT_Done_Face(face);
   free(buffer), buffer = NULL;
  }
 }





 bool computeStringBBox(char *facestring, FT_BBox  *abbox, float angle, FT_Vector _pos )
 {
  if (buffer == NULL) 
   return false;

  FT_BBox  bbox;
  FT_BBox  glyph_bbox;

  FT_Matrix matrix;
  // set up matrix 
  matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
  matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
  matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
  matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);

  /* initialize string bbox to "empty" values */
  bbox.xMin = bbox.yMin = 32000;
  bbox.xMax = bbox.yMax = -32000;
  FT_Error error;
  int num_chars = strlen(facestring);
  bool use_kerning = FT_HAS_KERNING(face);
  FT_UInt previous = 0;
  //memset(&glyph,0,sizeof(glyph));
  for (size_t n = 0; n < num_chars; n++) {
   error = FT_Load_Char(face, (unsigned char)facestring[n], FT_LOAD_DEFAULT);
   if (error)
    continue;  /* ignore errors */

   if (use_kerning && previous && glyph_index)
   {
    FT_Vector  delta;
    FT_Get_Kerning(face, previous, glyph_index,
     FT_KERNING_DEFAULT, &delta);
    _pos.x += delta.x>>6;
    _pos.y += delta.y>>6;
   }
   previous = glyph_index;
   {
    FT_Glyph glyph;
    error = FT_Get_Glyph(face->glyph, &glyph);
    FT_Glyph_Get_CBox(glyph, ft_glyph_bbox_pixels, &glyph_bbox);
    FT_Done_Glyph(glyph);
   }

   FT_Set_Transform(face, &matrix, &_pos);

   _pos.x += face->glyph->advance.x>>6;
   _pos.y += face->glyph->advance.y>>6;

   glyph_bbox.xMin += _pos.x;
   glyph_bbox.xMax += _pos.x;
   glyph_bbox.yMin += _pos.y;
   glyph_bbox.yMax += _pos.y;

   if (glyph_bbox.xMin < bbox.xMin)
    bbox.xMin = glyph_bbox.xMin;

   if (glyph_bbox.yMin < bbox.yMin)
    bbox.yMin = glyph_bbox.yMin;

   if (glyph_bbox.xMax > bbox.xMax)
    bbox.xMax = glyph_bbox.xMax;

   if (glyph_bbox.yMax > bbox.yMax)
    bbox.yMax = glyph_bbox.yMax;


  }


  /* check that we really grew the string bbox */
  if (bbox.xMin > bbox.xMax){
   bbox.xMin = 0;
   bbox.yMin = 0;
   bbox.xMax = 0;
   bbox.yMax = 0;
  }

  /* return string bbox */
  *abbox = bbox;

  return true;
 }


 GLuint DrawText(char *_text, FT_UInt _target_height, FT_Vector start_pos, double _angle, float *ub, float *vb, float *ut, float *vt) {

  InitFreeType();
  if(char_width){
   SetFaceSize();
  }else{
   SetFontPixelSize();
  }

  GLuint texture = GL_INVALID_VALUE;
  //  _angle=0;

  slot = face->glyph;                /* a small shortcut */

                                     //glDisable(GL_TEXTURE_2D);
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
  pen.x = start_pos.x>>6;
  pen.y = start_pos.y >> 6;
  //pen.x = 0 * 64;
  //pen.y = 0 * 64;
  size_t num_chars = strlen(_text);

  unsigned int predicted_width;
  unsigned int predicted_height;
  int expanded_width = 0;
  int expanded_height = 0;
  GLubyte* expanded_data = NULL;
  struct SRGBA {
   unsigned char r, g, b, a;
  };
  union URGBA {
   SRGBA rgba;
   unsigned int field : 32;
  };
  URGBA foreground, background;
  foreground.rgba.r = 25;
  foreground.rgba.g = 100;
  foreground.rgba.b = 250;
  foreground.rgba.a = 255;
  background.rgba.r = 10;
  background.rgba.g = 19;
  background.rgba.b = 135;
  background.rgba.a = 70;


  FT_BBox stringBBox;
  computeStringBBox(_text, &stringBBox, _angle, pen);
  predicted_width = stringBBox.xMax-stringBBox.xMin;
  predicted_height = stringBBox.yMax - stringBBox.yMin;
  expanded_width = next_p2(predicted_width);
  expanded_height = next_p2(predicted_height);
  // Allocate Memory For The Texture Data.
  //GLubyte* expanded_data = new GLubyte[2 * expanded_width * expanded_height];
  expanded_data = new GLubyte[4 * expanded_width * expanded_height];


  int j = 0, jb;
  int i = 0, ib;
  int k = 0, l=0;
  int k1 = INT_MIN, l1 = INT_MIN;

  for( j = 0; j < expanded_height; j++){
   for (i = 0; i < expanded_width; i++) {
    expanded_data[4 * (i + j * expanded_width) + 0] = background.rgba.r;
    expanded_data[4 * (i + j * expanded_width) + 1] = background.rgba.g;
    expanded_data[4 * (i + j * expanded_width) + 2] = background.rgba.b;
    expanded_data[4 * (i + j * expanded_width) + 3] = background.rgba.a;
   }
  }


  j = i = 0;
  unsigned int bmp_width, bmp_height;
  FT_UInt glyph_index;
  FT_Error error;
  bool use_kerning = FT_HAS_KERNING(face);
  FT_UInt previous = 0;
  for (size_t n = 0; n < num_chars; n++) {

   //load glyph image into the slot (erase previous one) 
   //error = FT_Load_Char(face, _text[n], FT_LOAD_RENDER);

   glyph_index = FT_Get_Char_Index(face, _text[n]);

   if (use_kerning && previous && glyph_index)
   {
    FT_Vector  delta;
    FT_Get_Kerning(face, previous, glyph_index,
     FT_KERNING_DEFAULT, &delta);
    pen.x += delta.x >> 6;
    pen.y += delta.y >> 6;
   }
   previous = glyph_index;

   error = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
   if (error)
    continue;  /* ignore errors */

               ///* now, draw to our target surface (convert position) */
               // my_draw_bitmap(&slot->bitmap,
               //  slot->bitmap_left,
               //  my_target_height - slot->bitmap_top);
               // set transformation 
   FT_Set_Transform(face, &matrix, &pen);


   bmp_width = slot->bitmap.width, bmp_height = slot->bitmap.rows;
   k = pen.y;
   l = pen.x;
   for (j = (k - bmp_height + slot->bitmap_top), jb = bmp_height-1; j < ((k ) + slot->bitmap_top); j++, jb--) {
    for (i = (l + slot->bitmap_left), ib=0; i < ((l + slot->bitmap_left) +bmp_width); i++, ib++) {
     {
      if((i>= expanded_width)||(j>=expanded_height)||(i <0)||(j<0)){
       continue;
      }
      if (k1 < j) k1 = j;
      if( l1 < i ) l1 = i;
      if ((slot->bitmap.buffer)[ib + bmp_width * jb] > 0) {
       expanded_data[4 * (i + j * expanded_width) + 0] = (unsigned char)((float)(foreground.rgba.r + (slot->bitmap.buffer)[ib + bmp_width * jb])*0.5);
       expanded_data[4 * (i + j * expanded_width) + 1] = (unsigned char)((float)(foreground.rgba.g + (slot->bitmap.buffer)[ib + bmp_width * jb])*0.5);
       expanded_data[4 * (i + j * expanded_width) + 2] = (unsigned char)((float)(foreground.rgba.b + (slot->bitmap.buffer)[ib + bmp_width * jb])*0.5);
       expanded_data[4 * (i + j * expanded_width) + 3] = (unsigned char)((float)(foreground.rgba.a + (slot->bitmap.buffer)[ib + bmp_width * jb])*0.5);
      }
     }    

    }
   }

   pen.x += (slot->advance.x >> 6);
   pen.y += (slot->advance.y >> 6);
  }



  if (ub) *ub = 0;
  if (vb) *vb = 0;
  if (ut) *ut = ((float)l1 + (float)(slot->advance.x >> 6)) / (float)expanded_width;
  if (vt) *vt = ((float)k1 + (float)(slot->advance.y >> 6)) / (float)expanded_height ;

  glBindTexture(GL_TEXTURE_2D, texture);
  //  glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, expanded_width, expanded_height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, (GLvoid*)(expanded_data) );
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, expanded_width, expanded_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)(expanded_data));
  GLenum glerror = glGetError();
  if (glerror){
   delete[] expanded_data;
   return texture;
  }

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


  //free(PNG_image);
  delete[] expanded_data;


  return texture;
 }


};



























// PNG TEXTURE START
/*




 // TEXTURES START
textInt = png_texture_load("ugly.png", &twidth, &theight);
 // TEXTURES STOP




typedef struct {
 png_byte *png_file_data;
 png_size_t png_file_data_size;
 png_size_t png_file_data_position;
} MEMORY_READER_STATE;
MEMORY_READER_STATE memory_reader_state;


void CubeTest_tearDownGL()
{
 if( memory_reader_state.png_file_data ){
  free(memory_reader_state.png_file_data);
  memory_reader_state.png_file_data = NULL;
 }
}


void read_data_memory(png_structp png_ptr, png_bytep data, png_size_t length){
 //MEMORY_READER_STATE *mrs = (MEMORY_READER_STATE *)png_ptr;
 if(memory_reader_state.png_file_data_position >= memory_reader_state.png_file_data_size ){
  return;
 }
 if( (memory_reader_state.png_file_data_position + length) > memory_reader_state.png_file_data_size ){
  length = memory_reader_state.png_file_data_size - memory_reader_state.png_file_data_position;
 }
 memcpy(data, &memory_reader_state.png_file_data[memory_reader_state.png_file_data_position], length);
 memory_reader_state.png_file_data_position += length;
}
GLuint png_texture_load(const char * file_name, int * width, int * height)
{

 // This function was originally written by David Grayson for
 // https://github.com/DavidEGrayson/ahrs-visualizer

 png_byte header[8];

 //FILE *fp = fopen(file_name, "rb");
 //if (fp == 0)
 //{
 // perror(file_name);
 // return 0;
 //}


 memory_reader_state.png_file_data = (png_byte*)getAssetFileToBuffer(((TS_User_Data*)p_user_data)->state,"ugly.png", memory_reader_state.png_file_data_size);
 memory_reader_state.png_file_data_position = 0;
 if (memory_reader_state.png_file_data == NULL)
 {
  //perror(file_name);
  return 0;
 }

 // read the header
 //fread(header, 1, 8, fp);
 //memcpy(header, memory_reader_state.png_file_data, 8);
 read_data_memory((png_structp)&memory_reader_state,header,8);

 if (png_sig_cmp(header, 0, 8))
 {
  //fprintf(stderr, "error: %s is not a PNG.\n", file_name);
  //fclose(fp);
  return 0;
 }

 png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
 if (!png_ptr)
 {
  //fprintf(stderr, "error: png_create_read_struct returned 0.\n");
  //fclose(fp);
  return 0;
 }

 // create png info struct
 png_infop info_ptr = png_create_info_struct(png_ptr);
 if (!info_ptr)
 {
  //fprintf(stderr, "error: png_create_info_struct returned 0.\n");
  png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
  //fclose(fp);
  return 0;
 }

 // create png info struct
 png_infop end_info = png_create_info_struct(png_ptr);
 if (!end_info)
 {
  //fprintf(stderr, "error: png_create_info_struct returned 0.\n");
  png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
  //fclose(fp);
  return 0;
 }

 // the code in this if statement gets called if libpng encounters an error
 if (setjmp(png_jmpbuf(png_ptr))) {
  //fprintf(stderr, "error from libpng\n");
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  //fclose(fp);
  return 0;
 }

 // init png reading
 //png_init_io(png_ptr, fp);
 png_set_read_fn(png_ptr, &memory_reader_state, (png_rw_ptr)read_data_memory);
// png_set_write_fn();


 // let libpng know you already read the first 8 bytes
 png_set_sig_bytes(png_ptr, 8);

 // read all the info up to the image data
 png_read_info(png_ptr, info_ptr);

 // variables to pass to get info
 int bit_depth, color_type;
 png_uint_32 temp_width, temp_height;

 // get info about png
 png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
  NULL, NULL, NULL);

 if (width){ *width = temp_width; }
 if (height){ *height = temp_height; }

 //printf("%s: %lux%lu %d\n", file_name, temp_width, temp_height, color_type);

 if (bit_depth != 8)
 {
  //fprintf(stderr, "%s: Unsupported bit depth %d.  Must be 8.\n", file_name, bit_depth);
  return 0;
 }

 GLint format;
 switch(color_type)
 {
 case PNG_COLOR_TYPE_RGB:
  format = GL_RGB;
  break;
 case PNG_COLOR_TYPE_RGB_ALPHA:
  format = GL_RGBA;
  break;
 default:
  fprintf(stderr, "%s: Unknown libpng color type %d.\n", file_name, color_type);
  return 0;
 }

 // Update the png info struct.
 png_read_update_info(png_ptr, info_ptr);

 // Row size in bytes.
 int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

 // glTexImage2d requires rows to be 4-byte aligned
 rowbytes += 3 - ((rowbytes-1) % 4);

 // Allocate the image_data as a big block, to be given to opengl
 png_byte * image_data = (png_byte *)malloc(rowbytes * temp_height * sizeof(png_byte)+15);
 if (image_data == NULL)
 {
  //fprintf(stderr, "error: could not allocate memory for PNG image data\n");
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  //fclose(fp);
  return 0;
 }

 // row_pointers is for pointing to image_data for reading the png with libpng
 png_byte ** row_pointers = (png_byte **)malloc(temp_height * sizeof(png_byte *));
 if (row_pointers == NULL)
 {
  //fprintf(stderr, "error: could not allocate memory for PNG row pointers\n");
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  free(image_data);
  //fclose(fp);
  return 0;
 }

 // set the individual row_pointers to point at the correct offsets of image_data
 for (unsigned int i = 0; i < temp_height; i++)
 {
  row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
 }

 // read the png into image_data through row_pointers
 png_read_image(png_ptr, row_pointers);
 //GL_MAX_TEXTURE_SIZE
 // Generate the OpenGL texture object

 GLuint texture;
 glGenTextures(1, &texture);
 glActiveTexture(GL_TEXTURE0);

 glBindTexture(GL_TEXTURE_2D, texture);
 glTexImage2D(GL_TEXTURE_2D, 0, format, temp_width, temp_height, 0, format, GL_UNSIGNED_BYTE, image_data);

 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

 // clean up
 png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
 free(image_data);
 free(row_pointers);
 //fclose(fp);
 return texture;
}


*/
// PNG TEXTURE STOP








