// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __VEC_H__PS__
#define __VEC_H__PS__

#include "ps_dec.h"
#include "bss-util/bss_vector.h"

namespace planeshader {
  // Geometric point intersection and distance functions
  template<typename T>
  static BSS_FORCEINLINE bool IntersectEllipse(T X, T Y, T A, T B, T x, T y) { T tx=X-x; T ty=Y-y; return ((tx*tx)/(A*A)) + ((ty*ty)/(B*B)) <= 1; }
  template<typename T>
  static inline void EllipseNearestPoint(T A, T B, T cx, T cy, T& outX, T& outY)
  {
    //f(t) = (A*A - B*B)cos(t)sin(t) - x*A*sin(t) + y*B*cos(t)
    //f'(t) = (A*A - B*B)(cos²(t) - sin²(t)) - x*a*cos(t) - y*B*sin(t)
    //x(t) = [A*cos(t),B*sin(t)]
    T A2=A*A;
    T B2=B*B;
    if(((cx*cx)/A2) + ((cy*cy)/B2) <= 1)
    {
      outX=cx;
      outY=cy;
      return;
    }
    T t = atan2(A*cy, B*cx); //Initial guess = arctan(A*y/B*x)
    T AB2 = (A2 - B2); //precalculated values for optimization
    T xA = cx*A;
    T yB = cy*B;
    T f_, f__, tcos, tsin;
    for(int i = 0; i < 4; ++i)
    {
      tcos=cos(t);
      tsin=sin(t);
      f_ = AB2*tcos*tsin - xA*tsin + yB*tcos; //f(t)
      f__ = AB2*(tcos*tcos - tsin*tsin) - xA*tcos - yB*tsin; //f'(t)
      t = t - f_/f__; //Newton's method: t - f(t)/f'(t)
    }

    outX=A*cos(t);
    outY=B*sin(t);
  }
  template<typename T>
  static BSS_FORCEINLINE T PointLineInfDistSqr(T X, T Y, T X1, T Y1, T X2, T Y2) { T tx=X2-X1; T ty=Y2-Y1; T det=(tx*(Y1-Y)) - ((X1-X)*ty); return (det*det)/((tx*tx)+(ty*ty)); }
  template<typename T>
  static BSS_FORCEINLINE T PointLineInfDist(T X, T Y, T X1, T Y1, T X2, T Y2) { T tx=X2-X1; T ty=Y2-Y1; return ((tx*(Y1-Y)) - ((X1-X)*ty))/bss_util::FastSqrt((tx*tx)+(ty*ty)); }
  template<typename T>
  static inline T PointLineDistSqr(T X, T Y, T X1, T Y1, T X2, T Y2) //line segment
  {
    float vx = X1-X; float vy = Y1-Y; float ux = X2-X1; float uy = Y2-Y1; float length = ux*ux+uy*uy;
    float det = (-vx*ux)+(-vy*uy); //if this is < 0 or > length then its outside the line segment
    if(det<0 || det>length) { ux=X2-X; uy=Y2-Y; vx=vx*vx+vy*vy; ux=ux*ux+uy*uy; return (vx<ux)?vx:ux; }
    det = ux*vy-uy*vx;
    return (det*det)/length;
  }
  template<typename T>
  static BSS_FORCEINLINE T PointLineDist(T X, T Y, T X1, T Y1, T X2, T Y2) { return bss_util::FastSqrt(PointLineDistSqr(X, Y, X1, Y1, X2, Y2)); }
  template<typename T>
  static inline void NearestPointToLineInf(T X, T Y, T X1, T Y1, T X2, T Y2, T& outX, T& outY) //treated as infinite line
  {
    T tx=X2-X1; T ty=Y2-Y1;
    T u = ((X-X1)*tx + (Y-Y1)*ty)/((tx*tx) + (ty*ty));

    outX=X1 + u*tx;
    outY=Y1 + u*ty;
  }
  template<typename T>
  static inline void NearestPointToLine(T X, T Y, T X1, T Y1, T X2, T Y2, T& outX, T& outY) //treated as line segment
  {
    T ux=X2-X1; T uy=Y2-Y1;
    T vx=X1-X; T vy=Y1-Y;
    float det = (-vx*ux)+(-vy*uy); //if this is < 0 or > length then its outside the line segment
    float length = ux*ux+uy*uy;
    if(det<0 || det>length)
    {
      ux=X2-X; uy=Y2-Y;
      vx=vx*vx+vy*vy; ux=ux*ux+uy*uy;
      if(vx<ux) { outX=X1; outY=Y1; } else { outX=X2; outY=Y2; } return;
    }

    T u = ((X-X1)*ux + (Y-Y1)*uy)/((ux*ux) + (uy*uy));

    outX=X1 + u*ux;
    outY=Y1 + u*uy;
  }

  typedef bss_util::Vector<float, 2> psVec; //default typedef
  typedef bss_util::Vector<float, 2> psVector;
  typedef bss_util::Vector<int, 2> psVeci;
  typedef bss_util::Vector<double, 2> psVecd;
  typedef bss_util::Vector<short, 2> psVecs;
  typedef bss_util::Vector<long, 2> psVecl;
  typedef bss_util::Vector<uint32_t, 2> psVeciu;
  typedef bss_util::Vector<uint16_t, 2> psVecsu;
  typedef bss_util::Vector<unsigned long, 2> psVeclu;

  static psVec const VEC_ZERO(0, 0);
  static psVec const VEC_ONE(1, 1);
  static psVec const VEC_HALF(0.5f, 0.5f);
  static psVec const VEC_NEGHALF(-0.5f, -0.5f);

  typedef bss_util::Vector<float, 3> psVec3D; //default typedef
  typedef bss_util::Vector<float, 3> psVector3D;
  typedef bss_util::Vector<int, 3> psVec3Di;
  typedef bss_util::Vector<double, 3> psVec3Dd;
  typedef bss_util::Vector<short, 3> psVec3Ds;
  typedef bss_util::Vector<long, 3> psVec3Dl;
  typedef bss_util::Vector<uint32_t, 3> psVec3Diu;
  typedef bss_util::Vector<uint16_t, 3> psVec3Dsu;
  typedef bss_util::Vector<unsigned long, 3> psVec3Dlu;

  static psVec3D const VEC3D_ZERO(0, 0, 0);
  static psVec3D const VEC3D_ONE(1, 1, 1);
}

#endif