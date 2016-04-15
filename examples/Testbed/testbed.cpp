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

using namespace bss_util;
using namespace magnesium;
using namespace planeshader;

#ifdef BSS_COMPILER_MSC
#if defined(BSS_DEBUG) && defined(BSS_CPU_x86_64)
#pragma comment(lib, "../../bin/Magnesium_d.lib")
#pragma comment(lib, "../../lib/PlaneShader_d.lib")
#pragma comment(lib, "../../lib/bss-util_d.lib")
#elif defined(BSS_CPU_x86_64)
#pragma comment(lib, "../../bin/Magnesium.lib")
#pragma comment(lib, "../../lib/PlaneShader.lib")
#pragma comment(lib, "../../lib/bss-util.lib")
#elif defined(BSS_DEBUG)
#pragma comment(lib, "../../bin32/Magnesium32_d.lib")
#pragma comment(lib, "../../lib/PlaneShader32_d.lib")
#pragma comment(lib, "../../lib/bss-util32_d.lib")
#else
#pragma comment(lib, "../../bin32/Magnesium32.lib")
#pragma comment(lib, "../../lib/PlaneShader32.lib")
#pragma comment(lib, "../../lib/bss-util32.lib")
#endif
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