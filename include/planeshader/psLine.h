// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __LINE_H__PS__
#define __LINE_H__PS__

#include "psRect.h"

namespace planeshader {
  // Represents a 2D line
  template <class T>
  struct BSS_COMPILER_DLLEXPORT psLineT
  {
    typedef bss_util::Vector<T, 2> VEC;
    inline psLineT() {} //The following constructors allow for implicit conversion between types
    template<class U>
    inline psLineT(const psLineT<U>& other) : p1(other.p1), p2(other.p2) {}
    inline psLineT(T X1, T Y1, T X2, T Y2) : x1(X1), y1(Y1), x2(X2), y2(Y2) {}
    inline psLineT(const VEC& P1, T X2, T Y2) : p1(P1), x2(X2), y2(Y2) {}
    inline psLineT(const VEC& P1, const VEC& P2) : p1(P1), p2(P2) {}
    inline T Length() const { return bss_util::dist<T>(x1, y1, x2, y2); }
    inline bool IntersectCircle(const psCircleT<T>& other) const { return DistanceToPointSquared(other.pos)<=other.r*other.r; }
    inline bool IntersectCircle(const VEC& pos, T R) const { return DistanceToPointSquared(pos) <= R*R; }
    inline bool IntersectCircle(T X, T Y, T R) const { return DistanceToPointSquared(X, Y) <= R*R; }
    inline bool IntersectRect(const psRectT<T>& rect) const { return RectLineIntersect(x1, y1, x2, y2, rect._ltrbarray); }
    inline bool IntersectRect(const T(&rect)[4]) const { return RectLineIntersect(x1, y1, x2, y2, rect); }
    inline bool IntersectLine(const psLineT<T>& other) const { return LineLineIntersect(x1, y1, x2, y2, other.x1, other.y1, other.x2, other.y2); }
    inline bool IntersectLine(T X1, T Y1, T X2, T Y2) const { return LineLineIntersect(x1, y1, x2, y2, X1, Y1, X2, Y2); }
    inline bool IntersectVertLine(T X, T Y1, T Y2) const
    {
      T db = Y2-Y1;
      T dx = x2-x1; T dy = y2-y1;
      dy = (dx*(Y1-y1)+dy*(x1-X))/(-db*dx);
      X = (db*(X-x1))/(db*dx);
      return (dy >= 0 && dy <= 1 && X >= 0 && X <= 1);
    }
    inline bool IntersectHorzLine(T X1, T X2, T Y) const {
      T da = X2-X1;
      T dx = x2-x1; T dy = y2-y1;
      dx = (dx*(Y-y1)+dy*(x1-X1))/(da*dy);
      Y = (da*(y1-Y))/(-da*dy);
      return (dx >= 0 && dx <= 1 && Y >= 0 && Y <= 1);
    }

    inline bool PointOfIntersection(const psLineT<T>& other, VEC* retpoint)
    {
      VEC p(x1, y1);
      VEC r(x2-x1, y2-y1);
      VEC q(other.x1, other.y1);
      VEC s(other.x2-other.x1, other.y2-other.y1);
      float rCrossS = r.CrossProduct(s);

      if(rCrossS <= 0 && rCrossS >= -1) return false;
      float t = s.CrossProduct(q-=p)/rCrossS;
      float u = r.CrossProduct(q)/rCrossS;
      if(0 <= u && u <= 1 && 0 <= t && t <= 1)
      {
        *retpoint = (p+=(r*=t));
        return true;
      }
      return false;
    }
    inline T DistanceToPointSquared(const VEC& point) const { return PointLineDistSqr(point.x, point.y, x1, y1, x2, y2); }
    inline T DistanceToPointSquared(T X, T Y) const { return PointLineDistSqr(X, Y, x1, y1, x2, y2); }
    inline T DistanceToPoint(const VEC& point) const { return PointLineDist(point.x, point.y, x1, y1, x2, y2); }
    inline T DistanceToPoint(T X, T Y) const { return PointLineDist(X, Y, x1, y1, x2, y2); }
    inline VEC ParametricPoint(float s) const { return pos1 + (pos2-pos1)*s; }
    inline psLineT<T>& Rotate(float r, VEC center) { return Rotate(r, center.x, center.y); }
    inline psLineT<T>& Rotate(float r, float x, float y) { pos1.Rotate(r, x, y); pos2.Rotate(r, x, y); return *this; }

    static inline bool LineLineIntersect(T x1, T y1, T x2, T y2, T X1, T Y1, T X2, T Y2)
    {
      T da = X2-X1;
      T db = Y2-Y1;
      T dx = x2-x1;
      T dy = y2-y1;
      T s = (dx*(Y1-y1)+dy*(x1-X1))/(da*dy-db*dx);
      T t = (da*(y1-Y1)+db*(X1-x1))/(db*dx-da*dy);
      return (s >= 0 && s <= 1 && t >= 0 && t <= 1);
    }

    static inline bool RectLineIntersect(T X1, T Y1, T X2, T Y2, const T(&rect)[4])
    {
      T m = X2-X1;
      if(m==(T)0) //this is a special case
      {
        return !(X1 < rect[0] || X2 > rect[2] || (Y1 < rect[1] && Y2 < rect[1]) || (Y1 > rect[3] && Y2 < rect[3]));
      }
      m = (Y2-Y1) / m; //this is a precaution to prevent divide by zero
      //T m = (Y2-Y1) / (X2-X1);
      T c = Y1 -(m*X1);
      T ti, bi, tp, bp;

      if(m>0) // if the line is going up from right to left then the top intersect point is on the left
      {
        ti = (m*rect[0]+c);
        bi = (m*rect[2]+c);
      } else // otherwise it's on the right
      {
        ti = (m*rect[2]+c);
        bi = (m*rect[0]+c);
      }

      if(Y1<Y2) // work out the top and bottom extents for the triangle
      {
        tp = Y1;
        bp = Y2;
      } else
      {
        tp = Y2;
        bp = Y1;
      }

      ti = ti>tp ? ti : tp;
      bi = bi<bp ? bi : bp;

      return (ti<bi) && (!((bi<rect[1]) || (ti>rect[3])));

      //return IntersectVertLine(rect.left,rect.top,rect.bottom) || //This method is almost 8 times slower
      //  IntersectVertLine(rect.right,rect.top,rect.bottom) || 
      //  IntersectHorzLine(rect.left,rect.right,rect.top) || 
      //  IntersectHorzLine(rect.left,rect.right,rect.bottom);
    }

    template<class U>
    inline psLineT& operator=(const psLineT<U>& other) { p1=other.p1; p2=other.p2; }

    union {
      struct {
        VEC p1;
        VEC p2;
      };
      struct {
        T x1;
        T y1;
        T x2;
        T y2;
      };
    };
  };

  typedef psLineT<float> psLine; //default typedef
  typedef psLineT<int> psLinei;
  typedef psLineT<double> psLined;
  typedef psLineT<short> psLines;
  typedef psLineT<long> psLinel;
  typedef psLineT<uint32_t> psLineiu;
  typedef psLineT<uint16_t> psLinesu;
  typedef psLineT<unsigned long> psLinelu;

  // Represents a 3D line
  template <class T>
  struct BSS_COMPILER_DLLEXPORT psLine3DT
  {
    typedef bss_util::Vector<T, 3> VEC;

    inline psLine3DT() {} //The following constructors allow for implicit conversion between types
    template<class U>
    inline psLine3DT(const psLine3DT<U>& other) : p1(other.p1), p2(other.p2) {}
    inline psLine3DT(T X1, T Y1, T Z1, T X2, T Y2, T Z2) : x1(X1), y1(Y1), z1(Z1), x2(X2), y2(Y2), z2(Z2) {}
    inline psLine3DT(const VEC& P1, T X2, T Y2, T Z2) : p1(P1), x2(X2), y2(Y2), z2(Z2) {}
    inline psLine3DT(const VEC& P1, const VEC& P2) : p1(P1), p2(P2) {}

    inline VEC ParametricPoint(float s) const { return pos1 + (pos2-pos1)*s; }

    template<class U>
    inline psLine3DT& operator=(const psLine3DT<U>& other) { p1=other.p1; p2=other.p2; }
    inline operator psLineT<T>() const { return psLineT<T>(p1.xy, p2.xy); }

    union {
      struct {
        VEC p1;
        VEC p2;
      };
      struct {
        T x1;
        T y1;
        T z1;
        T x2;
        T y2;
        T z2;
      };
    };
  };

  typedef psLine3DT<float> psLine3D; //default typedef
  typedef psLine3DT<int> psLine3Di;
  typedef psLine3DT<double> psLine3Dd;
  typedef psLine3DT<short> psLine3Ds;
  typedef psLine3DT<long> psLine3Dl;
  typedef psLine3DT<uint32_t> psLine3Diu;
  typedef psLine3DT<uint16_t> psLine3Dsu;
  typedef psLine3DT<unsigned long> psLine3Dlu;
}

#endif