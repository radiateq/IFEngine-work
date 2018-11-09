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
 virtual void Free() = 0;
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
  body_def = NULL;
  body = NULL;
 }
 ~ifCB2Body() {
  Free();
 }
 b2BodyDef *body_def;
 b2Body *body;
 std::list<b2FixtureDef> shape;
 std::list<b2FixtureDef> fixture;
 void Free() {
  World
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




  // Define the ground body.
  b2BodyDef groundBodyDef;
  groundBodyDef.position.Set(0.0f, -10.0f);

  // Call the body factory which allocates memory for the ground body
  // from a pool and creates the ground box shape (also from a pool).
  // The body is also added to the world.
  b2Body* groundBody = world.CreateBody(&groundBodyDef);

  // Define the ground box shape.
  b2PolygonShape groundBox;

  // The extents are the half-widths of the box.
  groundBox.SetAsBox(50.0f, 10.0f);

  // Add the ground fixture to the ground body.
  groundBody->CreateFixture(&groundBox, 0.0f);









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



 b2Vec2 *Gravity;
 b2World *World;
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

