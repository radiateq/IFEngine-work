#pragma once

#include <GLES/gl.h>



//Unsigned integer type for holding count of objects
typedef unsigned long int ifTCounter;



//Structure holding list of functions that can be executed on body

//Structures for holding multiple items related to body
typedef struct{
 ifTCounter elements_cnt;
 unsigned int stride;
 GLfloat *elements;
}ifTvertices, ifTcolors, ifTUVmapping;
typedef struct{
 ifTCounter elements_cnt;
 unsigned int stride;
 GLubyte *elements;
}ifTindices;

//Structure holding body shape, attributes and textures
typedef struct{
 //Verices
 ifTCounter vertices_cnt;
 ifTvertices *vertices;
 //Indexes
 ifTCounter indices_cnt;
 ifTindices *indices;
 //Color
 ifTCounter colors_cnt; 
 ifTcolors *colors;
 //UV
 ifTCounter UVmapping_cnt;
 ifTUVmapping *UVmapping;
}ifTBodyDefinition;

//Structure holding list of bodies
typedef struct{
 ifTCounter bodies_cnt;
 ifTBodyDefinition *bodies;
}ifTBodiesList;

//General callback function, that accepts pointer to void and returns pointer to void
typedef void* ifTcallback (void * );

//Structure for holding event description
typedef struct{
 ifTCounter eventID;
 //Function executed when event happens that propagates events down the line
 ifTcallback sendEvent;
 //These are subscribers to 
 ifTCounter subscribers_cnt;
 ifTcallback *subscribers;
}ifTevent;

//Structure*3 holding list of callback IDs and their functions for defined*1 events (touch, user events, sensors, ...)


//Structure*2 holding event ID and list of callback functions called by event callback functions

//Structure*4 holding all structures with callback functions for event callback functions*2 for certain entity (entitys supported events from list of defined*1 events)

//Structure holding events this body reacts to

//Functions for adding, removing, searching for a body in a list of bodies

//Functions for manipulating properties of body

//Function for presenting body to the screen (update, draw, ...)

//Functions for propagating event down structure*3 down to struct*4 and then struct*2 taking all needed paths

