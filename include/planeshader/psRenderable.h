// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __RENDERABLE_H__PS__
#define __RENDERABLE_H__PS__

#include "psStateBlock.h"
#include "psShader.h"
#include "bss-util/cBitField.h"
#include "bss-util/LLBase.h"
#include "bss-util/cTRBtree.h"
#include "bss-util/cSmartPtr.h"

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
    virtual void BSS_FASTCALL SetZOrder(int zorder);
    inline psPass* GetPass() const { return _pass; }
    virtual void BSS_FASTCALL SetPass(psPass* pass);
    void BSS_FASTCALL SetPass(); // Sets the pass to the 0th pass.
    inline bss_util::cBitField<psFlag>& GetFlags() { return _flags; }
    inline psFlag GetFlags() const { return _flags; }
    virtual psFlag GetAllFlags() const;
    inline psShader* GetShader() { return _shader; }
    inline const psShader* GetShader() const { return _shader; }
    inline void BSS_FASTCALL SetShader(psShader* shader) { _shader=shader; _invalidate(); }
    inline const psStateblock* GetStateblock() const { return _stateblock; }
    void BSS_FASTCALL SetStateblock(psStateblock* stateblock);
    virtual psTex* const* GetTextures() const;
    virtual uint8_t NumTextures() const;
    virtual psTex* const* GetRenderTargets() const;
    virtual uint8_t NumRT() const;
    virtual void BSS_FASTCALL SetRenderTarget(psTex* rt, uint32_t index = 0);
    void ClearRenderTargets();
    virtual psRenderable* Clone() const = 0;

    psRenderable& operator =(const psRenderable& right);
    psRenderable& operator =(psRenderable&& right);

    void Activate();
    virtual void BSS_FASTCALL _render() = 0;

  protected:
    void _destroy();
    void _invalidate();
    void _copyinsert(const psRenderable& r);
    virtual psRenderable* BSS_FASTCALL _getparent() const;
    virtual char BSS_FASTCALL _sort(psRenderable* r) const;

    bss_util::cBitField<psFlag> _flags;
    psPass* _pass; // Stores what pass we are in
    uint8_t _internalflags;
    int _zorder;
    bss_util::cAutoRef<psStateblock> _stateblock;
    bss_util::cAutoRef<psShader> _shader;
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