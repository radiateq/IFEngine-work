#pragma once

#include <GLES/gl.h>


#include <Box2D/Box2D.h>
#include <list>

#include <IFEngine.h>
#include <IFEUtils.h>

//
//class Iifclass1 {
//public:
// ~Iifclass1() {
//  Free();
// }
// virtual void Init() {
//  Free();
// }
//private:
// virtual void Free(){
// }
// };


extern b2World *IFA_World;
//All box2D coordinates are multiplied by this factor. Typical value is arround 0.01
extern float IFA_box2D_factor;



class ifCB2Body {
public:
 ifCB2Body() {
  body_def = NULL;
  body = NULL;
 }
 ~ifCB2Body() {
  Free();
 }
 void Init() {
  Free();
 }
 ////////////////////////////////////
 b2BodyDef *body_def;
 b2Body *body;
 S_listWrap_ptr<b2Shape, true> shape;
 S_listWrap_ptr<b2FixtureDef, true> fixture;
 ifTbodyDefinition *OGL_body;
 ////////////////////////////////////
 //After body_def, body, shape and fixture have been prepared this function will create a body and add it to the world 
 void CreateBody() {
  if (IFA_World && !body) {
   body = IFA_World->CreateBody(body_def);
   typename std::list<b2Shape*>::iterator iters = shape.begin();
   for (typename std::list<b2FixtureDef*>::iterator iter = fixture.begin();
    iter != fixture.end();
    iter++) {
    (*iter)->shape = (*iters);
    body->CreateFixture(*iter);
    iters++;
   }
   shape.clear();
  }
 }
 //Called before CreateBody at least once
 void AddShapeAndFixture(b2Shape *_shape, b2FixtureDef *_fixture) {
  shape.push_back(_shape);
  fixture.push_back(_fixture);
 }
 //Input parameters:
//shape_index IN
// - index of shape
//vertices_count IN/OUT//  
// - IN  - maximum number of vertices vertices array can hold
// - OUT - number of vertices used
//vertices and vertices_mode OUT
// - vertices must have 2*vertices_count elements available, storing x1,y1,x2,y2,...

 void BodyToVertices(unsigned int shape_index, ifTCounter &vertices_count, ifTCounter &indices_count, GLenum *vertices_mode, float **vertices, unsigned char **indices) {
  //[b2_maxPolygonVertices]
  for (typename std::list<b2FixtureDef*>::iterator iter = fixture.begin();
   iter != fixture.end();
   iter++) {   
   if (shape_index == 0){
    switch ((*iter)->shape->m_type) {
    case b2Shape::e_chain:
    case b2Shape::e_edge:
     *vertices_mode = GL_LINE_LOOP;
     if (*vertices == NULL) {
      break;
     }
     *vertices_mode = GL_LINES;
     break;
    case b2Shape::e_polygon:
    {
     vertices_count = ((b2PolygonShape*)(*iter)->shape)->m_count * 2;
     if(*vertices!=NULL){
      free(*vertices);
     }
     *vertices = (GLfloat*)malloc(sizeof(*vertices[0]) * vertices_count);
     *vertices_mode = GL_TRIANGLES;
     unsigned int cntmax = vertices_count * 0.5;
     //looking for the left first then bottom coordinate
     Vector2dVector v2vertices;
     for (ifTCounter cnt = 0; cnt < cntmax; cnt++) {
      (*vertices)[2 * cnt] = ((b2PolygonShape*)(*iter)->shape)->m_vertices[cnt].x;
      (*vertices)[2 * cnt + 1] = ((b2PolygonShape*)(*iter)->shape)->m_vertices[cnt].y;
      v2vertices.push_back(Vector2d((*vertices)[2 * cnt], (*vertices)[2 * cnt + 1]));
     }
     Triangulate::Process(v2vertices, indices_count, *indices);

     break;
    }
    case b2Shape::e_circle:
     if (*vertices == NULL) {
      break;
     }
     *vertices_mode = GL_LINE_LOOP;
     break;
    }
    //Exit for
    break;
   }
   shape_index--;
  }  
 }
 void Free() {
  if (IFA_World) {
   IFA_World->DestroyBody(body);
  }
  body = NULL;
  IF_NULL_DELETE(body_def);
  shape.clear();
  fixture.clear();
 }
};

class ifCB2BodyManager : public ifCB2Body {
public:
 ifCB2BodyManager() {
  OrderPending = false;
 }
 ~ifCB2BodyManager() {
  Free();
 }
 void Init() {
  Free();
 }
 void OrderBody() {
  if (!OrderPending) {
   Bodies.push_back(new ifCB2Body);
   Bodies.back()->body_def = new b2BodyDef;
   OrderPending = true;
  }
 }
 void MakeBody() {
  if (OrderPending) {
   OrderPending = false;
   Bodies.back()->CreateBody();
   B2Bodies2GLBodies(Bodies.size() - 1);
   ifCB2Body *work_body = Bodies.back();
   DefaultUVMapping( work_body->OGL_body );
  }
 }
 void CancelOrder() {
  if (OrderPending) {
   OrderPending = false;
   Bodies.removeElement(Bodies.back());
  }
 }
 ifCB2Body* OrderedBody() {
  return  (OrderPending ? Bodies.back() : NULL);
 }
 void B2Bodies2GLBodies(ifTCounter ordinary_index) {
  typename std::list<ifCB2Body*>::iterator iter;  
  for (iter = Bodies.begin(); ordinary_index > 0; ordinary_index--) {
   iter++;
  }  
  ifTbodyDefinition *work_body;
  for (unsigned int cnt = 0; cnt < (*iter)->fixture.size(); cnt++) {
   BodiesList.bodies = (ifTbodyDefinition**)(realloc(BodiesList.bodies, sizeof(ifTbodyDefinition*) * (BodiesList.bodies_cnt + 1)));
   BodiesList.bodies[BodiesList.bodies_cnt] = (ifTbodyDefinition*)malloc(sizeof(ifTbodyDefinition));
   work_body = BodiesList.bodies[BodiesList.bodies_cnt];
   Init_ifTbodyDefinition(work_body);
   (*iter)->BodyToVertices(cnt, work_body->vertices_cnt, work_body->indices_cnt, &(work_body->vertices_mode), &(work_body->vertices), &(work_body->indices));
   (*iter)->OGL_body = work_body;
   work_body->colors_cnt = work_body->vertices_cnt * 2;
   work_body->colors = (GLfloat*)malloc(sizeof(GLfloat) * work_body->colors_cnt);
   for( ifTCounter cnt1 = 0; cnt1 < work_body->colors_cnt; cnt1++ ){
    work_body->colors[cnt1] = 1.0f;
   }
   BodiesList.bodies_cnt++;
  }  
 }
 void B2Bodies2GLBodies(){
  for (ifTCounter cnt = 0; cnt < Bodies.size(); cnt++) {
   B2Bodies2GLBodies(cnt);
  }
 }
 void DefaultUVMapping( ifTbodyDefinition *_OGL_body ){
  
//OGL_body->vertices_cnt
//OGL_body->vertices
  GLfloat minx = 10^30, miny = 10 ^ 30, maxx = -10 ^ 30, maxy = -10 ^ 30;
  for( ifTCounter cnt = 0; cnt < _OGL_body->vertices_cnt; cnt++ ){
   if( minx > _OGL_body->vertices[cnt] ){
    minx = _OGL_body->vertices[cnt];
   }
   if (maxx < _OGL_body->vertices[cnt]) {
    maxx = _OGL_body->vertices[cnt];
   }
   cnt++;
   if (miny > _OGL_body->vertices[cnt]) {
    miny = _OGL_body->vertices[cnt];
   }
   if (maxy < _OGL_body->vertices[cnt]) {
    maxy = _OGL_body->vertices[cnt];
   }
  }

  float min_maxx = abs( maxx - minx );
  float min_maxy = abs( maxy - miny );
  float normalization_scalar_x = min_maxx;
  normalization_scalar_x = 1.0 / (normalization_scalar_x);
  float normalization_scalar_y = min_maxy;
  normalization_scalar_y = 1.0 / (normalization_scalar_y);

  _OGL_body->UVmapping_cnt = 0;
  if (_OGL_body->UVmapping != NULL) {
   free(_OGL_body->UVmapping);
  }

  _OGL_body->UVmapping_cnt = _OGL_body->vertices_cnt;
  _OGL_body->UVmapping = (GLfloat*)malloc(sizeof(_OGL_body->UVmapping[0])*_OGL_body->UVmapping_cnt);
  for( ifTCounter cnt = 0; cnt < _OGL_body->UVmapping_cnt; cnt++ ){
   _OGL_body->UVmapping[cnt] = ( _OGL_body->vertices[cnt] - minx ) * normalization_scalar_x;
   cnt++;
   _OGL_body->UVmapping[cnt] = ( _OGL_body->vertices[cnt] - miny ) * normalization_scalar_y;
  }

 }

 //void Add
// b2World *World;
 S_listWrap_ptr<ifCB2Body, true> Bodies;
private:
 bool OrderPending;
 void Free() {
  while(Bodies.size())
   Bodies.removeElement(Bodies.back());
 }
public:

};



class ifCB2GameManager : public ifCB2BodyManager {
public:
 ifCB2GameManager() {
  velocityIterations = 6;   //how strongly to correct velocity
  positionIterations = 2;   //how strongly to correct position
  screenResolutionX = 1;
  screenResolutionY = 1;
  CalculateBox2DSizeFactor(10);
  ResetClock();
 }
 void ResetClock() {
  clock_gettime(CLOCK_MONOTONIC, &temp_timespec);
  game_time_0 = temp_timespec;
 }
 void UpdateSim() {
  clock_gettime(CLOCK_MONOTONIC, &temp_timespec);
  //temp_int64 = timespec2ms64(&temp_timespec) - timespec2ms64(&game_time_0);
  temp_int64 = timespec2us64(&temp_timespec) - timespec2us64( &game_time_0 );
  game_time_0 = temp_timespec;
  timeStep = 10000.0f / (float)temp_int64;
  //timeThen = timeNow;
  IFA_World->Step( timeStep * 0.01, velocityIterations, positionIterations );
 }
 void UpdateGraphics() {
  //list all bodies and draw them 
  //unsigned int vertices_count;
  for( typename std::list<ifCB2Body*>::iterator iter = Bodies.begin(); iter != Bodies.end(); iter++ ){
   
   //get body position and rotation
   b2Vec2 position = (*iter)->body->GetPosition();
   float32 angle = (*iter)->body->GetAngle();
  // b2Transform b2trans2x2 = (*iter)->body->GetTransform();
   
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glRotatef(angle, 0, 0, 1);
   glScalef(IFA_box2D_factor, IFA_box2D_factor, 1.0f);
   glTranslatef(position.x, position.y, 0.0);
   //   GLfloat body_matrix[16];
   //LoadIdentityMatrix(body_matrix);
   //b2trans2x2.p.x;
   //b2trans2x2.p.y;
   //b2trans2x2.q.c;
   //b2trans2x2.q.s;
   //glLoadMatrixf(body_matrix);   
   glGetFloatv(GL_MODELVIEW_MATRIX, (*iter)->OGL_body->modelview_matrix);
   //printf("%4.2f %4.2f %4.2f\n", position.x, position.y, angle);

  }
  //call game update and draw functions
 }
 //Calculates conversion factor when presenting Box2D to graphic unit
 // pixels_per_box2d_unit is desired number of pixels on screen for delta Box coordinate 1. If we want 1 pixels for 0.1 box coord delta( one coordinate unit) we pass 10
 //We use smaller dimension of screen resolution as one unit of screen length 
 void CalculateBox2DSizeFactor(float pixels_per_box2d_unit = 10) {
  unsigned int ScreenPixels;
  if (screenResolutionX < screenResolutionY) {
   ScreenPixels = screenResolutionX;
  }
  else {
   ScreenPixels = screenResolutionY;
  }
  //Part of screen used by 1 box2d unit
  IFA_box2D_factor = 2.0 * ( pixels_per_box2d_unit / (float)ScreenPixels );
 }
 //This function transforms box2d coordinate to opengl es normalized coordinate where -1 is 0 on screen coordinates and 1 is the smaller of the resolutions
 float Coord_Box2D2OpenGLCoord(float _input_coord) {
  return _input_coord * IFA_box2D_factor - 1.0f;
 }
 //This function will transform box2d coordinates to absolute size in normalized opengl es coordinates
 float Size_Box2D2OpenGL(float _input_coord) {
  return _input_coord * IFA_box2D_factor;
 }
 float32 timeStep;
 int32 velocityIterations;   //how strongly to correct velocity
 int32 positionIterations;   //how strongly to correct position
 //Variables that determine scaling of box2d objects to our screen
 unsigned int screenResolutionX, screenResolutionY;//OpenGLES cooridinates are in range from -1 to 1 for width and heigth, where -1 is 0 in screen coordinates and 1 is las r/c pixel
 //b2World *World;
 struct timespec temp_timespec, game_time_0;
 int64_t temp_int64;
};


class ifCB2WorldManager :  public ifCB2GameManager {
public:
 ifCB2WorldManager() {
  //Gravity = NULL;
  IFA_World = NULL;
 }
 ~ifCB2WorldManager() {
  Free();
 }
 void Init() {
  Free();
 }
 void MakeWorld(float32 gx, float32 gy) {
  ifCB2BodyManager::Init();
  Init();
  Init_IFEngine();
  //Gravity = new b2Vec2(gx,gy);
  IFA_World = new b2World(b2Vec2(gx, gy));
 }
 void Free() {
  //IF_NULL_DELETE(Gravity)
  IF_NULL_DELETE(IFA_World)
 }
public:
};



class ifCB2Adapter : public ifCB2WorldManager {
public:
 ifCB2Adapter() {

 }
 void MakeWorld(float32 gx, float32 gy) {
  ifCB2WorldManager::MakeWorld(gx, gy);
 }

};
