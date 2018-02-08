// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgBox2D.h"
#include "mgEngine.h"
#include "Box2D/Box2D.h"

using namespace magnesium;

Box2DSystem* Box2DSystem::_instance = 0;
static_assert(!std::is_polymorphic<b2Component>::value, "Components shouldn't have a vtable");

b2Component::b2Component(b2Component&& mov) : mgComponent(std::move(mov)), _body(mov._body), _userdata(mov._body), _rp(mov._rp), _rc(mov._rc), _oldangle(0)
{ 
  mov._body = 0;
  mov._userdata = 0;
}
b2Component::b2Component(mgEntity* e) : mgComponent(e), _body(0), _userdata(0), _rp(0), _rc(0)
{
  if(e)
  {
    Event<EVENT_SETPOSITION>::Register<b2Component, &b2Component::SetPosition>(e);
    Event<EVENT_SETROTATION>::Register<b2Component, &b2Component::SetRotation>(e);
  }
}

b2Component::~b2Component() { _destruct(); }
void b2Component::_destruct()
{
  if(_body)
    _body->GetWorld()->DestroyBody(_body);
  _body = 0;
}

void b2Component::_setInterpolation()
{
  b2Vec2 p = _body->GetPosition();
  p *= Box2DSystem::Instance()->F_PPM;
  Event<EVENT_SETPOSITION_INTERPOLATE>::Send(entity, p.x, p.y);
  Event<EVENT_SETROTATION>::Send(entity, _body->GetAngle());
    /*if(auto loc = entity->Get<psLocatableComponent>())
    loc->Get()->SetPosition(PlaneshaderBox2DSystem::toVec());*/
}
void b2Component::Init(const b2BodyDef& def)
{ 
  _body = Box2DSystem::Instance()->GetWorld()->CreateBody(&def);
  _userdata = def.userData;
  _body->SetUserData(entity);
  _setInterpolation();
  UpdateOld();
}
b2Vec2 b2Component::GetPosition() const
{
  b2Vec2 p = _body->GetPosition();
  p *= Box2DSystem::Instance()->F_PPM;
  return p;
}
b2Vec2 b2Component::GetOldPosition() const
{
  b2Vec2 p = _oldposition;
  p *= Box2DSystem::Instance()->F_PPM;
  return p;
}

void b2Component::SetTransform(const b2Vec2& pos, float rotation)
{
  assert(_body);
  b2Vec2 p(pos);
  p *= Box2DSystem::Instance()->INV_PPM;
  _body->SetTransform(p, rotation);
  _setInterpolation();
  UpdateOld();
}

b2Component& b2Component::operator =(b2Component&& right)
{ 
  _destruct();
  _body = right._body;
  _userdata = right._userdata;
  right._body = 0;
  right._userdata = 0;
  return *this;
}

Box2DSystem::Box2DSystem(const B2INIT& init, int priority) : mgSystem(priority), _totaldelta(0), _dt(0), _world(0), _init(init), _debugdraw(0),
  _frozen(false), F_PPM(init.ppm), INV_PPM(1.0f/ init.ppm), _ratio(1.0)
{
  SetHertz(_init.hertz);
  _instance = this;
  Reload();
}
Box2DSystem::~Box2DSystem()
{
  b2Component::Store().Clear(); // Clear all components out BEFORE we delete the world, otherwise things get mad.
  _instance = 0;
  Unload();
}
void Box2DSystem::Process()
{
  if(!_world)
    return;

  double delta = _frozen ? 0.0 : (mgEngine::Instance()->GetDelta() / 1000.0);

  if(_init.hertz == 0.0) // Variable technique
  {
    _totaldelta += bssmin(1.0, delta);

    if(!bss::fSmall(_totaldelta))
    {
      if(_debugdraw)
        _debugdraw->Clear();
#ifdef b2_invalidParticleIndex
      _world->Step(_totaldelta, _init.vel_iters, _init.pos_iters, _init.particle_iters);
#else
      _world->Step(_totaldelta, _init.vel_iters, _init.pos_iters);
#endif
      _world->DrawDebugData();
      _totaldelta = 0.0;
      _ratio = 1.0;
    }
  }
  else if(_init.hertz > 0.0) // Substep technique
  {
    const int MAX_STEPS = 5;
    _totaldelta += delta;
    int steps = static_cast<int>(std::floor(_totaldelta / _dt));

    if(steps > 0)
      _totaldelta -= steps * _dt;
    assert(_totaldelta < _dt + FLT_EPSILON);
    _ratio = _totaldelta / _dt;

    if(steps > MAX_STEPS)
      steps = MAX_STEPS;

    if(_debugdraw)
      _debugdraw->Clear();

    if(steps > 0)
      _updateOld();

    for(int i = 0; i < steps; ++i)
    {
#ifdef b2_invalidParticleIndex
      _world->Step(_dt, _init.vel_iters, _init.pos_iters, _init.particle_iters);
#else
      _world->Step(_dt, _init.vel_iters, _init.pos_iters);
#endif
    }
    _world->DrawDebugData();
  }
  
  _world->ClearForces();
}
Box2DSystem* Box2DSystem::Instance() { return _instance; }

void Box2DSystem::_updateOld()
{
  for(b2Body * b = _world->GetBodyList(); b != NULL; b = b->GetNext())
  {
    if(b->GetUserData())
      reinterpret_cast<mgEntity*>(b->GetUserData())->Get<b2Component>()->UpdateOld();
  }
}

void Box2DSystem::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
  if(contact->IsEnabled())
  {
    mgEntity* a = reinterpret_cast<mgEntity*>(contact->GetFixtureA()->GetBody()->GetUserData());
    mgEntity* b = reinterpret_cast<mgEntity*>(contact->GetFixtureB()->GetBody()->GetUserData());
    if(a || b)
    {
      b2Component* ac = !a ? 0 : a->Get<b2Component>();
      b2Component* bc = !b ? 0 : b->Get<b2Component>();
      b2Component::PointResponse af = !ac ? 0 : ac->GetPointResponse();
      b2Component::PointResponse bf = !bc ? 0 : bc->GetPointResponse();
      if(af || bf)
      {
        b2PointState s1[b2_maxManifoldPoints];
        b2PointState s2[b2_maxManifoldPoints];
        b2GetPointStates(s1, s2, oldManifold, contact->GetManifold());
        b2WorldManifold m;
        contact->GetWorldManifold(&m);

        for(int i = 0; i < b2_maxManifoldPoints; ++i)
          if(s1[i] == b2_removeState)
          {
            if(af) af(*ac, b2Component::ContactPoint{ contact->GetFixtureA(), contact->GetFixtureB(), oldManifold->points[i].localPoint, m.normal, b2_removeState });
            if(bf) bf(*bc, b2Component::ContactPoint{ contact->GetFixtureB(), contact->GetFixtureA(), oldManifold->points[i].localPoint, m.normal, b2_removeState });
          }

        for(int i = 0; i < b2_maxManifoldPoints; ++i)
          if(s2[i] != b2_nullState)
          {
            assert(s2[i] != b2_removeState);
            if(af) af(*ac, b2Component::ContactPoint{ contact->GetFixtureA(), contact->GetFixtureB(), m.points[i], m.normal, s2[i] });
            if(bf) bf(*bc, b2Component::ContactPoint{ contact->GetFixtureB(), contact->GetFixtureA(), m.points[i], m.normal, s2[i] });
          }
      }
    }
  }
}
void Box2DSystem::ProcessContact(mgEntity& entity, b2Contact* contact, b2Fixture* other, bool begin)
{
  if(b2Component* c = entity.Get<b2Component>())
    if(b2Component::ContactResponse cr = c->GetContactResponse())
      cr(*c, contact, other, begin);
}
void Box2DSystem::BeginContact(b2Contact* contact)
{
  mgEntity* a = reinterpret_cast<mgEntity*>(contact->GetFixtureA()->GetBody()->GetUserData());
  mgEntity* b = reinterpret_cast<mgEntity*>(contact->GetFixtureB()->GetBody()->GetUserData());
  if(a)
    ProcessContact(*a, contact, contact->GetFixtureB(), true);
  if(b)
    ProcessContact(*b, contact, contact->GetFixtureA(), true);
}
void Box2DSystem::EndContact(b2Contact* contact)
{
  mgEntity* a = reinterpret_cast<mgEntity*>(contact->GetFixtureA()->GetBody()->GetUserData());
  mgEntity* b = reinterpret_cast<mgEntity*>(contact->GetFixtureB()->GetBody()->GetUserData());
  if(a)
    ProcessContact(*a, contact, contact->GetFixtureB(), false);
  if(b)
    ProcessContact(*b, contact, contact->GetFixtureA(), false);
}

void Box2DSystem::SetHertz(double hertz)
{
  _dt = (hertz == 0.0) ? 0.0 : (1.0 / fabs(hertz));
  _init.hertz = hertz;
}
inline b2Vec2 Box2DSystem::GetGravity() { return !_world ? b2Vec2(_init.gravity[0], _init.gravity[1]) : _world->GetGravity(); }
inline void Box2DSystem::SetGravity(const b2Vec2& gravity)
{
  _init.gravity[0] = gravity.x;
  _init.gravity[1] = gravity.y;
  if(_world)
    _world->SetGravity(gravity);
}
inline void Box2DSystem::SetDebugDraw(mgDebugDraw* debugdraw)
{
  _debugdraw = debugdraw;
  if(_world)
    _world->SetDebugDraw(_debugdraw);
}

void Box2DSystem::Reload()
{
  Unload();
  _world = new b2World(b2Vec2(_init.gravity[0], _init.gravity[1]));
  _world->SetDestructionListener(this);
  _world->SetContactListener(this);
  _world->SetAutoClearForces(false);

  if(_debugdraw != nullptr)
    _world->SetDebugDraw(_debugdraw);
}

void Box2DSystem::Unload()
{
  if(_world != nullptr)
  {
    delete _world;
    _world = nullptr;
  }
}