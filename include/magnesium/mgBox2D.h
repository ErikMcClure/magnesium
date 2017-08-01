// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __BOX2D_H__MG__
#define __BOX2D_H__MG__

#include "mgSystem.h"
#include "Box2D/Common/b2Math.h"
#include "Box2D/Dynamics/b2WorldCallbacks.h"
#include "Box2D/Common/b2Draw.h"
#include "Box2D/Dynamics/b2Body.h"
#include "bss-util/Hash.h"
#include "bss-util/Serializer.h"

class b2Draw;
class b2World;

namespace magnesium {
  // Holds a b2Body object that you can attach fixtures to.
  struct MG_DLLEXPORT b2Component : mgComponent<b2Component, true>
  {
    struct ContactPoint
    {
      b2Fixture* self;
      b2Fixture* other;
      b2Vec2 point;
      b2Vec2 normal;
      b2PointState state;
    };
    typedef void(*CPResponse)(b2Component&, ContactPoint&);

    b2Component(mgEntity* e);
    b2Component(b2Component&& mov);
    ~b2Component();
    void Init(const b2BodyDef& def);
    // Gets the b2Body* pointer 
    inline b2Body* GetBody() const { return _body; }
    // Sets the position or rotation, which wakes the body up and resets it's collisions
    inline void SetPosition(const b2Vec2& pos) { SetTransform(pos, _body->GetAngle()); }
    b2Vec2 GetPosition() const;
    b2Vec2 GetOldPosition() const;
    inline void SetRotation(float rotation) { _body->SetTransform(_body->GetPosition(), rotation); }
    inline float GetRotation() const { return _body->GetAngle(); }
    inline float GetOldRotation() const { return _oldangle; }
    inline void UpdateOld() { _oldposition = _body->GetPosition(); _oldangle = _body->GetAngle(); }
    void SetTransform(const b2Vec2& pos, float rotation);
    // Gets the root fixture for this body. Tends to be equal to the last compound fixture in the list 
    const b2Fixture* GetRootFixture() const { return _body->GetFixtureList(); }
    b2Fixture* GetRootFixture() { return _body->GetFixtureList(); }
    // Gets/Sets the physics object user data for storing application specific information (serves as replacement to b2Body userdata) 
    inline void SetUserData(void* userdata) { _userdata = userdata; }
    inline void* GetUserData() const { return _userdata; }
    // Gets/Sets the collision response 
    CPResponse GetCPResponse() const { return _rcp; }
    inline void SetCPResponse(CPResponse rcp) { _rcp = rcp; }

    b2Component& operator =(b2Component&& right);

  protected:
    void _destruct();

    b2Body* _body;
    void* _userdata;
    b2Vec2 _oldposition;
    float _oldangle;
    CPResponse _rcp;
    //std::function<void(b2Fixture*, b2Contact*)> _rfp;
    //std::function<void(b2Component*, b2Contact*)> _rbp;
    //int _phystype;
  };

  class mgDebugDraw : public b2Draw { public: virtual void Clear() = 0; };

#pragma warning(push)
#pragma warning(disable:4275)
  class MG_DLLEXPORT Box2DSystem : public b2DestructionListener, public b2ContactListener, public mgSystem<void>
  {
#pragma warning(pop)
  public:
    struct B2INIT {
      B2INIT() : gravity{ 0.0f,9.8f }, pos_iters(3), vel_iters(8), hertz(60.0), ppm(128) { }
      float gravity[2];
      uint32_t pos_iters;
      uint32_t vel_iters;
#ifdef b2_invalidParticleIndex
      uint32_t particle_iters;
#endif
      float hertz;
      float ppm;

      template<typename Engine>
      void Serialize(bss::Serializer<Engine>& e)
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

    Box2DSystem(const B2INIT& init, int priority = 0);
    ~Box2DSystem();
    virtual void Process() override;
    virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
    virtual void SayGoodbye(b2Joint* joint) override {}
    virtual void SayGoodbye(b2Fixture* fixture) override {}
    virtual const char* GetName() const override { return "Box2D"; }
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
    inline double GetDeltaRatio() const { return _ratio; }
    virtual void Reload();
    void Unload();

    static Box2DSystem* Instance();
    const float F_PPM;
    const float INV_PPM;

  protected:
    void _updateOld();
    BSS_FORCEINLINE static std::pair<void*, void*> _makepair(void* a, void* b)
    {
      if(reinterpret_cast<size_t>(a) < reinterpret_cast<size_t>(b)) // Using < comparison operators on unrelated pointers is undefined behavior.
        return std::pair<void*, void*>(a, b);
      return std::pair<void*, void*>(b, a);
    }

    static Box2DSystem* _instance;

    b2World* _world;
    mgDebugDraw* _debugdraw;
    bss::Hash<std::pair<void*, void*>, uint32_t> _collisionhash; // When inserting collision pairs, the first pointer should always be less than the second
    bss::DynArray<std::pair<void*, void*>, uint32_t> _deletions; // Stores potential deletions
    B2INIT _init;
    double _dt;
    bool _frozen;
    double _totaldelta;
    double _ratio;
  };
}

#endif