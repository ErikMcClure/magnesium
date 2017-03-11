// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __POLYGON_H__PS__
#define __POLYGON_H__PS__

#include "psLine.h"
#include "bss-util/cArray.h"

namespace planeshader {
  template<class T>
  class BSS_COMPILER_DLLEXPORT psPolygonT
  {
  public:
    typedef bss_util::Vector<T, 2> VEC;

    inline psPolygonT(const psPolygonT<T>& copy) : _verts(copy._verts) {}
    inline psPolygonT(psPolygonT<T>&& mov) : _verts(std::move(mov._verts)) {}
    template<class U>
    inline psPolygonT(const psPolygonT<U>& other) : _verts(other._size) { for(uint16_t i=0; i < other._size; ++i) _verts[i]=other._verts[i]; }
    inline psPolygonT(const VEC* vertices, uint16_t num) : _verts(num) { if(_verts!=0) memcpy(_verts, vertices, _size*sizeof(VEC)); }
    template <uint32_t num>
    inline psPolygonT(const VEC(&vertices)[num]) : _verts(num) { if(_verts!=0) memcpy(_verts, vertices, _size*sizeof(VEC)); }
    template<class U>
    inline psPolygonT(const bss_util::Vector<U, 2>* vertices, uint16_t num) : _verts(num) { for(uint16_t i=0; i < _numverts; ++i) _verts[i]=vertices[i]; }
    template <class U, uint32_t num>
    inline psPolygonT(const bss_util::Vector<U, 2>(&vertices)[num]) : _verts(num) { for(uint16_t i=0; i < _numverts; ++i) _verts[i]=vertices[i]; }
    inline const VEC* GetVertices() const { return _verts; }
    inline uint16_t GetNumVerts() const { return _size; }
    inline T Area() const { return PolyArea(_verts, _verts.Capacity()); }

    inline VEC GetCentroid() const
    {
      T A=Area()*((T)6);
      return (PartialCentroid(_verts, _verts.Capacity())/=A);
    }
    inline psRectT<T> GetAABB() const
    {
      return PolyAABB(_verts, _verts.Capacity());
    }
    // Centers the polygon on the centroid 
    inline void Center()
    {
      VEC c=GetCentroid();
      for(uint16_t i=0; i<_numverts; ++i)
        _verts[i]-=c;
    }
    // Assumes convex polygon 
    inline bool ConvexPointIntersection(const VEC& p) const //This is an example of where copy-pasted code is perfectly reasonable
    {
      assert(_numverts>2); //you must be at least a triangle or this is invalid

      //(y - y0)*(x1 - x0) - (x - x0)*(y1 - y0) where x,y is the point being tested and 0 and 1 are vertices on the polygon, returns negative if on the right, 0 if its on the line, and positive if its on the left
      T last = ((p.y - _verts[0].y)*(_verts[_numverts-1].x - _verts[0].x)) - ((p.x - _verts[0].x)*(_verts[_numverts-1].y - _verts[0].y));
      T cur;
      for(uint16_t i=1; i<_numverts; ++i)
      {
        cur=((p.y - _verts[i-1].y)*(_verts[i].x - _verts[i-1].x)) - ((p.x - _verts[i-1].x)*(_verts[i].y - _verts[i-1].y));
        if(cur<0&&last>0 || cur>0&&last<0) return false;//This is written so that being on the line counts as an intersection.
      }

      return true;
    }
    // Works with any simple polygon using the pnpoly test: http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html 
    inline bool PointIntersection(const VEC& p) const
    {
      return PolyIntersectPoint(_verts, _verts.Capacity(), p)!=0;
    }
    // Returns true if this polygon is convex 
    inline bool IsConvex() const
    {
      return PolyIsConvex(_verts, _verts.Capacity());
    }

    inline void SetVertices(const VEC* vertices, uint16_t num)
    {
      if(_verts==vertices)
        return;
      _verts.SetCapacityDiscard(num);
      if((const VEC*)_verts)
        memcpy(_verts, vertices, _size*sizeof(VEC));
    }
    template <uint32_t num>
    inline void SetVertices(const VEC(&vertices)[num]) { SetVertices(vertices, num); }

    inline psPolygonT<T>& operator =(const psPolygonT<T>& right) { _verts = right._verts; return *this; }
    inline psPolygonT<T>& operator =(psPolygonT<T>&& right) { _verts = std::move(right._verts); return *this; }
    template<class U>
    inline psPolygonT<T>& operator =(const psPolygonT<U>& right)
    {
      _verts.SetCapacityDiscard(right._verts.Capacity());
      for(uint16_t i=0; i < _size; ++i) _verts[i]=right._verts[i];
      return *this;
    }

    static inline int PolyIntersectPoint(const VEC* verts, uint16_t num, const VEC& p)
    {
      int i, j, c = 0; //signed ints don't matter here becuase the vertex count is in uint16_t
      for(i = 0, j = num-1; i < num; j = i++) {
        if(((verts[i].y>p.y) != (verts[j].y>p.y)) &&
          (p.x < (verts[j].x-verts[i].x) * (p.y-verts[i].y) / (verts[j].y-verts[i].y) + verts[i].x))
          c = !c;
      }
      return c;
    }

    static inline T PolyArea(const VEC* verts, uint16_t num)
    {
      assert(num>2); //you must be at least a triangle or this is invalid
      T retval=0; //Since the polygon is supposed to be non-intersecting, we can use a line integral to calculate the area

      for(uint16_t i=0, j=num-1; i<num; j=i++) //for loop trick for iterating each edge taken from pnpoly
        retval+=((verts[j].x*verts[i].y)-(verts[i].x*verts[j].y));

      return retval/((T)2);
    }
    static inline psRectT<T> PolyAABB(const VEC* verts, uint16_t num)
    {
      assert(num>1); //you can't be a 1D object or this is invalid
      psRectT<T> retval(verts[0], CVEC_ZERO);
      for(uint16_t i=1; i<num; ++i)
      {
        if(verts[i].x<retval.left) retval.left=verts[i].x;
        else if(verts[i].x>retval.right) retval.right=verts[i].x;
        if(verts[i].y<retval.top) retval.top=verts[i].y;
        else if(verts[i].y>retval.bottom) retval.bottom=verts[i].y;
      }
      return retval;
    }
    static inline VEC PartialCentroid(const VEC* verts, uint16_t num)
    {
      VEC c;

      for(uint16_t i=0, j=num-1; i<num; j=i++) //for loop trick for iterating each edge taken from pnpoly
      {
        c.x+=(verts[j].x + verts[i].x)*((verts[j].x*verts[i].y)-(verts[i].x*verts[j].y));
        c.y+=(verts[j].y + verts[i].y)*((verts[j].x*verts[i].y)-(verts[i].x*verts[j].y));
      }

      return c;
    }
    // Checks if a line intersects the outside border of the polygon. Does NOT check for inclusion! Use the pointintersection
    // test on each point of the line to check for that
    static inline bool PolyLineIntersect(psLineT<T>& line, const VEC* verts, uint16_t num)
    {
      for(uint16_t i=0, j=num-1; i<num; j=i++) //for loop trick for iterating each edge taken from pnpoly
        if(line.IntersectLine(verts[i].x, verts[i].y, verts[j].x, verts[j].y))
          return true;
      return false;
    }

    // Checks if the given series of points consitutes a convex polygon
    static inline bool PolyIsConvex(const VEC* p, uint16_t n)
    {
      uint8_t flag=0;
      double z;

      for(uint32_t i=0, k=0, j=n-1; i<n; j=i++) {
        k=((++k)%n);
        z  = ((p[i].x - p[j].x) * (p[k].y - p[i].y)) - ((p[i].y - p[j].y) * (p[k].x - p[i].x));
        flag|=((z<0)|((z>0)<<1));
      }
      return flag!=3;
    }

  protected:
    bss_util::cArray<VEC, uint32_t> _verts;
  };

  typedef psPolygonT<float> psPolygon; //default typedef
  typedef psPolygonT<int> psPolygoni;
  typedef psPolygonT<double> psPolygond;
  typedef psPolygonT<short> psPolygons;
  typedef psPolygonT<long> psPolygonl;
  typedef psPolygonT<uint32_t> psPolygoniu;
  typedef psPolygonT<uint16_t> psPolygonsu;
  typedef psPolygonT<unsigned long> psPolygonlu;
}

#endif