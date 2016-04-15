// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __EFFECT_H__PS__
#define __EFFECT_H__PS__

#include "psInheritable.h"
#include "bss-util/cArraySort.h"

namespace planeshader {
  // Given a set of interconnected renderables, does a topological sort to find the correct multipass rendering configuration.
  class psEffect : public psInheritable
  {
    struct Edge
    {
      psRenderable* f;
      uint8_t fi;
      psRenderable* t;
      uint8_t ti;
    };

  public:
    // Constructor - note that psEffect ignores it's shader and stateblock as it does not actually render anything itself.
    explicit psEffect(const psVec3D& position = VEC3D_ZERO, FNUM rotation = 0.0f, const psVec& pivot = VEC_ZERO, psFlag flags = 0, int zorder = 0, psPass* pass = 0, psInheritable* parent = 0);
    // Links the output of one shader to the input of another and recalculates the topological ordering. Fails if this creates a cycle in the directed graph.
    bool Link(psRenderable* src, uint8_t srcindex, psRenderable* dest, uint8_t destindex);

  protected:
    bool _sort();
    void _sortvisit(psRenderable* child, bool& fail, uint32_t& order);
    virtual void BSS_FASTCALL _render() override;

    static char CompEdge(const Edge& l, const Edge& r) { return SGNCOMPARE(l.f, r.f); }
    static bool LessEdge(const Edge& l, const Edge& r) { return l.f < r.f; }

    bss_util::cArraySort<Edge, &CompEdge> _edges;
  };
}

#endif