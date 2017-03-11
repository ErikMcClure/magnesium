// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __RENDERABLE_H__PS__
#define __RENDERABLE_H__PS__

#include "psStateBlock.h"
#include "psShader.h"
#include "bss-util/cBitField.h"
#include "bss-util/LLBase.h"
#include "bss-util/cTRBtree.h"

namespace planeshader {
  class PS_DLLEXPORT psRenderable
  {
    friend class psPass;
    friend class psInheritable;

  public:
    psRenderable(const psRenderable& copy);
    psRenderable(psRenderable&& mov);
    explicit psRenderable(psFlag flags=0, int zorder=0, psStateblock* stateblock=0, psShader* shader=0, psPass* pass=0);
    virtual ~psRenderable();
    virtual void Render();
    inline int GetZOrder() const { return _zorder; }
    virtual void SetZOrder(int zorder);
    inline psPass* GetPass() const { return _pass; }
    virtual void SetPass(psPass* pass);
    void SetPass(); // Sets the pass to the 0th pass.
    inline bss_util::cBitField<psFlag>& GetFlags() { return _flags; }
    inline psFlag GetFlags() const { return _flags; }
    virtual psFlag GetAllFlags() const;
    inline psShader* GetShader() { return _shader; }
    inline const psShader* GetShader() const { return _shader; }
    inline void SetShader(psShader* shader) { _shader=shader; _invalidate(); }
    inline const psStateblock* GetStateblock() const { return _stateblock; }
    void SetStateblock(psStateblock* stateblock);
    virtual psTex* const* GetTextures() const;
    virtual uint8_t NumTextures() const;
    virtual psTex* const* GetRenderTargets() const;
    virtual uint8_t NumRT() const;
    virtual void SetRenderTarget(psTex* rt, uint32_t index = 0);
    void ClearRenderTargets();

    psRenderable& operator =(const psRenderable& right);
    psRenderable& operator =(psRenderable&& right);

    void Activate();
    virtual void _render() = 0;

  protected:
    void _destroy();
    void _invalidate();
    void _copyinsert(const psRenderable& r);
    virtual psRenderable* _getparent() const;
    virtual char _sort(psRenderable* r) const;

    bss_util::cBitField<psFlag> _flags;
    psPass* _pass; // Stores what pass we are in
    uint8_t _internalflags;
    int _zorder;
    bss_util::ref_ptr<psStateblock> _stateblock;
    bss_util::ref_ptr<psShader> _shader;
    bss_util::LLBase<psRenderable> _llist;
    bss_util::TRB_Node<psRenderable*>* _psort;
    bss_util::cArray<psTex*, uint8_t> _rts;

    enum INTERNALFLAGS : uint8_t
    {
      INTERNALFLAG_ACTIVE = 0x80,
      INTERNALFLAG_SORTED = 0x40,
      INTERNALFLAG_OWNED = 0x20,
    };
  };
}

#endif