// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __JOIN_PS_B2_H__MG__
#define __JOIN_PS_B2_H__MG__

#include "mgLiquidFun.h"
#include "mgPlaneshader.h"

namespace magnesium {
#pragma warning(push)
#pragma warning(disable:4275)
  class MG_DLLEXPORT psDebugDraw : public mgDebugDraw, public planeshader::psDriverHold, public planeshader::psRenderable
  {
#pragma warning(pop)
  public:
    psDebugDraw(psDebugDraw&& mov);
    psDebugDraw(uint32 flags = b2Draw::e_shapeBit + b2Draw::e_jointBit);
    virtual void Clear() override;
    inline virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override { _drawpolygon(vertices, vertexCount, planeshader::psColor(0U), planeshader::psColor(color.r, color.g, color.b, _alpha)); }
    inline virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override { _drawpolygon(vertices, vertexCount, planeshader::psColor(color.r, color.g, color.b, _alpha), planeshader::psColor(0U)); }
    inline virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) override { _drawcircle(center, radius, planeshader::psColor(0U), planeshader::psColor(color.r, color.g, color.b, _alpha)); }
    inline virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) override { _drawcircle(center, radius, planeshader::psColor(color.r, color.g, color.b, _alpha), planeshader::psColor(0U)); }
    virtual void DrawParticles(const b2Vec2 *centers, float32 radius, const b2ParticleColor *colors, int32 count) override;
    virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;
    virtual void DrawTransform(const b2Transform& xf) override;
    inline float GetAlpha() const { return _alpha; }
    inline void SetAlpha(float alpha) { _alpha = alpha; }
    virtual psRenderable* Clone() const { return 0; }

  private:
    void _drawpolygon(const b2Vec2* vertices, int32 vertexCount, planeshader::psColor& color, planeshader::psColor& outline);
    void _drawcircle(const b2Vec2& center, float32 radius, planeshader::psColor& color, planeshader::psColor& outline);
    virtual void _render(const planeshader::psParent& parent) override;

    struct DrawBuf {
      uint32_t vindex;
      int32 vcount; // if negative, it's a circle
      float32 radius;
      uint32_t color;
      uint32_t outline;
    };

    bss::DynArray<DrawBuf, uint32_t> _drawbuf;
    bss::DynArray<planeshader::psVec, uint32_t> _drawverts;
    bss::Matrix<float, 4, 4> _m;
    float _alpha;
  };

  extern void Entity_SetPosition(mgEntity* entity, planeshader::psVec3D pos);
  extern void Entity_SetRotation(mgEntity* entity, float rotation);

  class MG_DLLEXPORT LiquidFunPlaneshaderSystem : public PlaneshaderSystem
  {
  public:
    LiquidFunPlaneshaderSystem(const planeshader::PSINIT& init, int priority = 0, SystemID box2DSystem = mgSystemManager::GetSystemID<LiquidFunSystem>());
    ~LiquidFunPlaneshaderSystem();
    virtual void Process() override;
    virtual void _process(mgEntity& root, const planeshader::psParent& prev) override;
    virtual const char* GetName() const override { return "LiquidFun-PlaneShader"; }

    inline static planeshader::psVec toVec(b2Vec2 v) { return planeshader::psVec(v.x, v.y); }

  protected:
    SystemID _physid;
    ComponentID _physrequired;
    float _ppm;
  };
}

#endif