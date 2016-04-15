// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __SOLID_H__PS__
#define __SOLID_H__PS__

#include "psInheritable.h"
#include "psRect.h"

namespace bss_util { template<typename T> struct KDNode; }

namespace planeshader {
  // A solid is an inheritable with dimensions. Only objects inheriting from psSolid can be culled.
  class PS_DLLEXPORT psSolid : public psInheritable
  {
  public:
    psSolid(const psSolid& copy);
    psSolid(psSolid&& mov);
    explicit psSolid(const psVec3D& position=VEC3D_ZERO, FNUM rotation=0.0f, const psVec& pivot=VEC_ZERO, psFlag flags=0, int zorder=0, psStateblock* stateblock=0, psShader* shader=0, psPass* pass = 0, psInheritable* parent=0, const psVec& scale=VEC_ONE);
    virtual ~psSolid();
    virtual void Render() override;
    // Recalculates a rotated rectangle for point collisions (rect-to-rect collisions should be done with an AABB bounding rect from GetBoundingRect()) 
    inline const psRectRotateZ& GetCollisionRect() { UpdateBoundingRect(); return _collisionrect; }
    // Recalculates an AABB bounding rect for general collisions (will also update the collision rect) 
    inline const psRect& GetBoundingRect() { UpdateBoundingRect(); return _boundingrect; }
    // Updates bounding rect if an update is needed 
    void UpdateBoundingRect();// { _updateboundingrect<PSFLAG_REQUIRE_CULL_REFRESH>(); }
    // Returns collision rect without updating it 
    inline const psRectRotateZ& GetCollisionRectStatic() const { return _collisionrect; } //This does not update the collision rect
    // Returns boudning rect without updating it 
    inline const psRect& GetBoundingRectStatic() const { return _boundingrect; } //This does not update the bounding rect
    // Gets the dimensions of the object 
    inline const psVec& GetDim() const { return _dim; }
    inline const psVec& GetUnscaledDim() const { return _realdim; }
    // Gets scale 
    inline const psVec& GetScale() const { return _scale; }
    // Sets scale 
    virtual void BSS_FASTCALL SetScale(const psVec& scale);
    void BSS_FASTCALL SetScaleDim(const psVec& dim);
    // Sets z-depth while maintaining invariant size 
    inline void BSS_FASTCALL SetPositionZInvariant(FNUM Z) { SetScale(_scale*((Z+1.0f)/(_relpos.z+1.0f))); SetPositionZ(Z); }
    // Sets the center position using coordinates relative to the dimensions
    inline void BSS_FASTCALL SetPivotRel(const psVec& relpos) { SetPivot(relpos*_dim); }
    // Generates a matrix transform using the solid's position, rotation, pivot, and scaling (used when the actual dimensions are implicit)
    void BSS_FASTCALL GetTransform(bss_util::Matrix<float, 4, 4>& matrix);

    psSolid& operator =(const psSolid& right);
    psSolid& operator =(psSolid&& right);

    friend class psCullGroup;

  protected:
    void BSS_FASTCALL SetDim(const psVec& dim);

    psVec _dim;
    psVec _realdim;
    psVec _scale;

  private:
    void BSS_FASTCALL _setdim(const psVec& dim);

    psRectRotateZ _collisionrect;
    psRect _boundingrect;
    bss_util::KDNode<psSolid>* _kdnode;
  };
}

#endif