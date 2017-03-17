// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __CAMERA_H__PS__
#define __CAMERA_H__PS__

#include "psLocatable.h"
#include "psRect.h"
#include "psDriver.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace planeshader {
  class psSolid;

  class PS_DLLEXPORT psCamera : public psLocatable, public psDriverHold
  {
  public:
    // Constructors
    psCamera(const psCamera& copy);
    explicit psCamera(const psVec3D& position = psVec3D(0, 0, -1.0f), FNUM rotation = 0.0f, const psVec& pivot = VEC_ZERO, const psVec& extent = default_extent);
    ~psCamera();
    // Gets the absolute mouse coordinates with respect to this camera. If RT is null, uses the primary monitor.
    psVec GetMouseAbsolute(const psTex* rt = 0) const;
    // Gets a rect representing the visible area of this camera in absolute coordinates given the provided flags.
    const psRectRotate GetScreenRect(psFlag flags=0) const;
    // Gets or sets the viewport of the camera
    const psRect& GetViewPort() const { return _viewport; }
    inline void SetViewPort(const psRect& vp) { _viewport = vp; }
    void SetViewPortAbs(const psRect& vp, const psTex* rt = 0); // The viewport must be in pixels, but fractional values are technically valid here if rendering at a strange DPI
    void SetPivotAbs(const psVec& pivot, const psTex* rt = 0);
    inline const psVec& GetExtent() const { return _extent; }
    inline void SetExtent(float znear, float zfar) { _extent.x = znear; _extent.y = zfar; }
    inline void SetExtent(const psVec& extent) { _extent = extent; }
    inline psCamera& operator =(const psCamera& copy) { psLocatable::operator =(copy); return *this; }
    inline const psRect& Apply(const psTex* rt) const;
    inline psRectRotateZ Resolve(const psRectRotateZ& rect) const;
    inline psParent Resolve(const psParent& rect) const;
    inline bool Cull(psSolid* solid, const psParent* parent) const;
    inline bool Cull(const psRectRotateZ& rect, const psParent* parent, psFlag flags) const;

    static const psVeci INVALID_LASTRELMOUSE;
    static const psCamera default_camera;
    static psVec default_extent;

    struct CamCache {
      BSS_ALIGN(16) psRect window;
      BSS_ALIGN(16) psRect winfixed;
      sseVec SSEwindow;
      sseVec SSEwindow_center;
      sseVec SSEwindow_hold;
      sseVec SSEfixed;
      sseVec SSEfixed_center;
      sseVec SSEfixed_hold;
      float last;
      float lastfixed;
      psRectRotateZ full;

      CamCache(const CamCache&);
      CamCache();
      inline void SetSSE();
      BSS_FORCEINLINE bool Cull(const psRect& rect, float rectz, float camz, psFlag flags);
    };

    inline const CamCache& GetCache() const { return _cache; }

  protected:
    psRect _viewport; // This is NOT an actual rectangle, it stores left/top/width/height
    psVec _extent;
    mutable CamCache _cache;
  };

  /*class PS_DLLEXPORT psCamera3D : public psCamera
  {
  public:
    // Constructors
    psCamera(const psCamera& copy);
    explicit psCamera(const psVec3D& position=VEC3D_ZERO, FNUM rotation=0.0f, const psVec& pivot=VEC_ZERO);
    ~psCamera();

    inline psCamera& operator =(const psCamera& copy) { psLocatable::operator =(copy); return *this; }
    
  protected:
    psRect _viewport;
  };*/
}
#pragma warning(pop)

#endif