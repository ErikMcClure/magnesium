// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __INHERITABLE_H__PS__
#define __INHERITABLE_H__PS__

#include "psLocatable.h"
#include "psRenderable.h"

namespace planeshader {
  class PS_DLLEXPORT psInheritable : public psLocatable, public psRenderable
  {
  public:
    psInheritable(const psInheritable& copy);
    psInheritable(psInheritable&& mov);
    explicit psInheritable(const psVec3D& position=VEC3D_ZERO, FNUM rotation=0.0f, const psVec& pivot=VEC_ZERO, psFlag flags=0, int zorder=0, psStateblock* stateblock=0, psShader* shader=0, psPass* pass = 0, psInheritable* parent=0);
    virtual ~psInheritable();
    virtual void Render() override;
    // Sets/gets the parent
    void SetParent(psInheritable* parent, bool ownership = false);
    inline psInheritable* GetParent() const { return _parent; }
    // Gets all the children
    inline psInheritable* GetChildren() const { return _children; }
    // Counts the number of children
    uint32_t NumChildren() const;
    // Gets the rotation plus the rotation of our parents
    inline FNUM GetTotalRotation() const { return !_parent?_rotation:(_rotation+_parent->GetTotalRotation()); }
    // Gets the absolute position by adding up all the parent positions.
    inline void GetTotalPosition(psVec3D& pos) const { pos = _relpos; if(_parent!=0) _parent->_gettotalpos(pos); }
    // Gets all the flags inherited from the parent
    virtual psFlag GetAllFlags() const override { return !_parent?_flags:(_flags|(_parent->GetAllFlags()&PSFLAG_INHERITABLE)); }
    // Overloads SetPass so it propogates to our children
    virtual void BSS_FASTCALL SetPass(psPass* pass) override;
    // Clone function
    virtual psInheritable* Clone() const override { return 0; }
    virtual psTex* const* GetRenderTargets() const override;
    virtual uint8_t NumRT() const override;
    virtual void BSS_FASTCALL SetZOrder(int zorder) override;
    psInheritable* BSS_FASTCALL AddClone(const psInheritable* inheritable);

    psInheritable& operator=(const psInheritable& copy);
    psInheritable& operator=(psInheritable&& mov);

    inline static bss_util::LLBase<psInheritable>& GetLLBase(psInheritable* x) { return x->_lchild; }
    template<typename F>
    BSS_FORCEINLINE void MapToChildren(F && fn) const
    {
      for(psInheritable* cur=_children; cur!=0; cur=cur->_lchild.next)
        fn(cur);
    }

    inline static bool INHERITABLECOMP(psInheritable* l, psInheritable* r) { return l->_zorder < r->_zorder; }
    virtual void BSS_FASTCALL _render() override;

  protected:
    friend class psSolid;

    void _gettotalpos(psVec3D& pos) const;
    void _sortchildren();
    virtual char BSS_FASTCALL _sort(psRenderable* r) const override;
    virtual psRenderable* BSS_FASTCALL _getparent() const override;
    psInheritable* _prerender();

    bss_util::LLBase<psInheritable> _lchild;
    psInheritable* _parent;
    psInheritable* _children;
    uint32_t _depth;
  };
}

#endif