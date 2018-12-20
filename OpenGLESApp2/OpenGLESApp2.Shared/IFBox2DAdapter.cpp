#include <IFBox2DAdapter.h>

b2World *IFA_World;
//All box2D coordinates are multiplied by this factor. Typical value is arround 0.01
float IFA_box2D_factor;

float speedFactor = 1.0f;

GLfloat zDefaultLayer = -0.5;