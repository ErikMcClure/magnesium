// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __SOLID_H__PS__
#define __SOLID_H__PS__

#include "psLocatable.h"
#include "psRenderable.h"
#include "psRect.h"

namespace bss_util { template<typename T> struct KDNode; }

namespace planeshader {
  // A solid is an inheritable with dimensions. Only objects inheriting from psSolid can be culled.
  class PS_DLLEXPORT psSolid : public psLocatable, public psRenderable
  {
  public:
    psSolid(const psSolid& copy);
    psSolid(psSolid&& mov);
    explicit psSolid(const psVec3D& position=VEC3D_ZERO, FNUM rotation=0.0f, const psVec& pivot=VEC_ZERO, psFlag flags=0, int zorder=0, psStateblock* stateblock=0, psShader* shader=0, psPass* pass = 0, const psVec& scale=VEC_ONE);
    virtual ~psSolid();
    virtual void Render(const psParent* parent) override;
    // Generates a rotated rectangle for point collisions (rect-to-rect collisions should be done with an AABB bounding rect from GetBoundingRect()) 
    inline psRectRotateZ GetCollisionRect(const psParent& parent)
    {
      psVec3D pos = parent.CalcPosition(_relpos, _rotation, _pivot);
      pos.xy -= _pivot;
      return psRectRotateZ(pos.x, pos.y, pos.x + _dim.x, pos.y + _dim.y, _rotation, _pivot, pos.z);
    }
    // Generates an AABB bounding rect for general collisions (will also update the collision rect) 
    inline const psRect& GetBoundingRect(const psParent& parent) { return _boundingrect = GetCollisionRect(parent).BuildAABB(); }
    // Gets the cached bounding rect (used by KD-trees)
    inline const psRect& GetBoundingRectStatic() { return _boundingrect; }
    // Gets the dimensions of the object 
    inline const psVec& GetDim() const { return _dim; }
    inline const psVec& GetUnscaledDim() const { return _realdim; }
    // Gets scale 
    inline const psVec& GetScale() const { return _scale; }
    // Sets scale 
    void SetScale(const psVec& scale);
    void SetScaleDim(const psVec& dim);
    // Sets z-depth while maintaining invariant size 
    inline void SetPositionZInvariant(FNUM Z) { SetScale(_scale*((Z+1.0f)/(_relpos.z+1.0f))); SetPositionZ(Z); }
    // Sets the center position using coordinates relative to the dimensions
    inline void SetPivotRel(const psVec& relpos) { SetPivot(relpos*_dim); }
    // Generates a matrix transform using the solid's position, rotation, pivot, and scaling (used when the actual dimensions are implicit)
    void GetTransform(psMatrix& matrix, const psParent* parent);
    virtual psSolid* Clone() const override { return 0; }

    psSolid& operator =(const psSolid& right);
    psSolid& operator =(psSolid&& right);

    friend class psCullGroup;

  protected:
    void SetDim(const psVec& dim);

    psVec _dim;
    psVec _realdim;
    psVec _scale;

  private:
    void _setdim(const psVec& dim);

    psRect _boundingrect;
    bss_util::KDNode<psSolid>* _kdnode;
  };
}

#endif