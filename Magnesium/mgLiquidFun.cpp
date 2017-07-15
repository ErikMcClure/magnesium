// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgLiquidFun.h"
#include "mgEngine.h"
#include "Box2D/Box2D.h"

using namespace magnesium;

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
void LiquidFunSystem::Process()
{
  if(!_particle_iters) // if this is zero we revert to box2D behavior
    return Box2DSystem::Process();
  assert(false);
  //if(_frozen || !_world)
  //  return;
  //double delta = _getphysdelta();

  //if(delta > 0.0) // This technique lets us do both variable and fixed-step updates.
  //{
  //  _world->Step(delta, _init.vel_iters, _init.pos_iters, _particle_iters);
  //  _world->ClearForces();
  //  _world->DrawDebugData();
  //}
}