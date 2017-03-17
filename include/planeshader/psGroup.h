// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __GROUP_H__PS__
#define __GROUP_H__PS__

#include "psLocatable.h"
#include "psRenderable.h"
#include "bss-util/cArraySort.h"

namespace planeshader {
  class PS_DLLEXPORT psGroup : public psLocatable, public psRenderable
  {
  public:
    psGroup(const psGroup& copy);
    psGroup(psGroup&& mov);
    explicit psGroup(const psVec3D& position=VEC3D_ZERO, FNUM rotation=0.0f, const psVec& pivot=VEC_ZERO, psFlag flags=0, int zorder=0, psStateblock* stateblock=0, psShader* shader=0, psPass* pass = 0);
    virtual ~psGroup();
    // Gets all the children
    inline psRenderable* const* GetChildren() const { return _children.begin(); }
    // Gets number of children
    inline size_t NumChildren() const { return _children.Length(); }
    // Clone function
    virtual psGroup* Clone() const override { return new psGroup(*this); }
    psRenderable* Add(const psRenderable* renderable);
    template<typename F>    BSS_FORCEINLINE void MapToChildren(F && fn) const    {      for(size_t i = 0; i < _children.Length(); ++i)
        fn(_children[i]);    }
    psGroup& operator=(const psGroup& copy);
    psGroup& operator=(psGroup&& mov);

    inline static char Comp(psRenderable* const& l, psRenderable* const& r) { char c = SGNCOMPARE(l->GetZOrder(), r->GetZOrder()); return !c ? SGNCOMPARE(l, r) : c; }
    virtual void _render(const psParent& parent) override;

  protected:
    bss_util::cArraySort<psRenderable*, &Comp> _children;
  };
}

#endif