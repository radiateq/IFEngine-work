#pragma once

#include <Box2D/Box2D.h>
#include <list>


class Iifclass1 {
public:
 ~Iifclass1() {
  Free();
 }
 void Init() {
  Free();
 }
private:
 void Free(){
 }
 };


class ifCB2GameManager {
public:
 ifCB2GameManager() {
  velocityIterations = 8;   //how strongly to correct velocity
  positionIterations = 3;   //how strongly to correct position

  ResetClock();
 }
 void ResetClock() {
  clock_gettime(CLOCK_MONOTONIC, &temp_timespec);
  game_time_0 = temp_timespec;
 }
 void UpdateSim() {
  clock_gettime(CLOCK_MONOTONIC, &temp_timespec);
  //temp_int64 = timespec2ms64(&temp_timespec) - timespec2ms64(&game_time_0);
  temp_int64 = timespec2us64(&temp_timespec) - timespec2us64(&game_time_0);
  game_time_0 = temp_timespec;
  timeStep = 1000.0f / (float)temp_int64;
  //timeThen = timeNow;
  World->Step(timeStep, velocityIterations, positionIterations);
 }
 void UpdateGraphics() {
  //list all bodies and draw them 
 }
 float32 timeStep;
 int32 velocityIterations;   //how strongly to correct velocity
 int32 positionIterations;   //how strongly to correct position
protected:
 b2Vec2 *Gravity;
 b2World *World;
 struct timespec temp_timespec, game_time_0;
 int64_t temp_int64;
};


class ifCB2WorldManager : public Iifclass1, public ifCB2GameManager {
public:
 ifCB2WorldManager() {
  Gravity = NULL;
  World = NULL;
 }
 ~ifCB2WorldManager() {
  Free();
 }
 void MakeWorld(float32 gx, float32 gy) {
  Init();
  Gravity = new b2Vec2(gx,gy);
  World = new b2World(*Gravity);
 }
 void Free() {
  IF_NULL_DELETE(Gravity)
  IF_NULL_DELETE(World)
 }
public:
};


class ifCB2Body : Iifclass1 {
public:
 ifCB2Body( b2World *_World = NULL) {
  World = _World;
  body_def = NULL;
  body = NULL;
 }
 ~ifCB2Body() {
  Free();
 }
 b2World *World;
 ////////////////////////////////////
 b2BodyDef *body_def;
 b2Body *body;
 S_listWrap_ptr<b2Shape, true> shape;
 std::list<b2FixtureDef> fixture;
 ////////////////////////////////////
 //After body_def, body, shape and fixture have been prepared this function will create a body and add it to the world 
 void CreateBody(){
  if(World&&!body){
   body = World->CreateBody(body_def);
   typename std::list<b2Shape*>::iterator iters = shape.begin();
   for( typename std::list<b2FixtureDef>::iterator iter = fixture.begin();
    iter != fixture.end();
    iter++){
    (*iter).shape = (*iters);
    body->CreateFixture(&(*iter));
    iters++;         
   }
  }
 }
 void Free() {
  if( World ){
   World->DestroyBody(body);
  }
  body = NULL;
  body_def = NULL;
  shape.clear();
  fixture.clear();
 }
};

class ifCB2BodyManager : public Iifclass1 {
public:
 ifCB2BodyManager() {
  World = NULL;
 }
 ~ifCB2BodyManager() {
  Free();
 }
private:
 b2World *World;
 S_listWrap_ptr<ifCB2Body> Bodies; 
 void Free() {
 }
public:

};





class ifCB2Adapter : public ifCB2WorldManager {
public:
 ifCB2Adapter() {

 }
 b2Vec2 *Gravity;
 b2World *World;

};



// Define the gravity vector.
//b2Vec2 gravity;//(2.0f, -2.0f);

// Construct a world object, which will hold and simulate the rigid bodies.
//b2World world;//(gravity);

