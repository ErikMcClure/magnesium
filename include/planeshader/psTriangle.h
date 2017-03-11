// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __TRIANGLE_H__PS__
#define __TRIANGLE_H__PS__

#include "psLine.h"

namespace planeshader {
  // Represents a geometric triangle
  template <class T>
  struct BSS_COMPILER_DLLEXPORT psTriangleT
  {
    typedef bss_util::Vector<T, 2> VEC;
    inline psTriangleT() { }
    template<class U>
    inline psTriangleT(const cTriangleT<U>& other) { for(int i = 0; i < 3; ++i) p[i] = other.p[i]; }
    inline T Area() const { return (p[1]-p[0]).CrossProduct(p[2]-p[0])/((T)2); }
    inline bool IntersectRect(const psRectT<T>& other) const
    { //Here we pick the nearest point on the triangle to the center of the rectangle, then pick the nearest point on the rectangle to that given triangle and construct lines out of them.
      VEC comppoint((other.left+other.right)/(T)2, (other.top+other.bottom)/(T)2);
      uint8_t nearptr = NearestPoint(comppoint);
      uint8_t corner=0;
      T lastdist=p[nearptr].GetDistanceSquared(other.left, other.top);
      T hold=0;
      if(hold=p[nearptr].GetDistanceSquared(other.right, other.top)<lastdist) {
        lastdist=hold; corner=1;
      }
      if(hold=p[nearptr].GetDistanceSquared(other.left, other.bottom)<lastdist) {
        lastdist=hold; corner=2;
      }
      if(p[nearptr].GetDistanceSquared(other.right, other.bottom)<lastdist) {
        corner=3;
      }

      psLineT<T> aline(p[nearptr], p[nearptr==0?2:nearptr-1]);
      psLineT<T> bline(p[nearptr], p[nearptr==2?0:nearptr+1]);

      switch(corner)
      {
      default:
      case 0:
        return IntersectPoint(other.left, other.top) || _LineTriRectCollide(aline, bline, other.left, other.right, other.top, other.bottom);
        //aline.IntersectVertLine(other.left,other.top,other.bottom);
        //aline.IntersectHorzLine(other.top,other.left,other.right);
        //bline.IntersectVertLine(other.left,other.top,other.bottom);
        //bline.IntersectHorzLine(other.top,other.left,other.right);
      case 1:
        return IntersectPoint(other.right, other.top) || _LineTriRectCollide(aline, bline, other.right, other.left, other.top, other.bottom);
      case 2:
        return IntersectPoint(other.left, other.bottom) || _LineTriRectCollide(aline, bline, other.left, other.right, other.bottom, other.top);
      case 3:
        return IntersectPoint(other.right, other.bottom) || _LineTriRectCollide(aline, bline, other.right, other.left, other.bottom, other.top);
      }
    }
    bool IntersectTriangle(const cTriangleT<T>& other) const
    {
      for(int i = 0; i < 3; ++i)
      {
        int k = (i+1)%3;
        for(int j = 0; j < 3; ++j)
        {
          if(psLineT<T>::LineLineIntersect(p[i].x, p[i].y, p[k].x, p[k].y, other.p[j].x, other.p[j].y, other.p[(j+1)%3].x, other.p[(j+1)%3].y))
            return true;
        }
      }
    }
    inline bool IntersectCircle(const VEC& pos, T radius) const
    {
      psLineT<T> start(p[0], p[1]);

      if(start.DistanceToPointSquared(other)<=(radius*=radius))
        return true;

      start.x1=p[1].x;
      start.x2=p[2].x;
      start.y1=p[1].y;
      start.y2=p[2].y;

      if(start.DistanceToPointSquared(other)<=radius)
        return true;

      start.x1=p[2].x;
      start.x2=p[0].x;
      start.y1=p[2].y;
      start.y2=p[0].y;

      if(start.DistanceToPointSquared(other)<=radius)
        return true;
      return false;
    }
    inline bool IntersectPoint(const VEC& pos) const { return IntersectPoint(pos.x, pos.y); }
    inline bool IntersectPoint(T X, T Y) const //Uses the standard barycentric method to figure out point inclusion
    {
      VEC v0(p[2]);
      VEC v1(p[1]);
      VEC v2(X, Y);
      v0 -= p[0];
      v1 -= p[0];
      v2 -= p[0];

      // Compute dot products
      T dot00 = v0.DotProduct(v0);
      T dot01 = v0.DotProduct(v1);
      v0.x = v0.DotProduct(v2);
      v0.y = v1.DotProduct(v1);
      v1.x = v1.DotProduct(v2);

      // Compute barycentric coordinates
      v1.y = 1 / (dot00 * v0.y - dot01 * dot01);
      v2.x = (v0.y * v0.x - dot01 * v1.x) * v1.y;
      v2.y = (dot00 * v1.x - dot01 * v0.x) * v1.y;

      // Check if point is in triangle
      return (v2.x > 0) && (v2.y > 0) && (v2.x + v2.y < 1);
    }

    template<class U>
    inline cTriangleT<T>& operator=(const cTriangleT<U>& other) { for(int i = 0; i < 3; ++i) p[i] = other.p[i]; return *this; } //forces conversion

    VEC p[3];

  protected:
    inline uint8_t NearestPoint(const VEC& pos) const
    {
      T lastdist=pos.GetDistanceSquared(p[0]);
      T store;
      if(store=pos.GetDistanceSquared(p[1])<lastdist)
        return pos.GetDistanceSquared(p[2])<store?2:1;
      return pos.GetDistanceSquared(p[2])<lastdist?1:0;
    }
    static inline bool _LineTriRectCollide(const psLineT<T>& aline, const psLineT<T>& bline, T a, T b, T c, T d)
    {
      return (aline.IntersectVertLine(a, c, d) ||
        aline.IntersectHorzLine(c, a, b) ||
        bline.IntersectVertLine(a, c, d) ||
        bline.IntersectHorzLine(c, a, b))
    }
  };

  typedef psTriangleT<float> psTriangle; //default typedef
  typedef psTriangleT<int> psTrianglei;
  typedef psTriangleT<double> psTriangled;
  typedef psTriangleT<short> psTriangles;
  typedef psTriangleT<long> psTrianglel;
  typedef psTriangleT<uint32_t> psTriangleiu;
  typedef psTriangleT<uint16_t> psTrianglesu;
  typedef psTriangleT<unsigned long> psTrianglelu;
}

#endif