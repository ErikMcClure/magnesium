// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __PASS_H__PS__
#define __PASS_H__PS__

#include "psDriver.h"
#include "psRenderable.h"
#include "psCamera.h"
#include "bss-util\bss_alloc_block.h"
#include "bss-util\cDynArray.h"

namespace planeshader {
  class psSolid;
  class psMonitor;

  // Defines a single, encapsulated render pass in the pipeline
  class PS_DLLEXPORT psPass : public psDriverHold
  {
  public:
    explicit psPass(psMonitor* monitor = 0);
    ~psPass();
    void Begin();
    void End();
    inline void BSS_FASTCALL SetCamera(const psCamera* camera) { _cam=!camera?&psCamera::default_camera:camera; }
    inline const psCamera* GetCamera() const { return _cam; }
    psTex* const* GetRenderTarget();
    inline void SetRenderTarget(psTex* rt=0) { _defaultrt = rt; }
    void Insert(psRenderable* r);
    void Remove(psRenderable* r);
    inline void SetClearColor(uint32_t color, bool enable = true) { _clearcolor = color; _clear = enable; }
    inline uint32_t GetClearColor() const { return _clearcolor; }
    uint32_t GetDPI();
    inline void SetDPI(uint32_t dpi = 0) { _dpi = dpi; }
    inline psMonitor* GetMonitor() const { return _monitor; }
    inline void SetMonitor(psMonitor* monitor = 0) { _monitor = monitor; }

    static BSS_FORCEINLINE bss_util::LLBase<psRenderable>& GetRenderableAlt(psRenderable* r) { return r->_llist; }
    static BSS_FORCEINLINE char StandardCompare(psRenderable* const& l, psRenderable* const& r)
    {
      return l->_sort(r);
      //char c = SGNCOMPARE(l->_zorder, r->_zorder);
      //if(!c) c = SGNCOMPARE(l,r); return c; 
    }
    static psPass* CurPass;

    typedef bss_util::BlockPolicy<bss_util::TRB_Node<psRenderable*>> ALLOC;
    friend class psRenderable;
    friend class psCullGroup;

  protected:
    void _sort(psRenderable* r);
    void _addcullgroup(psCullGroup* g);
    void _removecullgroup(psCullGroup* g);

    const psCamera* _cam;
    psRenderable* _renderables;
    psCullGroup* _cullgroups;
    psTex* _defaultrt;
    ALLOC _renderalloc;
    uint32_t _clearcolor;
    bool _clear;
    uint32_t _dpi;
    psMonitor* _monitor;
    bss_util::cTRBtree<psRenderable*, StandardCompare, ALLOC> _renderlist;
  };
}

#endif