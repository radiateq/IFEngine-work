#include "CubeTest.h"

float _rotation_x, _rotation_y;
//#include <sys/time.h>
#include "pch.h"

#include "limits.h"

#include "Cube_Test_Interface.h"


#include <Eigen/Dense>
using Eigen::MatrixXf;

//Start time used to compute relative time for the frame being rendered
//Time is captured inside CubeTest_setupGL
struct timespec game_time_0;


//GLOBAL VARIABLES DESPITE OF ALL TEACHINGS AGAINST IT BECAUSE NOT EVERYONE SHOULD PROGRAM IN C
int twidth, theight;
GLuint textInt;

//Temporary variables
struct timespec temp_timespec;
int64_t temp_int64;
int temp_int;
double temp_dbl;


#define TEMP_VERT 0.05
#define TEMP_VERT_Z 0.0
#define TEMP_VERT_UV 2.0
static GLfloat vertices[][3] =
{
 { -TEMP_VERT , -TEMP_VERT, -TEMP_VERT_Z },
 { TEMP_VERT, -TEMP_VERT, -TEMP_VERT_Z },
 { TEMP_VERT,  TEMP_VERT, -TEMP_VERT_Z },
 { -TEMP_VERT,  TEMP_VERT, -TEMP_VERT_Z }
};
static GLfloat UVcoordinates[][2] =
{
 { 0.0f, 0.0f },
 { TEMP_VERT_UV, 0.0f},
 { TEMP_VERT_UV, TEMP_VERT_UV},
 { 0.0f, TEMP_VERT_UV } 
};
//static GLfloat vertices[][4] =
//{
// { -TEMP_VERT * 1.2, -TEMP_VERT * 1.2,  0.0f, TEMP_VERT_UV },
//{ TEMP_VERT, -TEMP_VERT, TEMP_VERT_UV, TEMP_VERT_UV},
//{ TEMP_VERT,  TEMP_VERT, TEMP_VERT_UV, 0.0f},
//{ -TEMP_VERT,  TEMP_VERT, 0.0f, 0.0f }
//};
#undef TEMP_VERT
#undef TEMP_VERT_Z
#undef TEMP_VERT_UV

static GLfloat colors[][4] =
{
{ 1.0, 0.0, 0.0, 1.0 },
{ 0.0, 1.0, 0.0, 1.0 },
{ 0.0, 0.0, 1.0, 1.0 },
{ 1.0, 1.0, 1.0, 1.0 }
};

GLubyte indices[] =
{
 0, 1, 2,  0, 2, 3
};

void CubeTest_setupGL(double width, double height)
{

 ((TS_Cube_Test_Update_User_Data*)p_user_data)->screenWidth = width;
 ((TS_Cube_Test_Update_User_Data*)p_user_data)->screenHeight = height;
 //// Initialize GL state for 3D
 //glDisable(GL_DITHER);
 //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
 //glClearColor(1.0f, 0.41f, 0.71f, 1.0f);
 //glEnable(GL_CULL_FACE);
 //glShadeModel(GL_SMOOTH);
 //glEnable(GL_DEPTH_TEST);

 //glViewport(0, 0, width, height);
 //GLfloat ratio = (GLfloat)width / height;
 //glMatrixMode(GL_PROJECTION);
 //glLoadIdentity();
 //glFrustumf(-ratio, ratio, -1, 1, 1, 10);




 // Initialize GL state for 2D
 glDisable(GL_DITHER);
 glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
 glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
 //glDisable(GL_CULL_FACE); //     ORIGINAL VALUE glEnable(GL_CULL_FACE);
 glShadeModel(GL_SMOOTH);
 //glDisable(GL_DEPTH_TEST); //     ORIGINAL VALUE glEnable(GL_DEPTH_TEST);
 


 glDepthRangef(0.1,1.0);
 glViewport(0, 0, width, height);

 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();

 GLfloat ratio = (GLfloat)width / height;
 if (ratio > 1.0f) {
  ratio = (GLfloat)height / width;
  glFrustumf(-1.0f, 1.0f, -ratio, ratio, 0.1f, 1000.0f);
 }
 else {
  glFrustumf(-ratio, ratio, -1.0f, 1.0f, 0.1f, 1000.0f);
 }




////Ortho START
// glMatrixMode(GL_PROJECTION);
// glLoadIdentity();
// glOrthof(0, width, height, 0, 0, 0);
// GLenum err = glGetError();
// glMatrixMode(GL_MODELVIEW);
// glLoadIdentity();
// glDepthMask(GL_FALSE);
// //Ortho STOP

 

 // TEXTURES START
textInt = png_texture_load("ugly.png", &twidth, &theight);
 // TEXTIRES STOP

 clock_gettime(CLOCK_MONOTONIC, &temp_timespec);
 game_time_0 = temp_timespec;

}

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

 
 memory_reader_state.png_file_data = (png_byte*)getAssetFileToBuffer(((TS_Cube_Test_Update_User_Data*)p_user_data)->state,"ugly.png", memory_reader_state.png_file_data_size);
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



void CubeTest_update()
{
 clock_gettime(CLOCK_MONOTONIC, &temp_timespec);
 //temp_int64 = timespec2ms64(&temp_timespec) - timespec2ms64(&game_time_0);
 temp_int64 = timespec2us64(&temp_timespec) - timespec2us64(&game_time_0);
 game_time_0 = temp_timespec;


 if (p_user_data != NULL) {
  temp_dbl = ((TS_Cube_Test_Update_User_Data*)p_user_data)->Animation_Direction_X;
  if(temp_dbl == 0.0){
   _rotation_x = 0;
 }else{
  _rotation_x = (temp_dbl / (((TS_Cube_Test_Update_User_Data*)p_user_data)->screenWidth)) * 360.0;// * temp_int64 * 0.0001;
 }
  temp_dbl = ((TS_Cube_Test_Update_User_Data*)p_user_data)->Animation_Direction_Y;
  if (temp_dbl == 0.0) {
   _rotation_y = 0;
  } else {
   _rotation_y = (temp_dbl / (((TS_Cube_Test_Update_User_Data*)p_user_data)->screenHeight)) * 360.0;//  * temp_int64 * 0.0001;
  }
 }
 else {
  _rotation_x += temp_int64;
  _rotation_y += temp_int64;
 }
}
void CubeTest_prepare()
{
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}




void CubeTest_draw()
{
 //glMatrixMode(GL_TEXTURE); 
 
 //glActiveTexture(GL_TEXTURE0);

 glEnableClientState(GL_VERTEX_ARRAY);
 glEnableClientState(GL_TEXTURE_COORD_ARRAY);


 glMatrixMode(GL_MODELVIEW);
 //GLfloat workMatrix1[16], workMatrix2[16];
 //glLoadIdentity(); 
 //glRotatef(_rotation_x, 1, 0, 0);  // X
 //glGetFloatv(GL_MODELVIEW, workMatrix1);
 //glLoadIdentity();
 //glRotatef(_rotation_y, 0, 1, 0);          // Y
 //glGetFloatv(GL_MODELVIEW, workMatrix2);
 

 static double testme = 0.8;
 static double testmespeed = 0.04;


 MatrixXf m1(4,4), m2(4,4), m3(4,4);
 float mfa1[16], mfa2[16], mfa3[16];
glLoadIdentity();
testme += testmespeed * ((float)temp_int64 * 0.0001);
glScalef(testme, testme, 1.0);
if (((testme > 20.0)&&(testmespeed>0 )) || ((testme < 0.4) && (testmespeed < 0))) testmespeed *= -1;
static double LastAnimationInput = 0;
if ((((TS_Cube_Test_Update_User_Data*)p_user_data)->Animation_Direction_Y) != LastAnimationInput) {
 
 LastAnimationInput= (((TS_Cube_Test_Update_User_Data*)p_user_data)->Animation_Direction_Y) / ((((TS_Cube_Test_Update_User_Data*)p_user_data)->screenHeight)*0.5);
 glTranslatef(0.0, (1.0 - LastAnimationInput ), -0.5);
 LastAnimationInput = (((TS_Cube_Test_Update_User_Data*)p_user_data)->Animation_Direction_Y); 
}else{
 glTranslatef(0, testme / 20.0 - 0.8, -0.5);//-0.2 - testme / 60.0);
}
//glTranslatef( 0, 0, -0.2 );
//glRotatef( _rotation_y, 0, 1, 0 );          // Y
//glRotatef( _rotation_x, 1, 0, 0 );  // X
glRotatef(_rotation_x, 0, 0, 1);  // Z
//glTranslatef(0, testme / 20.0 - 0.8, 0.0);
//glGetFloatv(GL_MODELVIEW_MATRIX, mfa1);
//glLoadIdentity();
//glGetFloatv(GL_MODELVIEW_MATRIX, mfa3);
//glRotatef(_rotation_x, 0, 0, 1);  // z                                  
//glGetFloatv(GL_MODELVIEW_MATRIX, mfa2);
//
//glLoadIdentity();
//glScalef(testme, testme, 1.0);
//glTranslatef(0, testme / 20.0 - 0.8, -0.2 - testme / 60.0);
//glGetFloatv(GL_MODELVIEW_MATRIX, mfa3);
//glRotatef(_rotation_x, 0, 0, 1);  // z                                  
//glGetFloatv(GL_MODELVIEW_MATRIX, mfa3);
//
//CopyFloat16ToMatrix(m1, mfa1);
//CopyFloat16ToMatrix(m2, mfa2);
//m3 = m1 * m2;
//
//CopyMatrix16ToFloat(m3,mfa1);
////glRotatef(_rotation_x, 1, 1, 1);  // should be ignored
//glLoadMatrixf(mfa1);


 

 
 //glMultMatrixf(workMatrix1);
 //glLoadIdentity();
 
 
 //glRotatef(_rotation_y, 0, 0, 1);          // Y


// glEnableClientState(GL_COLOR_ARRAY);
// glColorPointer(4,GL_FLOAT,0, colors);

 
 glFrontFace(GL_CCW);
 glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * 3, vertices);

 glEnable(GL_TEXTURE_2D);
 glBindTexture(GL_TEXTURE_2D, textInt);
 glTexCoordPointer(2, GL_FLOAT, sizeof(GLfloat) * 2, UVcoordinates);
 //GLenum error = glGetError();

 glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);


}


