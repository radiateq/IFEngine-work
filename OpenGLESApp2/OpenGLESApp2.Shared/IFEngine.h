#pragma once

#include <GLES/gl.h>

//Unsigned integer type for holding count of objects
typedef unsigned long int ifTCounter;

//Structure holding list of functions that can be executed on body

//Structures for holding multiple items related to body
//typedef struct{
// ifTCounter elements_cnt;
// unsigned int stride;
// GLfloat *elements;
//}ifTvertices, ifTcolors, ifTUVmapping;

//typedef struct{
// ifTCounter elements_cnt;
// unsigned int stride;
// GLubyte *elements;
//}ifTindices;

//Structure holding body shape, attributes and textures
typedef struct{
 //Verices
 ifTCounter vertices_cnt;
 GLfloat *vertices;
 //Indexes
 ifTCounter indices_cnt;
 GLubyte *indices;
 //
 GLenum vertices_mode;
 //Color
 ifTCounter colors_cnt; 
 GLfloat *colors;
 //UV
 ifTCounter UVmapping_cnt;
 GLfloat *UVmapping;
 //Z coordinate is always same for body and is used for last transformation translate (z_pos,0,0,1) before drawing
 GLfloat z_pos;
 //matrix model
 //GL_MODELVIEW, GL_TEXTURE
 GLfloat modelview_matrix[16], texture_matrix[16];
 //Texture name
 GLuint texture_ID;
 //Normals (included in box2d b2_shape if needed
}ifTbodyDefinition;

//Structure holding list of bodies
typedef struct{
 ifTCounter bodies_cnt;
 ifTbodyDefinition **bodies;
}ifTbodiesList;

//General callback function, that accepts pointer to void and returns pointer to void
typedef void* ifTcallback (void * );

//Structure for holding event data
typedef struct{
 ifTCounter eventID; 
 //Function executed when event happens that propagates events down the line
 ifTcallback sendEvent;
 //These are subscribers to event 
 ifTCounter subscribers_cnt;
 //sendEvent will call all these functions one by one, whith subscribers_data as parameter
 ifTcallback *subscribers;
 void *subscribers_data;
}ifTevent;

//Structure for holding all events 
typedef struct {
 ifTCounter event_cnt;
 ifTevent *events;
}ifTeventRegistry;


extern ifTbodiesList BodiesList;
//

void Init_ifTbodyDefinition(ifTbodyDefinition *);
void Clean_ifTbodyDefinition(ifTbodyDefinition *);


void PrepareDraw();
void DrawBodies();
void RemoveBodies(ifTbodyDefinition **_body, ifTCounter _body_count = 1);


static bool IFEngine_Initialized = false;
void Init_IFEngine();
void CleanupBodies();
