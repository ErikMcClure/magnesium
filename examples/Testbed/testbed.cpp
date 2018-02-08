/* Magnesium Testbed
* -------------------------
* This example runs a series of tests to ensure Magnesium is
* working correctly
*
* Copyright ©2018 Black Sphere Studios
* For conditions of distribution and use, see copyright notice in Magnesium.h
*/

#include "magnesium/mgEngine.h"
#include "magnesium/mgPlaneshader.h"
#include "magnesium/mgTinyOAL.h"
#include "magnesium/mgJoinPSB2.h"
#include "magnesium/mgTimeline.h"
#include "planeshader/psLayer.h"
#include "planeshader/psSolid.h"
#include "planeshader/psFont.h"
#include "Box2D/Dynamics/b2Fixture.h"
#include "Box2D/Collision/Shapes/b2PolygonShape.h"

using namespace bss;
using namespace magnesium;
using namespace planeshader;
using namespace tinyoal;

typedef AniDataSmooth<uint32_t> ColorAniData;
typedef Animation<PositionAniData, RotationAniData, PivotAniData, ScaleAniData, ColorAniData> fxAnimation;

typedef AniState<psText, fxAnimation,
  AniStateSmooth<psLocatable, PositionAniData, const Vector<float, 3>&, &psLocatable::SetPosition>,
  AniStateSmooth<psLocatable, RotationAniData, float, &psLocatable::SetRotation>,
  AniStateSmooth<psLocatable, PivotAniData, const Vector<float, 2>&, &psLocatable::SetPivot>,
  AniStateSmooth<psSolid, ScaleAniData, const Vector<float, 2>&, &psSolid::SetScale>,
  AniStateSmooth<psText, ColorAniData, psColor32, &psText::SetColor>> TextAniState;

typedef AniState<psImage, fxAnimation,
  AniStateSmooth<psLocatable, PositionAniData, const Vector<float, 3>&, &psLocatable::SetPosition>,
  AniStateSmooth<psLocatable, RotationAniData, float, &psLocatable::SetRotation>,
  AniStateSmooth<psLocatable, PivotAniData, const Vector<float, 2>&, &psLocatable::SetPivot>,
  AniStateSmooth<psSolid, ScaleAniData, const Vector<float, 2>&, &psSolid::SetScale>,
  AniStateSmooth<psImage, ColorAniData, psColor32, &psImage::SetColor>> ImageAniState;

typedef mgTimeline<fxAnimation> Timeline;
typedef mgEffect<Timeline, TextAniState, ImageAniState> Effect;

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

std::function<size_t(fgRoot& self, const FG_Msg& evt)> guifunction;

int main(int argc, char** argv)
{
  SetWorkDirToCur();
  mgEngine engine;
  PSINIT init;
  init.width = 1024;
  init.height = 768;
  Box2DSystem::B2INIT b2init;
  PlaneshaderBox2DSystem ps(init, 0);
  TinyOALSystem tinyoal;
  Box2DSystem psbox2d(b2init, -1);

  engine.AddSystem(&ps);
  engine.AddSystem(&tinyoal);
  engine.AddSystem(&psbox2d);

  psDebugDraw psdd;
  ps[0].Insert(&psdd);
  psbox2d.SetDebugDraw(&psdd);

  globalcam.SetExtent(psVec(0.2, 50000));
  ps[0].SetCamera(&globalcam);
  ps[0].SetClearColor(0xFF555566);
  psFont* DEBUGFONT = psFont::Create("arial", 500, false, 14);

  fgInject injector;
  guifunction = [&](fgRoot& self, const FG_Msg& evt) -> size_t {
    if(evt.type == FG_KEYDOWN || evt.type == FG_KEYUP)
    {
      bool isdown = evt.type == FG_KEYDOWN;
      dirkeys[8] = evt.IsShiftDown();
      switch(evt.keycode)
      {
      case FG_KEY_A: dirkeys[0] = isdown; break;
      case FG_KEY_D: dirkeys[1] = isdown; break;
      case FG_KEY_W: dirkeys[2] = isdown; break;
      case FG_KEY_S: dirkeys[3] = isdown; break;
      case FG_KEY_X:
      case FG_KEY_OEM_PLUS: dirkeys[4] = isdown; break;
      case FG_KEY_Z:
      case FG_KEY_OEM_MINUS: dirkeys[5] = isdown; break;
      case FG_KEY_Q: dirkeys[6] = isdown; break;
      case FG_KEY_E: dirkeys[7] = isdown; break;
      case FG_KEY_ESCAPE:
        if(isdown) ps.Quit();
        break;
      }
    }
    return false;
    return injector(&self, &evt);
  };

  injector = fgSetInjectFunc([](struct _FG_ROOT* self, const FG_Msg* msg) -> size_t { return guifunction(*self, *msg); });

  // Define the ground body.
  b2BodyDef groundBodyDef;
  groundBodyDef.position.Set(0.0f, 6.0f);

  // Call the body factory which allocates memory for the ground body
  // from a pool and creates the ground box shape (also from a pool).
  // The body is also added to the world.
  mgEntityT<b2Component> groundBody;
  groundBody.Get<b2Component>()->Init(groundBodyDef);

  // Define the ground box shape.
  b2PolygonShape groundBox;

  // The extents are the half-widths of the box.
  groundBox.SetAsBox(50.0f, 2.0f);
  
  // Add the ground fixture to the ground body.
  groundBody.Get<b2Component>()->GetBody()->CreateFixture(&groundBox, 0.0f);

  // Define the dynamic body. We set its position and call the body factory.
  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;
  bodyDef.position.Set(4.0f, 0.0f);
  bodyDef.angularVelocity = 0.3f;
  mgEntityT<psImageComponent, b2Component> mbody;
  mbody.Get<b2Component>()->Init(bodyDef);

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
  mbody.Get<b2Component>()->GetBody()->CreateFixture(&fixtureDef);
  mbody.Get<psImageComponent>()->SetTexture(psTex::Create("../media/blendtest.png"));
  mbody.Get<psImageComponent>()->SetPass();

  mgEntityT<psImageComponent, b2Component> test;
  b2BodyDef bd;
  test.Get<b2Component>()->Init(bd);
  mgEntityT<psImageComponent> tests[100];
  tests[2].SetOrder(-1);

  AudioResource* songref = AudioResource::Create("../media/idea803.ogg", 0);
  Audio song(songref);

  Timeline t1;
  Timeline t2;
  std::swap(t1, t2);

  mbody.Get<b2Component>()->SetUserData(&song);
  mbody.Get<b2Component>()->SetContactResponse([](b2Component& c, b2Contact*, b2Fixture*, bool b) {
    if(b)
      static_cast<Audio*>(c.GetUserData())->Play();
  });

  Timeline timeline;
  Effect effect(&timeline);
  effect.Interpolate(0.0f);
  effect.Add<TextAniState, psTexFont*, const char*>(0, DEBUGFONT, "test");

  //fixref. = [&song](b2CompoundFixture* f, b2Contact* c) {
  //  if(c) song.Play();
  //};

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