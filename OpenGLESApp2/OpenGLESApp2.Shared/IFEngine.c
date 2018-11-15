#include "IFEngine.h"

#include <Eigen/Dense>
using Eigen::MatrixXf;

//extern bool IFEngine_Initialized = false;


ifTbodiesList BodiesList;


void Init_ifTbodyDefinition(ifTbodyDefinition *_init_var){ 
 _init_var->colors = NULL;
 _init_var->indices = NULL;
 _init_var->UVmapping = NULL;
 _init_var->vertices = NULL;
 _init_var->colors_cnt = 0;
 _init_var->indices_cnt = 0;
 _init_var->texture_ID = GL_INVALID_VALUE;
 _init_var->UVmapping_cnt = 0;
 _init_var->vertices_cnt = 0;
 _init_var->vertices_mode = 0;
 _init_var->z_pos = -0.5;
 LoadIdentityMatrix( _init_var->texture_matrix );
 LoadIdentityMatrix( _init_var->modelview_matrix );
 //_init_var->modelview_matrix
}
void Clean_ifTbodyDefinition(ifTbodyDefinition *_init_var){
 if( _init_var->colors != NULL ){
  free( _init_var->colors );
  _init_var->colors = NULL;
  _init_var->colors_cnt = 0;

 }
 if (_init_var->indices != NULL) {
  free(_init_var->indices);
  _init_var->indices = NULL;
  _init_var->indices_cnt = 0;

 }
 if (_init_var->UVmapping != NULL) {
  free(_init_var->UVmapping);
  _init_var->UVmapping = NULL;
  _init_var->UVmapping_cnt = 0;

 }
 if (_init_var->vertices != NULL) {
  free(_init_var->vertices);
  _init_var->vertices = NULL;
  _init_var->vertices_cnt = 0;
 }
}


void PrepareDraw(){
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void DrawBodies() {

 glMatrixMode(GL_MODELVIEW);
 glEnableClientState(GL_VERTEX_ARRAY);

 for( int bodies_count = 0; bodies_count < BodiesList.bodies_cnt; bodies_count++ ){
  ifTbodyDefinition *pifebody = BodiesList.bodies[bodies_count];
  //pifebody->vertices;
  //pifebody->indices;

  //load vertices
  glFrontFace(GL_CCW);
  //GLfloat *vertices = new GLfloat[ 1 * 2];
  glVertexPointer(2, GL_FLOAT, sizeof(pifebody->vertices[0]) * 2, pifebody->vertices);

  if(( pifebody->colors_cnt > 0 ) && ((pifebody->UVmapping_cnt == 0) || (pifebody->texture_ID == GL_INVALID_VALUE))){
   glEnableClientState(GL_COLOR_ARRAY);
   glColorPointer(4,GL_FLOAT,0, pifebody->colors);
  }

  if ((pifebody->UVmapping_cnt > 0)&&(pifebody->texture_ID!= GL_INVALID_VALUE)) {
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, pifebody->texture_ID);
   glActiveTexture(GL_TEXTURE0);
   glTexCoordPointer(2, GL_FLOAT, sizeof(GLfloat) * 2, pifebody->UVmapping);
   //GLenum error = glGetError();
  }

 
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity(); 
  glLoadMatrixf( pifebody->modelview_matrix );
  glTranslatef( 0.0f, 0.0f, pifebody->z_pos );

 
  if (pifebody->indices_cnt > 0) {
   glDrawElements(pifebody->vertices_mode, pifebody->indices_cnt, GL_UNSIGNED_BYTE, pifebody->indices);
  } else {
   glDrawArrays(pifebody->vertices_mode, 0, pifebody->vertices_cnt * 0.5);
  }
  
 }

}

void Init_IFEngine(){
 if(!IFEngine_Initialized)
  BodiesList.bodies = NULL;
 CleanupBodies();
 IFEngine_Initialized = true;
}

void CleanupBodies(){
 if (IFEngine_Initialized) {
  IFEngine_Initialized = false;
  for( ifTCounter cnt = 0; cnt < BodiesList.bodies_cnt; cnt++ ){
   glDeleteTextures(1, &BodiesList.bodies[cnt]->texture_ID);
   Clean_ifTbodyDefinition(BodiesList.bodies[cnt]);
   free(BodiesList.bodies[cnt]);
  }
  if (BodiesList.bodies) {
   free(BodiesList.bodies);
   BodiesList.bodies_cnt = 0;
   BodiesList.bodies = NULL;
  }
 }
}