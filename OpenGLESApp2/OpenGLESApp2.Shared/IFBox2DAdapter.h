#pragma once

#include <Box2D/Box2D.h>


class ifCBox2DAdapter {
public:
 
};

class ifCBox2WorldManager {
public:
 ifCBox2WorldManager() {
  Gravity = NULL;
  World = NULL;
 }
 ~ifCBox2WorldManager() {
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
 void Init() {
  Free();
 } 
 void Free() {
  IF_NULL_DELETE(Gravity)
  IF_NULL_DELETE(World)
 }
public:
};

class ifCBox2DBodyManager {
public:



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


};
class ifCBox2DGameLoop {

};

// Define the gravity vector.
//b2Vec2 gravity;//(2.0f, -2.0f);

// Construct a world object, which will hold and simulate the rigid bodies.
//b2World world;//(gravity);

