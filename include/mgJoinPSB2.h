// Copyright ©2016 Black Sphere Studios
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
    virtual void _render();

    struct DrawBuf {
      uint32_t vindex;
      int32 vcount; // if negative, it's a circle
      float32 radius;
      uint32_t color;
      uint32_t outline;
    };

    bss_util::cDynArray<DrawBuf, uint32_t> _drawbuf;
    bss_util::cDynArray<planeshader::psVec, uint32_t> _drawverts;
    bss_util::Matrix<float, 4, 4> _m;
    float _alpha;
  };

  extern void Entity_SetPosition(mgEntity* entity, planeshader::psVec3D pos);
  extern void Entity_SetRotation(mgEntity* entity, float rotation);

  class MG_DLLEXPORT LiquidFunPlaneshaderSystem : public mgSystem<b2PhysicsComponent, b2PhysicsComponent>
  {
  public:
    LiquidFunPlaneshaderSystem(const Box2DSystem::B2INIT& init, int priority = 0);
    LiquidFunPlaneshaderSystem(const LiquidFunSystem::LFINIT& init, int priority = 0);
    ~LiquidFunPlaneshaderSystem();
    virtual void Iterate(mgEntity* entity) override;
    
    inline static planeshader::psVec toVec(b2Vec2 v) { return planeshader::psVec(v.x, v.y); }
  };
}

#endif