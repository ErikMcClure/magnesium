// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __RENDERABLE_H__PS__
#define __RENDERABLE_H__PS__

#include "psStateBlock.h"
#include "psShader.h"
#include "psParent.h"
#include "bss-util/cBitField.h"
#include "bss-util/LLBase.h"
#include "bss-util/cTRBtree.h"

namespace planeshader {
  class PS_DLLEXPORT psRenderable
  {
    friend class psPass;
    friend class psCullGroup;

  public:
    psRenderable(const psRenderable& copy);
    psRenderable(psRenderable&& mov);
    explicit psRenderable(psFlag flags=0, int zorder=0, psStateblock* stateblock=0, psShader* shader=0, psPass* pass=0);
    virtual ~psRenderable();
    virtual void Render(const psParent* parent);
    int GetZOrder() const { return _zorder; }
    void SetZOrder(int zorder);
    inline psPass* GetPass() const { return _pass; }
    void SetPass(psPass* pass);
    void SetPass(); // Sets the pass to the 0th pass.
    inline bss_util::cBitField<psFlag>& GetFlags() { return _flags; }
    inline psFlag GetFlags() const { return _flags; }
    inline psShader* GetShader() { return _shader; }
    inline const psShader* GetShader() const { return _shader; }
    inline void SetShader(psShader* shader) { _shader=shader; }
    inline const psStateblock* GetStateblock() const { return _stateblock; }
    void SetStateblock(psStateblock* stateblock);
    virtual psTex* const* GetTextures() const;
    virtual uint8_t NumTextures() const;
    virtual psTex* const* GetRenderTargets() const;
    virtual uint8_t NumRT() const;
    virtual void SetRenderTarget(psTex* rt, uint32_t index = 0);
    void ClearRenderTargets();
    virtual psRenderable* Clone() const { return 0; }

    psRenderable& operator =(const psRenderable& right);
    psRenderable& operator =(psRenderable&& right);

    void Activate();
    virtual void _render(const psParent& parent) = 0;

  protected:
    void _destroy();
    void _copyinsert(const psRenderable& r);
    void _invalidate();

    bss_util::cBitField<psFlag> _flags;
    psPass* _pass; // Stores what pass we are in
    int _zorder;
    uint8_t _internalflags;
    bss_util::ref_ptr<psStateblock> _stateblock;
    bss_util::ref_ptr<psShader> _shader;
    bss_util::LLBase<psRenderable> _llist;
    bss_util::TRB_Node<psRenderable*>* _psort;
    bss_util::cArray<psTex*, uint8_t> _rts;

    enum INTERNALFLAGS : uint8_t
    {
      INTERNALFLAG_ACTIVE = 0x80,
    };
  };
}

#endif