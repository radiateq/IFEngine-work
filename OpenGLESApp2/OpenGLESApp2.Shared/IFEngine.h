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
}ifTbodyDefinition;

//Structure holding list of bodies
typedef struct{
 ifTCounter bodies_cnt;
 ifTbodyDefinition *bodies;
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

//Structure for holding all events that get called by program
typedef struct {
 ifTCounter event_cnt;
 ifTevent *events;
}ifTeventRegistry;



//Structure holding events body reacts to


//Functions for adding, removing, searching for a body in a list of bodies

//Functions for manipulating properties of body

//Function for presenting body to the screen (update, draw, ...)

//Functions for propagating events

