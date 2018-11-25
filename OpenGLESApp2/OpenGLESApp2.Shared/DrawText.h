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








