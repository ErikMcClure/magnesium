// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in PlaneShader.h

#ifndef __VECTOR_H__PS__
#define __VECTOR_H__PS__

#include "psSolid.h"
#include "psColored.h"
#include "bss-util\cDynArray.h"

namespace planeshader {
  // Used by all curve objects to render a curve as a deconstructed quadratic curve
  class PS_DLLEXPORT psQuadraticHull : public psRenderable, public psDriverHold
  {
    struct QuadVertex
    {
      float x, y, z, t;
      float p0[4];
      float p1[4];
      float p2[4];
      uint32_t color;
    };

  public:
    psQuadraticHull();
    ~psQuadraticHull();
    void Clear();
    void BSS_FASTCALL AppendQuadraticCurve(psVec p0, psVec p1, psVec p2, float thickness, uint32_t color, char cap);

    static const int CURVEBUFSIZE = 512 * 3;
    static inline psVec FromQuad(QuadVertex& v) { return psVec(v.x, v.y); }
    static void SetVert(float(&v)[4], psVec& x, float thickness);
    virtual psQuadraticHull* Clone() const override { return 0; }

  protected:
    virtual void BSS_FASTCALL _render() override;
    bss_util::cDynArray<QuadVertex, uint32_t> _verts;
  };

  class PS_DLLEXPORT psQuadraticCurve : public psQuadraticHull, public psColored
  {
  public:
    psQuadraticCurve(psVec p0, psVec p1, psVec p2, float thickness = 1.0f, uint32_t color = 0xFFFFFFFF);
    psQuadraticCurve(psVec(&p)[3], float thickness = 1.0f, uint32_t color = 0xFFFFFFFF);
    ~psQuadraticCurve();
    void Set(psVec p0, psVec p1, psVec p2);
    void Set(psVec (&p)[3]);
    inline void SetThickness(float thickness) { _thickness = thickness; }
    inline float GetThickness() const { return _thickness; }
    inline const psVec(&Get() const)[3] { return _p; }
    virtual psQuadraticCurve* Clone() const override { return 0; }

  protected:
    psVec _p[3];
    float _thickness;
  };

  class PS_DLLEXPORT psCubicCurve : public psQuadraticHull, public psColored
  {
  public:
    psCubicCurve(psVec p0, psVec p1, psVec p2, psVec p3, float thickness = 1.0f, uint32_t color = 0xFFFFFFFF, float maxerr = 1.0f);
    psCubicCurve(psVec(&p)[4], float thickness = 1.0f, uint32_t color = 0xFFFFFFFF);
    ~psCubicCurve();
    void Set(psVec p0, psVec p1, psVec p2, psVec p3);
    void Set(psVec(&p)[4]);
    inline void SetThickness(float thickness) { _thickness = thickness; }
    inline float GetThickness() const { return _thickness; }
    inline const psVec(&Get() const)[4] { return _p; }

  protected:
    void BSS_FASTCALL _addquad(const float(&P0)[2], const float(&P1)[2], const float(&P2)[2]);
    psVec _p[4];
    float _thickness;
    float _maxerr;
  };

  class PS_DLLEXPORT psRoundedRect : public psSolid, public psColored, public psDriverHold
  {
    struct RRVertex
    {
      float x, y, z, t;
      psRect dimpivot;
      psRect corners;
      float outline;
      uint32_t color;
      uint32_t outlinecolor;
    };

  public:
    psRoundedRect(const psRoundedRect& copy);
    psRoundedRect(psRoundedRect&& mov);
    explicit psRoundedRect(const psRectRotateZ& rect = psRectRotateZ(0, 0, 0, 0, 0, VEC_ZERO, 0), psFlag flags = 0, int zorder = 0, psStateblock* stateblock = 0, psShader* shader = 0, psPass* pass = 0, psInheritable* parent = 0, const psVec& scale = VEC_ONE);
    virtual ~psRoundedRect();
    const psRect& GetCorners() const { return _corners; }
    void SetCorners(const psRect& corners) { _corners = corners; }
    float GetOutline() const { return _edge; }
    void SetOutline(float outline) { _edge = outline; }
    psColor32 GetOutlineColor() const { return _outline; }
    void SetOutlineColor(psColor32 color) { _outline = color; }

    static const int RRBUFSIZE = 64;
    static void DrawRoundedRect(psShader* shader, psStateblock* stateblock, const psRectRotateZ& rect, const psRect& corners, psFlag flags, psColor32 color32, psColor32 outline32, float edge);

  protected:
    virtual void BSS_FASTCALL _render() override;

    psRect _corners;
    psColor32 _outline;
    float _edge;
  };

  class PS_DLLEXPORT psRenderCircle : public psSolid, public psColored, public psDriverHold
  {
    struct CircleVertex
    {
      float x, y, z, t;
      psRect dimpivot;
      psRect arcs;
      float outline;
      uint32_t color;
      uint32_t outlinecolor;
    };

  public:
    psRenderCircle(const psRenderCircle& copy);
    psRenderCircle(psRenderCircle&& mov);
    explicit psRenderCircle(float radius = 0, const psVec3D& position = VEC3D_ZERO, psFlag flags = 0, int zorder = 0, psStateblock* stateblock = 0, psShader* shader = 0, psPass* pass = 0, psInheritable* parent = 0, const psVec& scale = VEC_ONE);
    virtual ~psRenderCircle();
    const psRect& GetArcs() const { return _arcs; }
    void SetArcs(const psRect& arcs) { _arcs = arcs; }
    float GetOutline() const { return _edge; }
    void SetOutline(float outline) { _edge = outline; }
    psColor32 GetOutlineColor() const { return _outline; }
    void SetOutlineColor(psColor32 color) { _outline = color; }

    static const int CIRCLEBUFSIZE = 64;
    static void DrawCircle(psShader* shader, psStateblock* stateblock, const psRectRotateZ& rect, const psRect& arcs, psFlag flags, psColor32 color32, psColor32 outline32, float edge);

  protected:
    virtual void BSS_FASTCALL _render() override;

    psRect _arcs;
    psColor32 _outline;
    float _edge;
  };
}

#endif