// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __RECT_H__PS__
#define __RECT_H__PS__

#include "psCircle.h"
#include "bss-util/bss_sse.h"

namespace planeshader {
  template<class T>
  struct BSS_COMPILER_DLLEXPORT psRectT
  {
    typedef bss_util::Vector<T, 2> VEC;
    inline psRectT() {} //The following constructors allow for implicit conversion between rect types
    inline psRectT(const psRectT<T>& other) : left(other.left), top(other.top), right(other.right), bottom(other.bottom) {}
    template<class U>
    inline psRectT(const psRectT<U>& other) : left((T)other.left), top((T)other.top), right((T)other.right), bottom((T)other.bottom) {}
    inline psRectT(T Left, T Top, T Right, T Bottom) : left(Left), top(Top), right(Right), bottom(Bottom) {}
    inline psRectT(T X, T Y, const VEC& dim) : left(X), top(Y), right(X+dim.x), bottom(Y+dim.y) {}
    inline psRectT(const VEC& pos, const VEC& dim) : left(pos.x), top(pos.y), right(pos.x+dim.x), bottom(pos.y+dim.y) {}
    inline explicit psRectT(const VEC& v) : left(v.x), top(v.y), right(v.x), bottom(v.y) {}
    inline explicit psRectT(const sseVecT<T>& v) { v>>_ltrbarray; }
    inline explicit psRectT(const T(&rectarray)[4]) : left(rectarray[0]), top(rectarray[1]), right(rectarray[2]), bottom(rectarray[3]) {}
    inline T Area() const { return (right-left)*(bottom-top); }
    inline bool IntersectPoint(T x, T y) const { return (x >= left && y >= top && x < right && y < bottom); }
    inline bool IntersectPoint(const VEC& point) const { return (point.x >= left && point.y >= top && point.x < right && point.y < bottom); }
    inline bool IntersectCircle(const psCircleT<T>& other) const { return psCircleT<T>::CircleRectIntersect(other.x, other.y, other.r, _ltrbarray); }
    inline bool IntersectCircle(const VEC& pos, T R) const { return psCircleT<T>::CircleRectIntersect(pos.x, pos.y, R, _ltrbarray); }
    inline bool IntersectCircle(T X, T Y, T R) const { return psCircleT<T>::CircleRectIntersect(X, Y, R, _ltrbarray); }
    inline bool IntersectRect(const psRectT<T>& other) const { return IntersectRect(other.left, other.top, other.right, other.bottom); }
    inline bool IntersectRect(const T(&rectarray)[4]) const { return IntersectRect(rectarray[0], rectarray[1], rectarray[2], rectarray[3]); }
    inline bool IntersectRect(T Left, T Top, T Right, T Bottom) const { return left <= Right && top <= Bottom && right >= Left && bottom >= Top; }
    inline bool WithinRect(const psRectT<T>& other) const { return (left >= other.left && top >= other.top && right <= other.right && bottom <= other.bottom); }
    inline bool IntersectEllipse(T X, T Y, T A, T B) const { T s=A/B; return psCircleT<T>::CircleRectIntersect(X, Y*s, A, left, top*s, right, bottom*s); }
    inline psRectT<T> Abs() const { return psRectT<T>(abs(left), abs(top), abs(right), abs(bottom)); }
    inline psCircleT<T> ToCircle() const { VEC center=topleft+bottomright; center/=(T)2; return psCircleT<T>(center, (T)PTR_DISTANCE(center.x, center.y, topleft.x, topleft.y)); }
    inline psCircleT<T> ToCircleSameArea() const { VEC center=topleft+bottomright; center/=(T)2; return psCircleT<T>(center, (T)bss_util::FastSqrt(Area()/PI_DOUBLE)); }
    inline VEC GetCenter() const { VEC center=topleft; center+=bottomright; center/=(T)2; return center; }

    inline psRectT<T> operator +(const psRectT<T>& other) const { return psRectT<T>(left + other.left, top + other.top, right + other.right, bottom + other.bottom); }
    inline psRectT<T> operator -(const psRectT<T>& other) const { return psRectT<T>(left - other.left, top - other.top, right - other.right, bottom - other.bottom); }
    inline psRectT<T> operator *(const psRectT<T>& other) const { return psRectT<T>(left * other.left, top * other.top, right * other.right, bottom * other.bottom); }
    inline psRectT<T> operator /(const psRectT<T>& other) const { return psRectT<T>(left / other.left, top / other.top, right / other.right, bottom / other.bottom); }
    inline psRectT<T> operator +(const VEC& other) const { return psRectT<T>(left + other.x, top + other.y, right + other.x, bottom + other.y); }
    inline psRectT<T> operator -(const VEC& other) const { return psRectT<T>(left - other.x, top - other.y, right - other.x, bottom - other.y); }
    inline psRectT<T> operator *(const VEC& other) const { return psRectT<T>(left * other.x, top * other.y, right * other.x, bottom * other.y); }
    inline psRectT<T> operator /(const VEC& other) const { return psRectT<T>(left / other.x, top / other.y, right / other.x, bottom / other.y); }
    inline psRectT<T> operator +(const T other) const { return psRectT<T>(left + other, top + other, right + other, bottom + other); }
    inline psRectT<T> operator -(const T other) const { return psRectT<T>(left - other, top - other, right - other, bottom - other); }
    inline psRectT<T> operator *(const T other) const { return psRectT<T>(left * other, top * other, right * other, bottom * other); }
    inline psRectT<T> operator /(const T other) const { return psRectT<T>(left / other, top / other, right / other, bottom / other); }

    inline psRectT<T>& operator +=(const psRectT<T>& other) { left+=other.left; top+=other.top; right+=other.right; bottom+=other.bottom; return *this; }
    inline psRectT<T>& operator -=(const psRectT<T>& other) { left-=other.left; top-=other.top; right-=other.right; bottom-=other.bottom; return *this; }
    inline psRectT<T>& operator *=(const psRectT<T>& other) { left*=other.left; top*=other.top; right*=other.right; bottom*=other.bottom; return *this; }
    inline psRectT<T>& operator /=(const psRectT<T>& other) { left/=other.left; top/=other.top; right/=other.right; bottom/=other.bottom; return *this; }
    inline psRectT<T>& operator +=(const VEC& other) { left+=other.x; top+=other.y; right+=other.x; bottom+=other.y; return *this; }
    inline psRectT<T>& operator -=(const VEC& other) { left-=other.x; top-=other.y; right-=other.x; bottom-=other.y; return *this; }
    inline psRectT<T>& operator *=(const VEC& other) { left*=other.x; top*=other.y; right*=other.x; bottom*=other.y; return *this; }
    inline psRectT<T>& operator /=(const VEC& other) { left/=other.x; top/=other.y; right/=other.x; bottom/=other.y; return *this; }
    inline psRectT<T>& operator +=(const T other) { left+=other; top+=other; right+=other; bottom+=other; return *this; }
    inline psRectT<T>& operator -=(const T other) { left-=other; top-=other; right-=other; bottom-=other; return *this; }
    inline psRectT<T>& operator *=(const T other) { left*=other; top*=other; right*=other; bottom*=other; return *this; }
    inline psRectT<T>& operator /=(const T other) { left/=other; top/=other; right/=other; bottom/=other; return *this; }
    //inline psRectT<T>& operator +=(const psRectT<T>& other) { (sseVecT<T>(_ltrbarray)+sseVecT<T>(other._ltrbarray))>>_ltrbarray; return *this; }
    //inline psRectT<T>& operator -=(const psRectT<T>& other) { (sseVecT<T>(_ltrbarray)-sseVecT<T>(other._ltrbarray))>>_ltrbarray; return *this; }
    //inline psRectT<T>& operator *=(const psRectT<T>& other) { (sseVecT<T>(_ltrbarray)*sseVecT<T>(other._ltrbarray))>>_ltrbarray; return *this; }
    //inline psRectT<T>& operator /=(const psRectT<T>& other) { (sseVecT<T>(_ltrbarray)/sseVecT<T>(other._ltrbarray))>>_ltrbarray; return *this; }
    //inline psRectT<T>& operator +=(const T other) { (sseVecT<T>(_ltrbarray)+sseVecT<T>(other))>>_ltrbarray; return *this; }
    //inline psRectT<T>& operator -=(const T other) { (sseVecT<T>(_ltrbarray)-sseVecT<T>(other))>>_ltrbarray; return *this; }
    //inline psRectT<T>& operator *=(const T other) { (sseVecT<T>(_ltrbarray)*sseVecT<T>(other))>>_ltrbarray; return *this; }
    //inline psRectT<T>& operator /=(const T other) { (sseVecT<T>(_ltrbarray)/sseVecT<T>(other))>>_ltrbarray; return *this; }

    inline psRectT<T> operator -() const { return psRectT<T>(-left, -top, -right, -bottom); }

    inline bool operator !=(const psRectT<T> &other) const { return (left != other.left) || (top != other.top) || (right != other.right) || (bottom != other.bottom); }
    inline bool operator ==(const psRectT<T> &other) const { return (left == other.left) && (top == other.top) && (right == other.right) && (bottom == other.bottom); }
    inline bool operator !=(const T other) const { return (left != other) || (top != other) || (right != other) || (bottom != other); }
    inline bool operator ==(const T other) const { return (left == other) && (top == other) && (right == other) && (bottom == other); }

    inline bool operator >(const psRectT<T> &other) const { return (left > other.left) || ((left == other.left) && ((top > other.top) || ((top == other.top) && ((right > other.right) || ((right == other.right) && (bottom > other.bottom)))))); }
    inline bool operator <(const psRectT<T> &other) const { return (left < other.left) || ((left == other.left) && ((top < other.top) || ((top == other.top) && ((right < other.right) || ((right == other.right) && (bottom < other.bottom)))))); }
    inline bool operator >=(const psRectT<T> &other) const { return !operator<(other); }
    inline bool operator <=(const psRectT<T> &other) const { return !operator>(other); }

    inline psRectT<T>& operator =(const sseVecT<T>& _right) { _right>>_ltrbarray; return *this; }
    inline psRectT<T>& operator =(const psRectT<T>& _right) { left = _right.left; top = _right.top; right = _right.right;  bottom = _right.bottom; return *this; }
    template<class U>
    inline psRectT<T>& operator =(const psRectT<U>& _right) { left = (T)_right.left; top = (T)_right.top; right = (T)_right.right; bottom = (T)_right.bottom; return *this; }

    union {
      struct {
        VEC topleft;
        VEC bottomright;
      };
      struct {
        T left;
        T top;
        T right;
        T bottom;
      };
      T _ltrbarray[4];
    };

    BSS_FORCEINLINE VEC GetDimensions() const { return VEC(right-left, bottom-top); }
    BSS_FORCEINLINE psRectT<T> Inflate(T amount) const { return psRectT<T>(left-amount, top-amount, right+amount, bottom+amount); }
    inline psRectT<T> ZProject(T Z) { Z+=(T)1; psRectT<T> ret(*this); ret.topleft/=Z; ret.bottomright/=Z; return ret; }
    BSS_FORCEINLINE sseVecT<T> ToSSE() const { return sseVecT<T>(_ltrbarray); }

    // Returns a rectangle gaurenteed to have valid LTRB even if left and right or top and down are swapped in this one
    BSS_FORCEINLINE psRectT<T> EnforceLTRB() const { return psRectT<T>(bssmin(left, right), bssmin(top, bottom), bssmax(left, right), bssmax(top, bottom)); }
    //returns a new rectangle that includes the given point.
    BSS_FORCEINLINE psRectT<T> ExpandTo(const VEC& point) const { return psRectT<T>(bssmin(left, point.x), bssmin(top, point.y), bssmax(right, point.x), bssmax(bottom, point.y)); }
    BSS_FORCEINLINE psRectT<T> ExpandTo(const psRectT<T>& rect) const { return psRectT<T>(bssmin(left, rect.left), bssmin(top, rect.top), bssmax(right, rect.right), bssmax(bottom, rect.bottom)); }
    //Finds the rectangle of intersection
    inline psRectT<T> GenerateIntersection(const psRectT<T>& other) const { if(!IntersectRect(other)) return psRectT<T>(left, top, left, top); return psRectT<T>(bssmax(left, other.left), bssmax(top, other.top), bssmin(right, other.right), bssmin(bottom, other.bottom)); }
  };

  typedef psRectT<FNUM> psRect; //Default typedef
  typedef psRectT<int> psRecti;
  typedef psRectT<double> psRectd;
  typedef psRectT<short> psRects;
  typedef psRectT<long> psRectl;
  typedef psRectT<uint32_t> psRectiu;
  typedef psRectT<uint16_t> psRectsu;
  typedef psRectT<unsigned long> psRectlu;

  psRect const RECT_ZERO(0, 0, 0, 0);
  psRect const RECT_UNITRECT(0, 0, 1, 1);

  // Global operator overloads 
  template<typename T>
  inline bool operator==(const T _Left, const psRectT<T>& _Right) { return _Right==_Left; }
  template<typename T>
  inline bool operator!=(const T _Left, const psRectT<T>& _Right) { return _Right!=_Left; }

  // The psRectRotateT class is a rotated rectagle 
  template<class T>
  struct BSS_COMPILER_DLLEXPORT psRectRotateT : public psRectT<T>
  {
    inline psRectRotateT() {} //The following constructors allow for implicit conversion between rect types
    template<class U>
    inline psRectRotateT(const psRectRotateT<U>& other) : psRectT(other), rotation(other.rotation), pivot(other.pivot) {}
    inline psRectRotateT(T Left, T Top, T Right, T Bottom, T rotation, const VEC& pivot=VEC_ZERO) : psRectT(Left, Top, Right, Bottom), rotation(rotation), pivot(pivot) { }
    inline psRectRotateT(T X, T Y, const VEC& dim, T rotation, const VEC& pivot=VEC_ZERO) : rotation(rotation), pivot(pivot) { left = X; right = X+dim.x; top = Y; bottom = Y+dim.y; }
    inline psRectRotateT(const psRectT<T>& rect, T rotation=0.0f, const VEC& pivot=VEC_ZERO) : psRectT(rect), rotation(rotation), pivot(pivot) { }
    inline psRectRotateT(const VEC& pos, const VEC& dim, T rotation, const VEC& pivot=VEC_ZERO) : rotation(rotation), pivot(pivot) { left = pos.x; right = pos.x+dim.x; top = pos.y; bottom = pos.y+dim.y; }
    inline bool IntersectPoint(T x, T y) const
    {
      if(!bss_util::fsmall(rotation))
        VEC::RotatePoint(x, y, -rotation, pivot.x+left, pivot.y+top);
      return psRectT<T>::IntersectPoint(x, y);
    }
    inline bool IntersectPoint(const VEC& point) const { return IntersectPoint(point.x, point.y); }

    // This builds an Axis-Aligned Bounding Box from the rotated rectangle, rotated around a pivot RELATIVE TO THE TOPLEFT CORNER OF THE RECTANGLE. It does this by representing the box by the distance from an arbitrary rotation axis, and rotating that axis. 
    inline psRectT<T> BuildAABB() const
    {
      if(bss_util::fsmall(rotation))
        return *this;
      float c = (cos(rotation));
      float s = (sin(rotation));
      float ex = ((right-left)*0.5f);
      float ey = ((bottom-top)*0.5f);
      float x_radius = ex * abs(c) + ey * abs(s);
      float y_radius = ex * abs(s) + ey * abs(c);

      //Rotate pivot around origin. ex and ey equal the center of the rectangle minus the topleft corner. That is to say, psVec((r+l)/2 - l, (b+t)/2 - t) -> psVec((r+l)/2 - 2*l/2, (b+t)/2 - 2*t/2) -> psVec((r+l-2l)/2, (b+t-2t)/2) -> psVec((r-l)/2, (b-t)/2) == psVec(ex,ey) since ex and ey were already set to that.
      ex-=pivot.x;
      ey-=pivot.y;
      float tmpex=ex;
      ex = ex * c - ey * s;
      ey = ey * c + tmpex * s;
      ex+=pivot.x+left;
      ey+=pivot.y+top;

      return psRectT<T>(ex - x_radius, ey - y_radius, ex + x_radius, ey + y_radius);
    }

    inline psRectRotateT<T>& operator =(const psRectT<T>& _right) { psRectT<T>::operator =(_right); return *this; }
    inline psRectRotateT<T>& operator =(const psRectRotateT<T>& _right) { if(&_right!=this) { psRectT<T>::operator =(_right); rotation=_right.rotation; pivot=_right.pivot; } return *this; }
    BSS_FORCEINLINE psRectRotateT<T> EnforceLTRB() const { return psRectRotateT<T>(psRectT<T>::EnforceLTRB(), rotation, pivot); }
    BSS_FORCEINLINE psRectRotateT<T> Inflate(T amount) const { return psRectRotateT<T>(psRectT<T>::Inflate(amount), rotation, pivot); }
    inline psRectRotateT RelativeTo(const psVec& pos, float r, const psVec& p) const
    {
      psVec dim = { right - left, bottom - top };
      psRectRotateT ret(*this, r + rotation, pivot);
      if(r != 0.0f)
        psVec::RotatePoint(ret.left, ret.top, r, ret.left - pivot.x, ret.top - pivot.y);
      ret.topleft += pos + p;
      ret.bottomright = ret.topleft + dim;
      return ret;
    }

    T rotation;
    VEC pivot; //this is what its rotated around
  };

  typedef psRectRotateT<FNUM> psRectRotate; //Default typedef
  typedef psRectRotateT<int> psRectRotatei;
  typedef psRectRotateT<double> psRectRotated;
  typedef psRectRotateT<short> psRectRotates;
  typedef psRectRotateT<long> psRectRotatel;
  typedef psRectRotateT<uint32_t> psRectRotateiu;
  typedef psRectRotateT<uint16_t> psRectRotatesu;
  typedef psRectRotateT<unsigned long> psRectRotatelu;

  // The psRectRotateZT class is a rotated rectangle with a Z coordinate for the sole purpose of storing that Z coordinate in the culling rect
  template<class T>
  struct BSS_COMPILER_DLLEXPORT psRectRotateZT : public psRectRotateT<T>
  {
    inline psRectRotateZT() {}
    template<class U>
    inline psRectRotateZT(const psRectRotateZT<U>& other) : psRectRotateT<T>(other), z((T)other.z) {}
    inline psRectRotateZT(T Left, T Top, T Right, T Bottom, T rotation, const VEC& pivot=VEC_ZERO, FNUM Z=0.0f) : psRectRotateT<T>(Left, Top, Right, Bottom, rotation, pivot), z(Z) { }
    inline psRectRotateZT(const psRectRotateT<T>& rect, FNUM Z=0.0f) : psRectRotateT<T>(rect), z(Z) { }

    inline psRectRotateZT<T>& operator =(const psRectT<T>& _right) { psRectT<T>::operator =(_right); return *this; }
    inline psRectRotateZT<T>& operator =(const psRectRotateT<T>& _right) { psRectRotateT<T>::operator =(_right); return *this; }
    inline psRectRotateZT<T>& operator =(const psRectRotateZT<T>& _right) { psRectRotateT<T>::operator =(_right); z=_right.z; return *this; }
    BSS_FORCEINLINE psRectRotateZT<T> EnforceLTRB() const { return psRectRotateZT<T>(psRectRotateT<T>::EnforceLTRB(), z); }
    BSS_FORCEINLINE psRectRotateZT<T> Inflate(T amount) const { return psRectRotateZT<T>(psRectRotateT<T>::Inflate(amount), z); }
    BSS_FORCEINLINE psRectRotateZT RelativeTo(const psVec3D& pos, float r, const psVec& p) const { return psRectRotateZT<T>(psRectRotateT<T>::RelativeTo(pos.xy, r, p), pos.z + z); }

    T z;
  };

  typedef psRectRotateZT<FNUM> psRectRotateZ; // Why would you ever use anything other than the FNUM typedef on this struct?
}

#endif