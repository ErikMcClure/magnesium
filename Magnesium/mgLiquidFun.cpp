// Copyright �2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgLiquidFun.h"
#include "mgEngine.h"
#include "Box2D/Box2D.h"

using namespace magnesium;

#ifdef BSS_COMPILER_MSC
#if defined(BSS_DEBUG) && defined(BSS_CPU_x86_64)
#pragma comment(lib, "../lib/liquidfun_d.lib")
#elif defined(BSS_CPU_x86_64)
#pragma comment(lib, "../lib/liquidfun.lib")
#elif defined(BSS_DEBUG)
#pragma comment(lib, "../lib/liquidfun32_d.lib")
#else
#pragma comment(lib, "../lib/liquidfun32.lib")
#endif
#endif

LiquidFunSystem::LiquidFunSystem(const LFINIT& init, int priority) : Box2DSystem(init.b2init, priority), _particle_iters(init.particle_iters)
{
  Reload();
}
LiquidFunSystem::LiquidFunSystem(const B2INIT& init, int priority) : Box2DSystem(init, priority), _particle_iters(0)
{
}
LiquidFunSystem::~LiquidFunSystem()
{
  b2PhysicsComponent::Store().Clear(); // Clear all components out BEFORE we delete the world, otherwise things get mad.
  Unload();
}
void LiquidFunSystem::Preprocess()
{
  if(!_particle_iters) // if this is zero we revert to box2D behavior
    return Box2DSystem::Preprocess();

  if(_frozen || !_world)
    return;
  double delta = _getphysdelta();

  if(delta > 0.0) // This technique lets us do both variable and fixed-step updates.
  {
    _world->Step(delta, _init.vel_iters, _init.pos_iters, _particle_iters);
    _world->ClearForces();
    _world->DrawDebugData();
  }
}