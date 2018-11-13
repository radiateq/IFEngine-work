#pragma once

#include <GLES/gl.h>
#include <GLES/egl.h>

#include <vector>  // Include STL vector class.

//#include <Eigen/Dense>

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


