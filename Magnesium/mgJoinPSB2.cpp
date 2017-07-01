// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgJoinPSB2.h"
#include "Box2D/Box2D.h"
#include "planeshader/psVector.h"

using namespace magnesium;
using namespace planeshader;
using namespace bss;

psDebugDraw::psDebugDraw(psDebugDraw&& mov) : _alpha(mov._alpha), psRenderable(std::move(mov)) { SetFlags(static_cast<b2Draw&>(mov).GetFlags()); }
psDebugDraw::psDebugDraw(uint32 flags) : _alpha(1.0) { SetFlags(flags); }
void psDebugDraw::Clear()
{
  _drawbuf.Clear();
  _drawverts.Clear();
}
void psDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
  uint32_t color32;
  psColor(color.r, color.g, color.b, _alpha).WriteFormat(_driver->GetBackBuffer()->GetFormat(), &color32);

  _drawbuf.Add(DrawBuf { _drawverts.Length(), -2, 0, color32, 0 });
  _drawverts.Add(LiquidFunPlaneshaderSystem::toVec(p1));
  _drawverts.Add(LiquidFunPlaneshaderSystem::toVec(p2));
}
void psDebugDraw::DrawTransform(const b2Transform& xf) {}
void psDebugDraw::DrawParticles(const b2Vec2 *centers, float32 radius, const b2ParticleColor *colors, int32 count)
{

}

void psDebugDraw::_drawpolygon(const b2Vec2* vertices, int32 vertexCount, psColor& color, psColor& outline)
{
  if(!vertexCount) return;

  uint32_t color32;
  uint32_t outline32;
  color.WriteFormat(_driver->GetBackBuffer()->GetFormat(), &color32);
  outline.WriteFormat(_driver->GetBackBuffer()->GetFormat(), &outline32);

  _drawbuf.Add(DrawBuf { _drawverts.Length(), vertexCount, 0, color32, outline32 });
  for(int32 i = 0; i < vertexCount; ++i)
    _drawverts.Add(LiquidFunPlaneshaderSystem::toVec(vertices[i]));
}
void psDebugDraw::_drawcircle(const b2Vec2& center, float32 radius, psColor& color, psColor& outline)
{
  if(radius <= 0.0f) return;

  uint32_t color32;
  uint32_t outline32;
  color.WriteFormat(_driver->GetBackBuffer()->GetFormat(), &color32);
  outline.WriteFormat(_driver->GetBackBuffer()->GetFormat(), &outline32);

  _drawbuf.Add(DrawBuf { _drawverts.Length(), 0, radius, color32, outline32 });
  _drawverts.Add(LiquidFunPlaneshaderSystem::toVec(center));
}
void psDebugDraw::_render(const psParent& parent)
{
  Matrix<float, 4, 4> m;
  parent.GetTransform(m.v);
  Matrix<float, 4, 4>::AffineScaling(Box2DSystem::Instance()->F_PPM, Box2DSystem::Instance()->F_PPM, 1.0f, m);
  _driver->PushTransform(m.v);

  for(DrawBuf& buf : _drawbuf)
  {
    if(buf.vcount > 0) // polygon
      _driver->DrawPolygon(_driver->library.POLYGON, 0, _drawverts.begin() + buf.vindex, buf.vcount, VEC3D_ZERO, buf.color, PSFLAG_DONOTBATCH);
    if(!buf.vcount) // circle
    {
      psVec p = _drawverts[buf.vindex];
      psRenderCircle::DrawCircle(0, 0, psRectRotateZ(p.x - buf.radius - 1, p.y - buf.radius - 1, p.x + buf.radius + 1, p.y + buf.radius + 1, 0), psRect(0, PI_DOUBLEf, 0, PI_DOUBLEf), 0, buf.color, buf.outline, 1.0f, _m.v);
    }
    if(buf.vcount == -2) // line
    {
      psBatchObj* batch = _driver->DrawLinesStart(_driver->library.LINE, 0, 0);
      _driver->DrawLines(batch, psLine(_drawverts[buf.vindex], _drawverts[buf.vindex + 1]), 0, 0, buf.color);
    }
  }

  _driver->PopTransform();
}

void magnesium::Entity_SetPosition(mgEntity* entity, planeshader::psVec3D pos)
{
  auto b = entity->Get<b2PhysicsComponent>();
  auto r = entity->Get<psLocatableComponent>();
  if(r) r->Get()->SetPosition(pos);
  if(b) b->SetPosition(b2Vec2(pos.x, pos.y));
}
void magnesium::Entity_SetRotation(mgEntity* entity, float rotation)
{
  auto b = entity->Get<b2PhysicsComponent>();
  auto r = entity->Get<psLocatableComponent>();
  if(r) r->Get()->SetRotation(rotation);
  if(b) b->SetRotation(rotation);
}

LiquidFunPlaneshaderSystem::LiquidFunPlaneshaderSystem(const planeshader::PSINIT& init, int priority, SystemID id) :
  PlaneshaderSystem(init, priority), _physid(id), _physrequired(b2PhysicsComponent::ID()|psLocatableComponent::ID()) {}
LiquidFunPlaneshaderSystem::~LiquidFunPlaneshaderSystem() {}
void LiquidFunPlaneshaderSystem::Process()
{
  float _ppm = _manager->MessageSystem(_physid, 1, 0).f;
  PlaneshaderSystem::Process();
}
void LiquidFunPlaneshaderSystem::_process(mgEntity& root, const psParent& prev)
{
  if((root.graphcomponents&_physrequired) == _physrequired)
  {
    auto b = root.Get<b2PhysicsComponent>();
    auto r = root.Get<psLocatableComponent>();
    assert(r);
    assert(b);
    psLocatable* loc = r->Get();
    loc->SetPivot(toVec(b->GetBody()->GetLocalCenter()) *= _ppm);
    loc->SetPosition(toVec(b->GetPosition()));
    loc->SetRotation(b->GetRotation());
  }

  PlaneshaderSystem::_process(root, prev);
}