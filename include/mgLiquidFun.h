// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __LIQUIDFUN_H__MG__
#define __LIQUIDFUN_H__MG__

#include "mgBox2D.h"

class b2Draw;
class b2World;

namespace magnesium {
  // Extension of box2D system for liquidfun particle simulations
  class MG_DLLEXPORT LiquidFunSystem : public Box2DSystem
  {
  public:
    struct LFINIT {
      Box2DSystem::B2INIT b2init;
      uint32_t particle_iters;

      template<typename Engine>
      void Serialize(bss_util::cSerializer<Engine>& e)
      {
        e.EvaluateType<LFINIT>(
          GenPair("b2init", b2init),
          GenPair("particle_iters", particle_iters)
          );
      }
    };

    LiquidFunSystem(const B2INIT& init);
    LiquidFunSystem(const LFINIT& init);
    ~LiquidFunSystem();
    virtual void Preprocess() override;
    inline uint32_t GetParticleIters() const { return _particle_iters; }
    inline void SetParticleIters(uint32_t particleiters) { _particle_iters = particleiters; }

  protected:
    uint32_t _particle_iters;
  };
}

#endif