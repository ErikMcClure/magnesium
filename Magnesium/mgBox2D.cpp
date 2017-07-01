// Copyright ©2017 Black Sphere Studios
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

b2PhysicsComponent::b2PhysicsComponent(b2PhysicsComponent&& mov) : mgComponent(std::move(mov)), _body(mov._body), _userdata(mov._body), _fixtures(std::move(mov._fixtures))
{ 
  mov._body = 0;
  mov._userdata = 0;
}
b2PhysicsComponent::b2PhysicsComponent(mgEntity* e) : mgComponent(e), _body(0), _userdata(0) {}
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
b2CompoundFixture& b2PhysicsComponent::AddCompoundFixture(const b2FixtureDef& fd, bool append)
{ 
  assert(_body);
  b2Fixture* f = _body->CreateFixture(&fd);
  if(!append || !_fixtures.size())
    _fixtures.push_back(std::unique_ptr<b2CompoundFixture>(new b2CompoundFixture(f)));
  else
    f->SetUserData(_fixtures.back().get());

  return *_fixtures.back();
}
b2CompoundFixture& b2PhysicsComponent::AddCompoundFixture(const b2Shape& shape, float density, bool append)
{ 
  assert(_body);
  b2Fixture* f = _body->CreateFixture(&shape, density);
  if(!append || !_fixtures.size())
    _fixtures.push_back(std::unique_ptr<b2CompoundFixture>(new b2CompoundFixture(f)));
  else
    f->SetUserData(_fixtures.back().get());

  return *_fixtures.back();
}
b2Vec2 b2PhysicsComponent::GetPosition() const
{
  b2Vec2 p = _body->GetPosition();
  p *= Box2DSystem::Instance()->F_PPM;
  return p;
}
void b2PhysicsComponent::SetTransform(const b2Vec2& pos, float rotation)
{
  assert(_body);
  b2Vec2 p(pos);
  p *= Box2DSystem::Instance()->INV_PPM;
  _body->SetTransform(p, rotation);
}

b2PhysicsComponent& b2PhysicsComponent::operator =(b2PhysicsComponent&& right)
{ 
  _destruct();
  _body = right._body;
  _userdata = right._userdata;
  _fixtures = std::move(right._fixtures);
  right._body = 0;
  right._userdata = 0;
  return *this;
}

Box2DSystem::Box2DSystem(const B2INIT& init, int priority) : mgSystem(priority), _totaldelta(0), _dt(0), _world(0), _init(init), _debugdraw(0),
  _frozen(false), F_PPM(init.ppm), INV_PPM(1.0f/ init.ppm)
{
  SetHertz(_init.hertz);
  _instance = this;
  Reload();
}
Box2DSystem::~Box2DSystem()
{
  b2PhysicsComponent::Store().Clear(); // Clear all components out BEFORE we delete the world, otherwise things get mad.
  _instance = 0;
  Unload();
}
void Box2DSystem::Process()
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
    
    // Process deletions
    for(auto& pair : _deletions)
    {
      khiter_t iter = _collisionhash.Iterator(pair);
      if(_collisionhash.ExistsIter(iter) && !_collisionhash.GetValue(iter))
      {
        b2CompoundFixture* a = (b2CompoundFixture*)pair.first;
        b2CompoundFixture* b = (b2CompoundFixture*)pair.second;

        if(a->rsp) a->rsp(b, nullptr);
        if(b->rsp) b->rsp(a, nullptr);

        _collisionhash.RemoveIter(iter);
      }
    }
    _deletions.Clear();
  }
}
Box2DSystem* Box2DSystem::Instance() { return _instance; }

void Box2DSystem::SayGoodbye(b2Joint* joint) {}
void Box2DSystem::SayGoodbye(b2Fixture* fixture) {}
void Box2DSystem::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
  b2CompoundFixture* a = (b2CompoundFixture*)contact->GetFixtureA()->GetUserData();
  b2CompoundFixture* b = (b2CompoundFixture*)contact->GetFixtureB()->GetUserData();
  auto& af = a->GetParent()->GetCPResponse();
  auto& bf = b->GetParent()->GetCPResponse();
  if(af || bf)
  {
    b2PointState s1[b2_maxManifoldPoints];
    b2PointState s2[b2_maxManifoldPoints];
    b2GetPointStates(s1, s2, oldManifold, contact->GetManifold());
  }
}
void Box2DSystem::BeginContact(b2Contact* contact)
{
  auto pair = _makepair(contact->GetFixtureA()->GetUserData(), contact->GetFixtureB()->GetUserData());

  khiter_t iter = _collisionhash.Iterator(pair);
  if(!_collisionhash.ExistsIter(iter)) // Note: It's important to NOT do a function call if the hash exists, even if it's set at 0, because this means a deletion has been canceled out.
  {
    b2CompoundFixture* a = (b2CompoundFixture*)pair.first;
    b2CompoundFixture* b = (b2CompoundFixture*)pair.second;

    if(a->rsp || b->rsp) // Only bother tracking this if there are actually callbacks to call
    {
      _collisionhash.Insert(pair, 1);
      if(a->rsp) a->rsp(b, contact);
      if(b->rsp) b->rsp(a, contact);
    }
  }
  else
    _collisionhash.SetValue(iter, _collisionhash.GetValue(iter) + 1);
}
void Box2DSystem::EndContact(b2Contact* contact)
{
  auto pair = _makepair(contact->GetFixtureA()->GetUserData(), contact->GetFixtureB()->GetUserData());

  khiter_t iter = _collisionhash.Iterator(pair);
  if(_collisionhash.ExistsIter(iter)) // If it doesn't exist we must have ignored it
  {
    uint32_t c = _collisionhash.GetValue(iter) - 1;
    _collisionhash.SetValue(iter, c);
    if(!c)
      _deletions.Add(pair);
  }
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