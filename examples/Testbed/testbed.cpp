/* Magnesium Testbed
* -------------------------
* This example runs a series of tests to ensure Magnesium is
* working correctly
*
* Copyright ©2016 Black Sphere Studios
*/

#include "Magnesium.h"
#include "mgPlaneshader.h"
#include "mgTinyOAL.h"
#include "mgJoinPSB2.h"
#include "Box2D/Box2D.h"

using namespace bss_util;
using namespace magnesium;
using namespace planeshader;

#ifdef BSS_COMPILER_MSC
#ifdef BSS_DEBUG
#define BSS_LIBDEBUG "_d"
#else
#define BSS_LIBDEBUG 
#endif
#ifndef BSS_CPU_x86_64
#define BSS_LIB32 "32"
#else
#define BSS_LIB32 
#endif

#ifdef MG_STATIC_LIB
#pragma comment(lib, "../../bin/Magnesium" BSS_LIB32 "_s" BSS_LIBDEBUG ".lib")
#else
#pragma comment(lib, "../../bin/Magnesium" BSS_LIB32 BSS_LIBDEBUG ".lib")
#endif
#pragma comment(lib, "../../lib/PlaneShader" BSS_LIB32 BSS_LIBDEBUG  ".lib")
#pragma comment(lib, "../../lib/bss-util" BSS_LIB32 BSS_LIBDEBUG  ".lib")
#pragma comment(lib, "../../lib/liquidfun" BSS_LIB32 BSS_LIBDEBUG  ".lib")
#endif

int main(int argc, char** argv)
{
  SetWorkDirToCur();
  PSINIT init;
  init.width = 1024;
  init.height = 768;
  init.mediapath = "../media/shaders";
  Box2DSystem::B2INIT b2init;
  PlaneshaderSystem ps(init);
  TinyOALSystem tinyoal;
  LiquidFunPlaneshaderSystem psbox2d(b2init);

  mgEngine engine;
  engine.AddSystem(&ps);
  engine.AddSystem(&tinyoal);
  engine.AddSystem(&psbox2d);


  // Define the ground body.
  b2BodyDef groundBodyDef;
  groundBodyDef.position.Set(0.0f, -10.0f);

  // Call the body factory which allocates memory for the ground body
  // from a pool and creates the ground box shape (also from a pool).
  // The body is also added to the world.
  mgEntityT<b2PhysicsComponent> groundBody;
  groundBody.Get<b2PhysicsComponent>()->Init(groundBodyDef);

  // Define the ground box shape.
  b2PolygonShape groundBox;

  // The extents are the half-widths of the box.
  groundBox.SetAsBox(50.0f, 10.0f);

  // Add the ground fixture to the ground body.
  groundBody.Get<b2PhysicsComponent>()->AddCompoundFixture(groundBox, 0.0f);

  // Define the dynamic body. We set its position and call the body factory.
  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;
  bodyDef.position.Set(0.0f, 4.0f);
  mgEntityT<b2PhysicsComponent> mbody;
  mbody.Get<b2PhysicsComponent>()->Init(bodyDef);

  // Define another box shape for our dynamic body.
  b2PolygonShape dynamicBox;
  dynamicBox.SetAsBox(1.0f, 1.0f);

  // Define the dynamic body fixture.
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &dynamicBox;

  // Set the box density to be non-zero, so it will be dynamic.
  fixtureDef.density = 1.0f;

  // Override the default friction.
  fixtureDef.friction = 0.3f;

  // Add the shape to the body.
  mbody.Get<b2PhysicsComponent>()->AddCompoundFixture(fixtureDef);

  mgEntityT<psSolidComponent, psImageComponent, b2PhysicsComponent> test;
  b2BodyDef bd;
  test.Get<b2PhysicsComponent>()->Init(bd);

  while(!ps.GetQuit())
  {
    engine.Process();
  }
}

struct HINSTANCE__;

// WinMain function, simply a catcher that calls the main function
int __stdcall WinMain(HINSTANCE__* hInstance, HINSTANCE__* hPrevInstance, char* lpCmdLine, int nShowCmd)
{
  main(0, (char**)hInstance);
}