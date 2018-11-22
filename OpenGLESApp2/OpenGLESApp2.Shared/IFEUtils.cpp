#include <IFEUtils.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static const float EPSILON = 0.0000000001f;

namespace IFEUtils {
GLfloat ratio, zvnear, zvfar;
GLfloat left, right, top, bottom, znear, zfar;
}

float Triangulate::Area(const Vector2dVector &contour)
{

 int n = contour.size();

 float A = 0.0f;

 for (int p = n - 1, q = 0; q < n; p = q++)
 {
  A += contour[p].GetX()*contour[q].GetY() - contour[q].GetX()*contour[p].GetY();
 }
 return A * 0.5f;
}

/*
  InsideTriangle decides if a point P is Inside of the triangle
  defined by A, B, C.
*/
bool Triangulate::InsideTriangle(float Ax, float Ay,
 float Bx, float By,
 float Cx, float Cy,
 float Px, float Py)

{
 float ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
 float cCROSSap, bCROSScp, aCROSSbp;

 ax = Cx - Bx;  ay = Cy - By;
 bx = Ax - Cx;  by = Ay - Cy;
 cx = Bx - Ax;  cy = By - Ay;
 apx = Px - Ax;  apy = Py - Ay;
 bpx = Px - Bx;  bpy = Py - By;
 cpx = Px - Cx;  cpy = Py - Cy;

 aCROSSbp = ax * bpy - ay * bpx;
 cCROSSap = cx * apy - cy * apx;
 bCROSScp = bx * cpy - by * cpx;

 return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
};

bool Triangulate::Snip(const Vector2dVector &contour, int u, int v, int w, int n, int *V)
{
 int p;
 float Ax, Ay, Bx, By, Cx, Cy, Px, Py;

 Ax = contour[V[u]].GetX();
 Ay = contour[V[u]].GetY();

 Bx = contour[V[v]].GetX();
 By = contour[V[v]].GetY();

 Cx = contour[V[w]].GetX();
 Cy = contour[V[w]].GetY();

 if (EPSILON > (((Bx - Ax)*(Cy - Ay)) - ((By - Ay)*(Cx - Ax)))) return false;

 for (p = 0; p < n; p++)
 {
  if ((p == u) || (p == v) || (p == w)) continue;
  Px = contour[V[p]].GetX();
  Py = contour[V[p]].GetY();
  if (InsideTriangle(Ax, Ay, Bx, By, Cx, Cy, Px, Py)) return false;
 }

 return true;
}

bool Triangulate::Process(const Vector2dVector &contour, unsigned long int &indices_cnt, unsigned char *&indices){
 /* allocate and initialize list of Vertices in polygon */
 if( indices != NULL ){
  free(indices);
  indices = NULL;
 }
 
 int n = contour.size();
 if (n < 3) return false;

 int *V = new int[n];

 /* we want a counter-clockwise polygon in V */

 if (0.0f < Area(contour))
  for (int v = 0; v < n; v++) V[v] = v;
 else
  for (int v = 0; v < n; v++) V[v] = (n - 1) - v;

 int nv = n;

 /*  remove nv-2 Vertices, creating 1 triangle every time */
 int count = 2 * nv;   /* error detection */

 for (int m = 0, v = nv - 1; nv > 2; )
 {
  /* if we loop, it is probably a non-simple polygon */
  if (0 >= (count--))
  {
   //** Triangulate: ERROR - probable bad polygon!
   return false;
  }

  /* three consecutive vertices in current polygon, <u,v,w> */
  int u = v; if (nv <= u) u = 0;     /* previous */
  v = u + 1; if (nv <= v) v = 0;     /* new v    */
  int w = v + 1; if (nv <= w) w = 0;     /* next     */

  if (Snip(contour, u, v, w, nv, V))
  {
   int a, b, c, s, t;

   /* true names of the vertices */
   a = V[u]; b = V[v]; c = V[w];

   /* output Triangle */
   indices = ( unsigned char * )( realloc( indices, indices_cnt + 3 ) );

   indices[indices_cnt++] = a;
   indices[indices_cnt++] = b;
   indices[indices_cnt++] = c;

   //result.push_back(contour[a]);
   //result.push_back(contour[b]);
   //result.push_back(contour[c]);

   m++;

   /* remove v from remaining polygon */
   for (s = v, t = v + 1; t < nv; s++, t++) V[s] = V[t]; nv--;

   /* resest error detection counter */
   count = 2 * nv;
  }
 }

 delete [] V;

 return true;
}



//
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <assert.h>
//
//
//#include "triangulate.h"
//
//void main(int argc, char **argv)
//{
//
// // Small test application demonstrating the usage of the triangulate
// // class.
//
//
// // Create a pretty complicated little contour by pushing them onto
// // an stl vector.
//
// Vector2dVector a;
//
// a.push_back(Vector2d(0, 6));
// a.push_back(Vector2d(0, 0));
// a.push_back(Vector2d(3, 0));
// a.push_back(Vector2d(4, 1));
// a.push_back(Vector2d(6, 1));
// a.push_back(Vector2d(8, 0));
// a.push_back(Vector2d(12, 0));
// a.push_back(Vector2d(13, 2));
// a.push_back(Vector2d(8, 2));
// a.push_back(Vector2d(8, 4));
// a.push_back(Vector2d(11, 4));
// a.push_back(Vector2d(11, 6));
// a.push_back(Vector2d(6, 6));
// a.push_back(Vector2d(4, 3));
// a.push_back(Vector2d(2, 6));
//
// // allocate an STL vector to hold the answer.
//
// Vector2dVector result;
//
// //  Invoke the triangulator to triangulate this polygon.
// Triangulate::Process(a, result);
//
// // print out the results.
// int tcount = result.size() / 3;
//
// for (int i = 0; i < tcount; i++)
// {
//  const Vector2d &p1 = result[i * 3 + 0];
//  const Vector2d &p2 = result[i * 3 + 1];
//  const Vector2d &p3 = result[i * 3 + 2];
//  printf("Triangle %d => (%0.0f,%0.0f) (%0.0f,%0.0f) (%0.0f,%0.0f)\n", i + 1, p1.GetX(), p1.GetY(), p2.GetX(), p2.GetY(), p3.GetX(), p3.GetY());
// }
//
//}






// To find orientation of ordered triplet (p1, p2, p3). 
// The function returns following values 
// 0 --> p, q and r are colinear 
// 1 --> Clockwise 
// 2 --> Counterclockwise 
int orientation(const Vector2d &p1, const Vector2d &p2, const Vector2d &p3)
{
 // See 10th slides from following link for derivation 
 // of the formula 
 int val = (p2.y - p1.y) * (p3.x - p2.x) -
  (p2.x - p1.x) * (p3.y - p2.y);

 if (val == 0) return 0;  // colinear 

 return (val > 0) ? 1 : 2; // clock or counterclock wise 
}

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
unsigned decodeBMP(std::vector<unsigned char>& image, unsigned& w, unsigned& h, const std::vector<unsigned char>& bmp)
{
 static const unsigned MINHEADER = 54; //minimum BMP header size

 if (bmp.size() < MINHEADER) return -1;
 if (bmp[0] != 'B' || bmp[1] != 'M') return 1; //It's not a BMP file if it doesn't start with marker 'BM'
 unsigned pixeloffset = bmp[10] + 256 * bmp[11]; //where the pixel data starts
 //read width and height from BMP header
 w = bmp[18] + bmp[19] * 256;
 h = bmp[22] + bmp[23] * 256;
 //read number of channels from BMP header
 if (bmp[28] != 24 && bmp[28] != 32) return 2; //only 24-bit and 32-bit BMPs are supported.
 unsigned numChannels = bmp[28] / 8;

 //The amount of scanline bytes is width of image times channels, with extra bytes added if needed
 //to make it a multiple of 4 bytes.
 unsigned scanlineBytes = w * numChannels;
 if (scanlineBytes % 4 != 0) scanlineBytes = (scanlineBytes / 4) * 4 + 4;

 unsigned dataSize = scanlineBytes * h;
 if (bmp.size() < dataSize + pixeloffset) return 3; //BMP file too small to contain all pixels

 image.resize(w * h * 4);

 /*
 There are 3 differences between BMP and the raw image buffer for LodePNG:
 -it's upside down
 -it's in BGR instead of RGB format (or BRGA instead of RGBA)
 -each scanline has padding bytes to make it a multiple of 4 if needed
 The 2D for loop below does all these 3 conversions at once.
 */
 for (unsigned y = 0; y < h; y++)
  for (unsigned x = 0; x < w; x++)
  {
   //pixel start byte position in the BMP
   unsigned bmpos = pixeloffset + (h - y - 1) * scanlineBytes + numChannels * x;
   //pixel start byte position in the new raw image
   unsigned newpos = 4 * y * w + 4 * x;
   if (numChannels == 3)
   {
    image[newpos + 0] = bmp[bmpos + 2]; //R
    image[newpos + 1] = bmp[bmpos + 1]; //G
    image[newpos + 2] = bmp[bmpos + 0]; //B
    image[newpos + 3] = 255;            //A
   }
   else
   {
    image[newpos + 0] = bmp[bmpos + 3]; //R
    image[newpos + 1] = bmp[bmpos + 2]; //G
    image[newpos + 2] = bmp[bmpos + 1]; //B
    image[newpos + 3] = bmp[bmpos + 0]; //A
   }
  }
 return 0;
}

void Setup_OpenGL(double width, double height) {
 ((TS_User_Data*)p_user_data)->screenWidth = width;
 ((TS_User_Data*)p_user_data)->screenHeight = height;
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
 glDisable(GL_TEXTURE_2D);
 glDisable(GL_DITHER);
 glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
 glClearColor(0.35f, 0.2f, 0.65f, 1.0f);
 //glDisable(GL_CULL_FACE); //     ORIGINAL VALUE glEnable(GL_CULL_FACE);
 glShadeModel(GL_SMOOTH);
 //glDisable(GL_DEPTH_TEST); //     ORIGINAL VALUE glEnable(GL_DEPTH_TEST);
 glEnable(GL_NORMALIZE);

 IFEUtils::zvnear = 0.0;
 IFEUtils::zvfar = 1.0;
 glDepthRangef(IFEUtils::zvnear, IFEUtils::zvfar);
 glViewport(0, 0, width, height);

 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();

 IFEUtils::ratio = (GLfloat)width / (GLfloat)height;
 if (IFEUtils::ratio > 1.0f) {
  IFEUtils::ratio = (GLfloat)height / (GLfloat)width;
  IFEUtils::left = -1.0;
  IFEUtils::right = 1.0;
  IFEUtils::bottom = -IFEUtils::ratio;
  IFEUtils::top = IFEUtils::ratio;
 }
 else {
  IFEUtils::left = -IFEUtils::ratio;
  IFEUtils::right = IFEUtils::ratio;
  IFEUtils::bottom = -1.0;
  IFEUtils::top = 1.0;
 }
 IFEUtils::znear = 0.1;
 IFEUtils::zfar = 100;

 glFrustumf(IFEUtils::left, IFEUtils::right, IFEUtils::bottom, IFEUtils::top, IFEUtils::znear, IFEUtils::zfar);

}


MEMORY_READER_STATE memory_reader_state;
void read_data_memory(png_structp png_ptr, png_bytep data, png_size_t length) {
 //MEMORY_READER_STATE *mrs = (MEMORY_READER_STATE *)png_ptr;
 if (memory_reader_state.png_file_data_position >= memory_reader_state.png_file_data_size) {
  return;
 }
 if ((memory_reader_state.png_file_data_position + length) > memory_reader_state.png_file_data_size) {
  length = memory_reader_state.png_file_data_size - memory_reader_state.png_file_data_position;
 }
 memcpy(data, &memory_reader_state.png_file_data[memory_reader_state.png_file_data_position], length);
 memory_reader_state.png_file_data_position += length;
}


void Window2ObjectCoordinates(float &x_inout, float &y_inout, float z, float w, float h){
 //Go backwards from viewport to frustrum to transformation coordinates
   // First calculate z because we need it to calculate x and y
   //Since z is known and does not exist in screen coordinates we calculate it as OpenGL ES would
   //RQNDKUtils::SRangeScale2 zRange(IFEUtils::zvnear, IFEUtils::zvfar);
 float zCoord = -z;
 //zCoord = zRange.ScaleAndClip(zCoord, IFEUtils::znear, IFEUtils::zfar);
 //      then frustum matrix   
 zCoord = (zCoord*(IFEUtils::zfar + IFEUtils::znear)) / (IFEUtils::zfar - IFEUtils::znear) + (zCoord*(IFEUtils::zfar* IFEUtils::znear)) / (IFEUtils::zfar - IFEUtils::znear);
 //      perspective division
 //zCoord /= -zCoord;
//   zCoord = ((IFEUtils::zvfar - IFEUtils::zvnear) / 2.0)*zCoord + (IFEUtils::zvnear + IFEUtils::zvfar) / 2.0;


   //               this is result of viewport matrix for x, backwards
 float screenxy;
 //                find center of viewport
 float oxy = (float)w * 0.5;
 //                viewport size in pixels
 float pxy = w;
 //                send back through window
 screenxy = 2.0*(oxy - x_inout) / pxy;
 //                this is result of frustum matrix for x, backwards
 screenxy = (IFEUtils::left*screenxy + IFEUtils::left*(zCoord)-IFEUtils::right*screenxy + IFEUtils::right*(zCoord)) / (2 * IFEUtils::znear);
 //                this is reverse transformation matrix
 x_inout = (screenxy * -z);// LET THE USER DEAL WITH MODEL VIEW - / IFA_box2D_factor;
 //Same for y
 oxy = (float)h  * 0.5;
 pxy = h;
 screenxy = 2.0*(oxy - y_inout) / pxy;
 //                this is reversed frustum matrix
 screenxy = (IFEUtils::bottom*screenxy + IFEUtils::bottom * (zCoord)-IFEUtils::top * screenxy + IFEUtils::top * (zCoord)) / (2 * IFEUtils::znear);
 //                this is reverse transformation matrix
 y_inout = (screenxy*z);
}