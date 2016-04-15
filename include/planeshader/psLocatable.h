// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __LOCATABLE_H__PS__
#define __LOCATABLE_H__PS__

#include "psVec.h"

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
    virtual void BSS_FASTCALL SetRotation(FNUM rotation);
    // Gets the pivot
    inline const psVec& GetPivot() const { return _pivot; }
    // Sets the pivot (this is where the image will rotate around) 
    virtual void BSS_FASTCALL SetPivot(const psVec& pivot);
    // Gets the position relative to the object parent 
    inline const psVec3D& GetPosition() const { return _relpos; }
    // Sets the position of this object 
    inline void BSS_FASTCALL SetPositionX(FNUM X) { SetPosition(X, _relpos.y, _relpos.z); }
    inline void BSS_FASTCALL SetPositionY(FNUM Y) { SetPosition(_relpos.x, Y, _relpos.z); }
    inline void BSS_FASTCALL SetPositionZ(FNUM Z) { SetPosition(_relpos.x, _relpos.y, Z); }
    inline void BSS_FASTCALL SetPosition(const psVec& pos) { SetPosition(pos.x, pos.y, _relpos.z); }
    inline void BSS_FASTCALL SetPosition(const psVec3D& pos) { SetPosition(pos.x, pos.y, pos.z); }
    inline void BSS_FASTCALL SetPosition(FNUM X, FNUM Y) { SetPosition(X, Y, _relpos.z); }
    virtual void BSS_FASTCALL SetPosition(FNUM X, FNUM Y, FNUM Z);

    inline psLocatable& operator=(const psLocatable& right) { _relpos=right._relpos; _rotation=right._rotation; _pivot=right._pivot; return *this; }

  protected:
    psVec3D _relpos;
    psVec _pivot; // Rotational center in absolute coordinates relative from its location
    FNUM _rotation; //Rotation of the object
  };
}

#endif