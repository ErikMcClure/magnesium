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
#include "planeshader/psPass.h"

using namespace bss_util;
using namespace magnesium;
using namespace planeshader;
using namespace tinyoal;

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
#pragma comment(lib, "../../lib/TinyOAL" BSS_LIB32 BSS_LIBDEBUG  ".lib")
#endif

psCamera globalcam;
bool dirkeys[9] = { false }; // left right up down in out counterclockwise clockwise shift
float dirspeeds[4] = { 600.0f, 600.0f, 2.0f, 1.0f }; // X Y Z R

void processGUI()
{
  float secdelta = mgEngine::Instance()->GetDeltaNS() / 1000000000.0f;
  float scale = dirkeys[8] ? 0.01f : 1.0f;
  if(dirkeys[0])
    globalcam.SetPositionX(globalcam.GetPosition().x - dirspeeds[0] * secdelta * scale);
  if(dirkeys[1])
    globalcam.SetPositionX(globalcam.GetPosition().x + dirspeeds[0] * secdelta * scale);
  if(dirkeys[2])
    globalcam.SetPositionY(globalcam.GetPosition().y - dirspeeds[1] * secdelta * scale);
  if(dirkeys[3])
    globalcam.SetPositionY(globalcam.GetPosition().y + dirspeeds[1] * secdelta * scale);
  if(dirkeys[4])
    globalcam.SetPositionZ(globalcam.GetPosition().z + dirspeeds[2] * secdelta * scale);
  if(dirkeys[5])
    globalcam.SetPositionZ(globalcam.GetPosition().z - dirspeeds[2] * secdelta * scale);
  if(dirkeys[6])
    globalcam.SetRotation(globalcam.GetRotation() + dirspeeds[3] * secdelta * scale);
  if(dirkeys[7])
    globalcam.SetRotation(globalcam.GetRotation() - dirspeeds[3] * secdelta * scale);
}

int main(int argc, char** argv)
{
  SetWorkDirToCur();
  PSINIT init;
  init.width = 1024;
  init.height = 768;
  init.mediapath = "../media/shaders";
  Box2DSystem::B2INIT b2init;
  PlaneshaderSystem ps(init, 0);
  TinyOALSystem tinyoal;
  LiquidFunPlaneshaderSystem psbox2d(b2init, -1);

  mgEngine engine;
  engine.AddSystem(&ps);
  engine.AddSystem(&tinyoal);
  engine.AddSystem(&psbox2d);

  psDebugDraw psdd;
  ps[0].Insert(&psdd);
  psbox2d.SetDebugDraw(&psdd);

  globalcam.SetExtent(psVec(0.2, 50000));
  ps[0].SetCamera(&globalcam);
  ps[0].SetClearColor(0xFF555566);

  std::function<bool(const psGUIEvent&)> guicallback = [&](const psGUIEvent& evt) -> bool
  {
    if(evt.type == GUI_KEYDOWN || evt.type == GUI_KEYUP)
    {
      bool isdown = evt.type == GUI_KEYDOWN;
      dirkeys[8] = evt.IsShiftDown();
      switch(evt.keycode)
      {
      case KEY_A: dirkeys[0] = isdown; break;
      case KEY_D: dirkeys[1] = isdown; break;
      case KEY_W: dirkeys[2] = isdown; break;
      case KEY_S: dirkeys[3] = isdown; break;
      case KEY_X:
      case KEY_OEM_PLUS: dirkeys[4] = isdown; break;
      case KEY_Z:
      case KEY_OEM_MINUS: dirkeys[5] = isdown; break;
      case KEY_Q: dirkeys[6] = isdown; break;
      case KEY_E: dirkeys[7] = isdown; break;
      case KEY_ESCAPE:
        if(isdown) ps.Quit();
        break;
      }
    }
    return false;
  };
  ps.SetInputReceiver(guicallback);

  // Define the ground body.
  b2BodyDef groundBodyDef;
  groundBodyDef.position.Set(0.0f, 6.0f);

  // Call the body factory which allocates memory for the ground body
  // from a pool and creates the ground box shape (also from a pool).
  // The body is also added to the world.
  mgEntityT<b2PhysicsComponent> groundBody;
  groundBody.Get<b2PhysicsComponent>()->Init(groundBodyDef);

  // Define the ground box shape.
  b2PolygonShape groundBox;

  // The extents are the half-widths of the box.
  groundBox.SetAsBox(50.0f, 2.0f);
  
  // Add the ground fixture to the ground body.
  groundBody.Get<b2PhysicsComponent>()->AddCompoundFixture(groundBox, 0.0f);

  // Define the dynamic body. We set its position and call the body factory.
  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;
  bodyDef.position.Set(4.0f, 0.0f);
  bodyDef.angularVelocity = 0.3f;
  mgEntityT<psImageComponent, b2PhysicsComponent> mbody;
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
  auto& fixref = mbody.Get<b2PhysicsComponent>()->AddCompoundFixture(fixtureDef);
  mbody.Get<psImageComponent>()->SetTexture(psTex::Create("../media/blendtest.png"));
  mbody.Get<psImageComponent>()->SetPass();

  mgEntityT<psImageComponent, b2PhysicsComponent> test;
  b2BodyDef bd;
  test.Get<b2PhysicsComponent>()->Init(bd);
  mgEntityT<psImageComponent> test2;
  mgEntityT<psImageComponent> test3;

  cAudioResource* songref = cAudioResource::Create("../media/idea803.ogg", 0);
  cAudio song(songref);

  fixref.rsp = [&song](b2CompoundFixture* f, b2Contact* c) {
    if(c) song.Play();
  };

  engine.ResetDelta();
  engine.ResetTime();
  while(!ps.GetQuit())
  {
    processGUI();
    engine.Process();
  }
}

struct HINSTANCE__;

// WinMain function, simply a catcher that calls the main function
int __stdcall WinMain(HINSTANCE__* hInstance, HINSTANCE__* hPrevInstance, char* lpCmdLine, int nShowCmd)
{
  main(0, (char**)hInstance);
}