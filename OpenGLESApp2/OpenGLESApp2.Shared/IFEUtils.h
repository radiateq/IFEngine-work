#pragma once

#include <GLES/gl.h>
#include <GLES/egl.h>

#include "../OpenGLESApp2.Android.NativeActivity/IFGlobals.h"

#include <vector>  // Include STL vector class.

//#include <Eigen/Dense>
extern GLfloat ratio;
extern GLfloat left, right, top, bottom, znear, zfar;

class Vector2d
{
public:
 Vector2d(float x, float y)
 {
  Set(x, y);
 };

 float GetX(void) const { return x; };

 float GetY(void) const { return y; };

 void  Set(float _x, float _y)
 {
  x = _x;
  y = _y;
 };
 float x;
 float y;
};

// Typedef an STL vector of vertices which are used to represent
// a polygon/contour and a series of triangles.
typedef std::vector< Vector2d > Vector2dVector;


class Triangulate
{
public:
 // triangulate a contour/polygon, places results in STL vector
 // as series of triangles.
 static bool Process( const Vector2dVector &contour, unsigned long int &indices_cnt, unsigned char *&indices );

  //Vector2dVector &result);

 // compute area of a contour/polygon
 static float Area(const Vector2dVector &contour);

 // decide if point Px/Py is inside triangle defined by
 // (Ax,Ay) (Bx,By) (Cx,Cy)
 static bool InsideTriangle(float Ax, float Ay,
  float Bx, float By,
  float Cx, float Cy,
  float Px, float Py);

private:
 static bool Snip(const Vector2dVector &contour, int u, int v, int w, int n, int *V);

};


void Setup_OpenGL(double width, double height);

typedef struct {
 png_byte *png_file_data;
 png_size_t png_file_data_size;
 png_size_t png_file_data_position;
} MEMORY_READER_STATE;
extern MEMORY_READER_STATE memory_reader_state;


void read_data_memory(png_structp png_ptr, png_bytep data, png_size_t length);


class IFEUtilsLoadTexture{
public:
 IFEUtilsLoadTexture() {
  memory_reader_state.png_file_data = NULL;
 }
 ~IFEUtilsLoadTexture(){
  Clean();
 }



 static void Clean()
 {
  if (memory_reader_state.png_file_data) {
   free(memory_reader_state.png_file_data);
   memory_reader_state.png_file_data = NULL;
  }
 }


 static GLuint png_texture_load(const char * file_name, int * width, int * height)
 {
  Clean();
  // This function was originally written by David Grayson for
  // https://github.com/DavidEGrayson/ahrs-visualizer

  png_byte header[8];

  //FILE *fp = fopen(file_name, "rb");
  //if (fp == 0)
  //{
  // perror(file_name);
  // return 0;
  //}


  memory_reader_state.png_file_data = (png_byte*)getAssetFileToBuffer(((TS_User_Data*)p_user_data)->state, file_name, memory_reader_state.png_file_data_size);
  memory_reader_state.png_file_data_position = 0;
  if (memory_reader_state.png_file_data == NULL)
  {
   //perror(file_name);
   return 0;
  }

  // read the header
  //fread(header, 1, 8, fp);
  //memcpy(header, memory_reader_state.png_file_data, 8);
  read_data_memory((png_structp)&memory_reader_state, header, 8);

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
   png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
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

  if (width) { *width = temp_width; }
  if (height) { *height = temp_height; }

  //printf("%s: %lux%lu %d\n", file_name, temp_width, temp_height, color_type);

  if (bit_depth != 8)
  {
   //fprintf(stderr, "%s: Unsupported bit depth %d.  Must be 8.\n", file_name, bit_depth);
   return 0;
  }

  GLint format;
  switch (color_type)
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
  rowbytes += 3 - ((rowbytes - 1) % 4);

  // Allocate the image_data as a big block, to be given to opengl
  png_byte * image_data = (png_byte *)malloc(rowbytes * temp_height * sizeof(png_byte) + 15);
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


};





// To find orientation of ordered triplet (p1, p2, p3). 
// The function returns following values 
// 0 --> p, q and r are colinear 
// 1 --> Clockwise 
// 2 --> Counterclockwise 
int orientation(const Vector2d &p1, const Vector2d &p2, const Vector2d &p3);

//// Driver program to test above functions 
//int main()
//{
// Point p1 = { 0, 0 }, p2 = { 4, 4 }, p3 = { 1, 2 };
// int o = orientation(p1, p2, p3);
// if (o == 0)         cout << "Linear";
// else if (o == 1)  cout << "Clockwise";
// else              cout << "CounterClockwise";
// return 0;
//}









 //returns 0 if all went ok, non-0 if error
//output image is always given in RGBA (with alpha channel), even if it's a BMP without alpha channel
unsigned decodeBMP(std::vector<unsigned char>& image, unsigned& w, unsigned& h, const std::vector<unsigned char>& bmp);














