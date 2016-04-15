// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __BOX2D_H__MG__
#define __BOX2D_H__MG__

#include "mgSystem.h"
#include "Box2D/Common/b2Math.h"
#include "Box2D/Dynamics/b2WorldCallbacks.h"
#include "Box2D/Common/b2Draw.h"
#include "Box2D/Dynamics/b2Body.h"
#include "bss-util/cHash.h"
#include "bss-util/cSerializer.h"

class b2Draw;
class b2World;

namespace magnesium {
  struct b2PhysicsComponent;

  // A compound fixture that has its own responses to Begin/End contact (contact points are handled in the body) 
  struct MG_DLLEXPORT b2CompoundFixture
  {
    b2CompoundFixture(b2Fixture* fixture);
    b2CompoundFixture(b2CompoundFixture&& mov);
    ~b2CompoundFixture();
    b2Fixture* BSS_FASTCALL AddFixture(const b2FixtureDef& fd);
    b2Fixture* BSS_FASTCALL AddFixture(const b2Shape& shape, float density);
    inline b2PhysicsComponent* GetParent();

    void* userdata;
    b2Fixture* root; // Fixtures in a SuperFixture are together in the master fixture list, so we only keep a pointer to the root.
    unsigned int count; // Number of fixtures (if 0, root becomes invalid. If root is valid this must be at least 1)
    //std::function<void(const cPhysCollision&, cPhysFix*)> rsp;
  };

  struct MG_DLLEXPORT b2PhysicsComponent : mgComponent<b2PhysicsComponent>
  {
    b2PhysicsComponent();
    b2PhysicsComponent(b2PhysicsComponent&& mov);
    ~b2PhysicsComponent();
    void Init(const b2BodyDef& def);
    // Gets the b2Body* pointer 
    inline b2Body* GetBody() const { return _body; }
    // Sets the position or rotation, which wakes the body up and resets it's collisions
    inline void BSS_FASTCALL SetPosition(const b2Vec2& pos) { SetTransform(pos, _body->GetAngle()); }
    b2Vec2 BSS_FASTCALL GetPosition() const;
    inline void BSS_FASTCALL SetRotation(float rotation) { SetTransform(_body->GetPosition(), rotation); }
    inline float BSS_FASTCALL GetRotation() const { return _body->GetAngle(); }
    void BSS_FASTCALL SetTransform(const b2Vec2& pos, float rotation);
    // Gets the root fixture for this body. Tends to be equal to the last compound fixture in the list 
    const b2Fixture* GetRootFixture() const { return _body->GetFixtureList(); }
    b2Fixture* GetRootFixture() { return _body->GetFixtureList(); }
    // Gets/Sets the physics object user data for storing application specific information (serves as replacement to b2Body userdata) 
    inline void SetUserData(void* userdata) { _userdata = userdata; }
    inline void* GetUserData() const { return _userdata; }
    // Gets number of compound fixtures 
    inline size_t BSS_FASTCALL NumFixtures() const { return _fixtures.size(); }
    // Gets a compound fixture 
    inline b2CompoundFixture& BSS_FASTCALL GetCompoundFixture(size_t i) { assert(i < _fixtures.size()); return *_fixtures[i]; }
    inline const b2CompoundFixture& BSS_FASTCALL GetCompoundFixture(size_t i) const { assert(i < _fixtures.size()); return *_fixtures[i]; }
    // Creates a new compound fixture with a single fixture inside it
    b2CompoundFixture& BSS_FASTCALL AddCompoundFixture(const b2FixtureDef& fd);
    b2CompoundFixture& BSS_FASTCALL AddCompoundFixture(const b2Shape& shape, float density);

    b2PhysicsComponent& BSS_FASTCALL operator =(b2PhysicsComponent&& right);
    inline const b2CompoundFixture& BSS_FASTCALL operator [](size_t i) const { assert(i < _fixtures.size()); return *_fixtures[i]; }
    inline b2CompoundFixture& BSS_FASTCALL operator [](size_t i) { assert(i < _fixtures.size()); return *_fixtures[i]; }

  protected:
    void _destruct();

    b2Body* _body;
    void* _userdata;
    std::vector<std::unique_ptr<b2CompoundFixture>> _fixtures;
    //std::function<void(const cPhysCP&, cPhysFix*)> _rcp;
    //int _phystype;
  };

  class mgDebugDraw : public b2Draw { public: virtual void Clear() = 0; };

#pragma warning(push)
#pragma warning(disable:4275)
  class MG_DLLEXPORT Box2DSystem : public b2DestructionListener, public b2ContactListener, public mgSystem<b2PhysicsComponent, b2PhysicsComponent>
  {
#pragma warning(pop)
  public:
    struct B2INIT {
      float gravity[2];
      uint32_t pos_iters;
      uint32_t vel_iters;
      float hertz;
      float ppm;

      template<typename Engine>
      void Serialize(bss_util::cSerializer<Engine>& e)
      {
        e.EvaluateType<B2INIT>(
          GenPair("gravity", gravity),
          GenPair("pos_iters", pos_iters),
          GenPair("vel_iters", vel_iters),
          GenPair("hertz", hertz),
          GenPair("ppm", ppm)
          );
      }
    };

    Box2DSystem(const B2INIT& init);
    ~Box2DSystem();
    virtual void Preprocess() override;
    virtual void Process(mgEntity* entity) override;
    virtual void Postprocess() override;
    virtual void SayGoodbye(b2Joint* joint) override;
    virtual void SayGoodbye(b2Fixture* fixture) override;
    virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
    virtual void BeginContact(b2Contact* contact) override;
    virtual void EndContact(b2Contact* contact) override;
    inline b2World* GetWorld() { return _world; }
    inline double GetHertz() const { return _init.hertz; }
    void SetHertz(double hertz);
    inline uint32_t GetPosIters() const { return _init.pos_iters; }
    inline void SetPosIters(uint32_t positers) { _init.pos_iters = positers; }
    inline uint32_t GetVelIters() const { return _init.vel_iters; }
    inline void SetVelIters(uint32_t veliters) { _init.vel_iters = veliters; }
    inline b2Vec2 GetGravity();
    inline void SetGravity(const b2Vec2& gravity);
    inline mgDebugDraw* GetDebugDraw() const { return _debugdraw; }
    inline void SetDebugDraw(mgDebugDraw* debugdraw);
    inline bool GetFrozen() const { return _frozen; }
    inline void SetFrozen(bool freeze) { _frozen = freeze; }
    virtual void Reload();
    void Unload();

    static Box2DSystem* Instance();
    const float F_PPM;
    const float INV_PPM;

  protected:
    double _getphysdelta();
    BSS_FORCEINLINE static std::pair<void*, void*> _makepair(void* a, void* b)
    {
      if(reinterpret_cast<size_t>(a) < reinterpret_cast<size_t>(b)) // Using < comparison operators on unrelated pointers is undefined behavior.
        return std::pair<void*, void*>(a, b);
      return std::pair<void*, void*>(b, a);
    }

    static Box2DSystem* _instance;

    b2World* _world;
    mgDebugDraw* _debugdraw;
    bss_util::cHash<std::pair<void*, void*>, uint32_t> _collisionhash; // When inserting collision pairs, the first pointer should always be less than the second
    B2INIT _init;
    double _dt;
    bool _frozen;
    double _totaldelta;
  };
}

#endif