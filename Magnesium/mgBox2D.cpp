// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgBox2D.h"
#include "mgEngine.h"
#include "Box2D/Box2D.h"

using namespace magnesium;

Box2DSystem* Box2DSystem::_instance = 0;

b2CompoundFixture::b2CompoundFixture(b2Fixture* fixture) : userdata(fixture->GetUserData()), root(fixture), count(1) { fixture->SetUserData(this); }
b2CompoundFixture::b2CompoundFixture(b2CompoundFixture&& mov) : root(mov.root), count(mov.count), userdata(mov.userdata) { mov.root = 0; mov.count = 0; mov.userdata = 0; }
b2CompoundFixture::~b2CompoundFixture() {}
inline b2PhysicsComponent* b2CompoundFixture::GetParent()
{
  if(root != 0 && root->GetBody() != 0)
    return reinterpret_cast<b2PhysicsComponent*>(root->GetBody()->GetUserData());
  return 0;
}

b2PhysicsComponent::b2PhysicsComponent(b2PhysicsComponent&& mov) : _body(mov._body), _userdata(mov._body), _fixtures(std::move(mov._fixtures))
{ 
  mov._body = 0;
  mov._userdata = 0;
}
b2PhysicsComponent::b2PhysicsComponent() : _body(0), _userdata(0) {}
b2PhysicsComponent::~b2PhysicsComponent() { _destruct(); }
void b2PhysicsComponent::_destruct()
{
  if(_body)
    Box2DSystem::Instance()->GetWorld()->DestroyBody(_body);
  _body = 0;
  _fixtures.clear();
}

void b2PhysicsComponent::Init(const b2BodyDef& def)
{ 
  _body = Box2DSystem::Instance()->GetWorld()->CreateBody(&def);
  _userdata = def.userData;
  _body->SetUserData(this);
}
b2CompoundFixture& BSS_FASTCALL b2PhysicsComponent::AddCompoundFixture(const b2FixtureDef& fd)
{ 
  assert(_body);
  _fixtures.push_back(std::unique_ptr<b2CompoundFixture>(new b2CompoundFixture(_body->CreateFixture(&fd))));
  return *_fixtures.back();
}
b2CompoundFixture& BSS_FASTCALL b2PhysicsComponent::AddCompoundFixture(const b2Shape& shape, float density)
{ 
  assert(_body);
  _fixtures.push_back(std::unique_ptr<b2CompoundFixture>(new b2CompoundFixture(_body->CreateFixture(&shape, density))));
  return *_fixtures.back();
}
b2Vec2 BSS_FASTCALL b2PhysicsComponent::GetPosition() const
{
  b2Vec2 p = _body->GetPosition();
  p *= Box2DSystem::Instance()->F_PPM;
  return p;
}
void BSS_FASTCALL b2PhysicsComponent::SetTransform(const b2Vec2& pos, float rotation)
{
  assert(_body);
  b2Vec2 p(pos);
  p *= Box2DSystem::Instance()->INV_PPM;
  _body->SetTransform(p, rotation);
}

b2PhysicsComponent& BSS_FASTCALL b2PhysicsComponent::operator =(b2PhysicsComponent&& right)
{ 
  _destruct();
  _body = right._body;
  _userdata = right._userdata;
  _fixtures = std::move(right._fixtures);
  right._body = 0;
  right._userdata = 0;
  return *this;
}

Box2DSystem::Box2DSystem(const B2INIT& init) : _totaldelta(0), _dt(0), _world(0), _init(init), _debugdraw(0),
  _frozen(false), F_PPM(_init.ppm), INV_PPM(1.0f/_init.ppm)
{
  _instance = this;
  Reload();
}
Box2DSystem::~Box2DSystem()
{
  _instance = 0;
  Unload();
}
void Box2DSystem::Preprocess()
{
  if(_frozen || !_world)
    return;

  double delta = _getphysdelta();
  if(delta > 0.0) // This technique lets us do both variable and fixed-step updates.
  {
    if(_debugdraw) _debugdraw->Clear();
    _world->Step(delta, _init.vel_iters, _init.pos_iters);
    _world->ClearForces();
    _world->DrawDebugData();
  }
}
Box2DSystem* Box2DSystem::Instance() { return _instance; }
void Box2DSystem::Process(mgEntity* entity) {}
void Box2DSystem::Postprocess() {}

void Box2DSystem::SayGoodbye(b2Joint* joint) {}
void Box2DSystem::SayGoodbye(b2Fixture* fixture) {}
void Box2DSystem::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {}
void Box2DSystem::BeginContact(b2Contact* contact) {}
void Box2DSystem::EndContact(b2Contact* contact) {}

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
  if(_debugdraw != 0)
    _world->SetDebugDraw(_debugdraw);
}

void Box2DSystem::Unload()
{
  if(_world != 0)
  {
    delete _world;
    _world = 0;
  }
}

double Box2DSystem::_getphysdelta()
{
  double delta = mgEngine::Instance()->GetDelta() / 1000.0;

  if(_init.hertz == 0.0) // Variable technique
    delta = bssmin(1.0, delta);
  else if(_init.hertz < 0) // Variable ceiling technique
  {
    _totaldelta += bssmin(1.0, delta);
    if(_totaldelta >= (_dt*2.0))
    {
      delta = _dt;
      _totaldelta -= _dt;
    }
    else {
      delta = _totaldelta;
      _totaldelta = 0;
    }
  }
  else // Constant technique
  {
    _totaldelta += bssmin(1.0, delta);
    delta = 0.0;
    if(_totaldelta > -_dt) // We use a negative value here to allow variations in frame deltas to even out over time
    {
      delta = _dt;
      _totaldelta -= _dt;
    }
  }

  return delta;
}