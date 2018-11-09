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

class ifCB2WorldManager : Iifclass1 {
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
private:
 b2Vec2 *Gravity;
 b2World *World;
 void Free() {
  IF_NULL_DELETE(Gravity)
  IF_NULL_DELETE(World)
 }
public:
};


class ifCB2Body : Iifclass1 {
public:
 ifCB2Body() {
  World = NULL;
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

class ifCB2BodyManager {
public:
 ifCB2BodyManager() {
  B2WorldManager = NULL;






 }
 ~ifCB2BodyManager() {
  Free();
 }
 void MakeWorld(ifCB2WorldManager *_B2WorldManager) {
  Init();
  B2WorldManager = _B2WorldManager;
 }
private:
 ifCB2WorldManager *B2WorldManager;
 ifCB2Body
 void Init() {
  Free();
  
 }
 void Free() {  
  IF_NULL_DELETE(Gravity)
  IF_NULL_DELETE(World)
 }
public:

};
class ifCB2GameLoop {

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

