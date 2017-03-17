// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __LOCATABLE_H__PS__
#define __LOCATABLE_H__PS__

#include "psVec.h"
#include "psDriver.h"
#include "psParent.h"

namespace planeshader {
  // This holds position information. 
  class PS_DLLEXPORT psLocatable
  {
  public:
    psLocatable(const psLocatable& copy);
    explicit psLocatable(const psVec3D& position, FNUM rotation=0.0f, const psVec& pivot=VEC_ZERO);
    virtual ~psLocatable();
    // Gets the rotation 
    inline FNUM GetRotation() const { return _rotation; }
    // Sets the rotation of this object 
    void SetRotation(FNUM rotation);
    // Gets the pivot
    inline const psVec& GetPivot() const { return _pivot; }
    // Sets the pivot (this is where the image will rotate around) 
    void SetPivot(const psVec& pivot);
    // Gets the position relative to the object parent 
    inline const psVec3D& GetPosition() const { return _relpos; }
    // Sets the position of this object 
    inline void SetPositionX(FNUM X) { SetPosition(X, _relpos.y, _relpos.z); }
    inline void SetPositionY(FNUM Y) { SetPosition(_relpos.x, Y, _relpos.z); }
    inline void SetPositionZ(FNUM Z) { SetPosition(_relpos.x, _relpos.y, Z); }
    inline void SetPosition(const psVec& pos) { SetPosition(pos.x, pos.y, _relpos.z); }
    inline void SetPosition(const psVec3D& pos) { SetPosition(pos.x, pos.y, pos.z); }
    inline void SetPosition(FNUM X, FNUM Y) { SetPosition(X, Y, _relpos.z); }
    void SetPosition(FNUM X, FNUM Y, FNUM Z);
    void GetTransform(psMatrix& matrix, const psParent* parent);

    inline psLocatable& operator=(const psLocatable& right) { _relpos=right._relpos; _rotation=right._rotation; _pivot=right._pivot; return *this; }

  protected:
    psVec3D _relpos;
    psVec _pivot; // Rotational center in absolute coordinates relative from its location
    FNUM _rotation; //Rotation of the object
  };
}

#endif