// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __CIRCLE_H__PS__
#define __CIRCLE_H__PS__

#include "psVec.h"

namespace planeshader {
  // This is a circle. Circles are round.
  template <class T>
  struct BSS_COMPILER_DLLEXPORT psCircleT
  {
    typedef bss_util::Vector<T, 2> VEC;
    inline psCircleT() {} //The following constructors allow for implicit conversion between types
    template<class U>
    inline psCircleT(const psCircleT<U>& other) : x((T)other.x), y((T)other.y), r((T)other.r) {}
    inline psCircleT(T X, T Y, T R) : x(X), y(Y), r(R) {}
    inline psCircleT(const VEC& Pos, T R) : x(Pos.x), y(Pos.y), r(R) {}
    inline T Area() const { return (T)(PI*r*r); }
    inline bool IntersectPoint(T X, T Y) const { return bss_util::distsqr<T>(x, y, X, Y)<=(r*r); }
    inline bool IntersectPoint(const VEC& point) const { return IntersectPoint(point.x, point.y); }
    inline bool IntersectCircle(const psCircleT& other) const { return IntersectCircle(other.x, other.y, other.r); }
    inline bool IntersectCircle(const VEC& pos, T R) const { return IntersectCircle(pos.x, pos.y, R); }
    inline bool IntersectCircle(T X, T Y, T R) const { T tr=r+R; return bss_util::distsqr<T>(x, y, X, Y)<=(tr*tr); }
    inline void IntersectCircle(const psCircleT& other, const VEC(&output)[2]) const { return _IntersectCircle(other.x, other.y, other.r, output[0]._xyarray); }
    inline void IntersectCircle(const VEC& pos, T R, const VEC(&output)[2]) const { return _IntersectCircle(pos.x, pos.y, R, output[0]._xyarray); }
    inline void IntersectCircle(T X, T Y, T R, const VEC(&output)[2]) const { return _IntersectCircle(X, Y, R, output[0]._xyarray); }
    inline void IntersectCircle(const psCircleT& other, T(&output)[4]) const { return _IntersectCircle(other.x, other.y, other.r, output); }
    inline void IntersectCircle(const VEC& pos, T R, T(&output)[4]) const { return _IntersectCircle(pos.x, pos.y, R, output); }
    inline void IntersectCircle(T X, T Y, T R, T(&output)[4]) const { _IntersectCircle(X, Y, R, output); }
    inline bool WithinCircle(const psCircleT& other) const { return WithinCircle(other.x, other.y, other.r); }
    inline bool WithinCircle(const VEC& pos, T R) const { return WithinCircle(pos.x, pos.y, R); }
    inline bool WithinCircle(T X, T Y, T R) const { T tr=R-r; return (r>=R || bss_util::distsqr(X, Y, x, y)>=(tr*tr)); }
    inline bool IntersectLineInf(T X1, T Y1, T X2, T Y2) const { return pos.LineInfDistanceSqr(X1, Y1, X2, Y2)<=(r*r); }
    inline bool IntersectLine(T X1, T Y1, T X2, T Y2) const { return pos.LineDistanceSqr(X1, Y1, X2, Y2)<=(r*r); }
    inline bool IntersectRect(const T(&rect)[4]) const { return CircleRectIntersect(x, y, r, rect); }
    inline bool IntersectRect(T left, T top, T right, T bottom) const { return _CircleRectIntersect(x, y, r, left, top, right, bottom); }
    inline bool IntersectEllipse(T X, T Y, T A, T B) const { return CircleEllipseIntersect(X, Y, A, B, x, y, r); }
    inline void NearestPoint(T X, T Y, T& outX, T& outY) const { CircleNearestPoint(X, Y, x, y, r, outX, outY); }
    inline void NearestPoint(const VEC& pos, T& outX, T& outY) const { CircleNearestPoint(pos.x, pos.y, x, y, r, outX, outY); }

    inline psCircleT& operator=(const psCircleT<T>& other) { r=other.r; x=other.x; y=other.y; }
    template<class U>
    inline psCircleT& operator=(const psCircleT<U>& other) { r=(T)other.r; x=(T)other.x; y=(T)other.y; }

    static inline void CircleNearestPoint(T X, T Y, T x, T y, T r, T& outX, T& outY) { T tx=X-x; T ty=Y-y; T s=r/bss_util::FastSqrt((tx*tx)+(ty*ty)); outX=tx*s + x; outY=ty*s + y; }
    static inline bool CircleRectIntersect(T X, T Y, T R, const T(&rect)[4]) { return _CircleRectIntersect(X, Y, R, rect[0], rect[1], rect[2], rect[3]); }
    static inline bool CircleRectIntersect(T X, T Y, T R, T left, T top, T right, T bottom) { return _CircleRectIntersect(X, Y, R, left, top, right, bottom); }
    static inline bool CircleEllipseIntersect(T X, T Y, T A, T B, T x, T y, T r)
    {
      T dx=x-X; //Centers the ellipse/circle pair such that the ellipse is at the origin
      T dy=y-Y;
      EllipseNearestPoint(A, B, dx, dy, X, Y);
      return bss_util::distsqr(X, Y, dx, dy)<(r*r);
    }

    static inline bool FastCircleEllipseIntersect(T X, T Y, T A, T B, T x, T y, T r) //Fast circle-ellipse collision (5-10% error)
    {
      T tx=(x-X); T ty=(y-Y);
      T tx2=tx*tx; T ty2=ty*ty;
      T A2 = A*A; T B2 = B*B;
      T d = tx2+ty2; T r2=r*r;

      if((d<r2) || ((tx2/A2) + (ty2/B2) <= 1)) return true; //checks to see if the ellipse center is inside the circle or vice versa

      T U1; T U2; T Vx; T Vy; //If that fails we build a line between the foci of the ellipse
      if(B>A) { U1=tx; tx=ty; ty=U1; U1=A; A=B; B=U1; }//if B is larger then A we swap the axes
      T ratio = (A-B + (2*B/(1+(ty/A)*B)))/(A+B);
      U1=A/ratio;
      U2=-U1; //the y coordinates of the line are always 0

      NearestPointToLine(tx, ty, U1, 0, U2, 0, Vx, Vy); //compiler should optimize the zeros out.
      Vx*=ratio;
      if(bss_util::distsqr(Vx, Vy, tx, ty)<r2) return true; //if the nearest point on the line is inside the circle, the circle intersects the line, so it must intersect the ellipse
      CircleNearestPoint(Vx, Vy, tx, ty, r, U1, U2); //Failing that we get the nearest point on the circle to the line and check to see if its in the ellipse
      return IntersectEllipse(0, 0, A, B, U1, U2);
    }
    //static inline bool CircleEllipseIntersect(T c, T e, T d, T f, T a, T b, T r)
    //{
    //  a -= c;
    //  b -= e;
    //  T m = (1 + 1/(d*d));
    //  //T n = -((2*a) + ((2*c)/(d*d)));
    //  T n = -(2*a);
    //  T o = (1 + 1/(f*f));
    //  //T p = -((2*b) + ((2*e)/(f*f)));
    //  T p = -(2*b);
    //  T z = (1 + (r*r) - (a*a) - (b*b) - ((c*c)/(d*d)) - ((e*e)/(f*f)) + (m*(n/(2*m))*(n/(2*m))) + (o*(p/(2*o))*(p/(2*o))));
    //  return z >= 0;
    //}

    T r;
    union {
      struct {
        VEC pos;
      };
      struct {
        T x;
        T y;
      };
    };

  private:
    static inline bool _CircleRectIntersect(T X, T Y, T R, T left, T top, T right, T bottom)
    { //based on a solution from this thread: http://stackoverflow.com/questions/401847/circle-rectangle-collision-detection-intersection
      T rwh=(right-left)/2;
      T rhh=(bottom-top)/2;
      T cx = abs(X - ((right+left)/2));
      T cy = abs(Y - ((bottom+top)/2));

      if(cx > (rwh + R) || cy > (rhh + R)) return false;
      if(cx <= rwh || cy <= rhh) return true;

      cx-=rwh;
      cy-=rhh;
      rwh=(cx*cx) + (cy*cy);
      return rwh <= (R*R);
    }
    inline void _IntersectCircle(T X, T Y, T R, T* output) const //This does not work with an integral type.
    {
      T dx = X - x;
      T dy = Y - y;
      T dsq = (dx*dx) + (dy*dy);
      T d = bss_util::FastSqrt(d);
      T id = ((T)1)/d;

      //We don't check if there's a solution - you should do that before calling this
      //if (d > (r0 + r1)) return;
      //if (d < fabs(r0 - r1)) return;

      T r02 = r*r;
      T a = (r02 - (R*R) + dsq)*0.5f*id;

      T x2 = X + (dx*a*id);
      T y2 = Y + (dy*a*id);
      T h = bss_util::FastSqrt(r02 - (a*a));

      T rx = -dy*h*id;
      T ry = dx*h*id;

      output[0] = x2 + rx;
      output[2] = x2 - rx;
      output[1] = y2 + ry;
      output[3] = y2 - ry;
    }
  };

  typedef psCircleT<float> psCircle; //default typedef
  typedef psCircleT<int> psCirclei;
  typedef psCircleT<double> psCircled;
  typedef psCircleT<short> psCircles;
  typedef psCircleT<long> psCirclel;
  typedef psCircleT<uint32_t> psCircleiu;
  typedef psCircleT<uint16_t> psCirclesu;
  typedef psCircleT<unsigned long> psCirclelu;
}

#endif