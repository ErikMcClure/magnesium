// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __CULL_GROUP_H__PS__
#define __CULL_GROUP_H__PS__

#include "bss-util/bss_alloc_block.h"
#include "bss-util/cKDTree.h"
#include "bss-util/LLBase.h"
#include "psSolid.h"
#include "psPass.h"

namespace planeshader {
  // Used to efficiently cull a group of images that are static relative to each other. Intended for static level geometry.
  class PS_DLLEXPORT psCullGroup : public bss_util::LLBase<psCullGroup>
  {
  public:
    psCullGroup(psCullGroup&& mov);
    psCullGroup();
    ~psCullGroup();
    // Inserts a solid that must not move relative to the other images in this culling group and removes it from the internal pass list
    void Insert(psSolid* img, bool recalc = false);
    // Removes a solid from this culling group and rebases the origin if necessary.
    void Remove(psSolid* img);
    // Solves the tree
    void Solve();
    // Clears the tree 
    void Clear();
    // Traverses the tree 
    void Traverse(const float(&rect)[4], FNUM camZ);
    // Sets the pass this cull group belongs to
    void SetPass(psPass* pass);
    // Gets or sets the rebalance threshold
    BSS_FORCEINLINE uint32_t GetRBThreshold() const { return _tree.GetRBThreshold(); }
    BSS_FORCEINLINE void SetRBThreshold(uint32_t rbthreshold) { _tree.SetRBThreshold(rbthreshold); }

    typedef bss_util::BlockPolicy<bss_util::KDNode<psSolid>> KDNODE_ALLOC;

  protected:
    BSS_FORCEINLINE static const float* CF_FRECT(psSolid* p) { return p->GetBoundingRectStatic()._ltrbarray; }
    BSS_FORCEINLINE static bss_util::LLBase<psSolid>& CF_FLIST(psSolid* p) { return *((bss_util::LLBase<psSolid>*)&p->_llist); }
    BSS_FORCEINLINE static void CF_FACTION(psSolid* p) { if(!p->GetPass()) { if(!psPass::CurPass) p->_render(); else psPass::CurPass->_sort(p); } else p->GetPass()->_sort(p); }
    BSS_FORCEINLINE static bss_util::KDNode<psSolid>*& CF_FNODE(psSolid* p) { return p->_kdnode; }

    bss_util::cKDTree<psSolid, KDNODE_ALLOC, CF_FRECT, CF_FLIST, CF_FACTION, CF_FNODE> _tree;
    KDNODE_ALLOC _nodealloc;
    psPass* _pass;
  };
}

#endif