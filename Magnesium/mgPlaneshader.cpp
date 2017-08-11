// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgPlaneshader.h"
#include "mgEngine.h"
#include "planeshader/psPass.h"
#include "planeshader/psCamera.h"

using namespace magnesium;
using namespace planeshader;

static_assert(!std::is_polymorphic<psRenderableComponent>::value, "Components shouldn't have a vtable");
static_assert(!std::is_polymorphic<psLocatableComponent>::value, "Components shouldn't have a vtable");
static_assert(!std::is_polymorphic<psSolidComponent>::value, "Components shouldn't have a vtable");

PlaneshaderSystem::PlaneshaderSystem(const PSINIT& init, int priority) : psEngine(init, &mgEngine::Instance()->GetLog().GetStream()), mgSystemComplex(psRenderableComponent::GraphID(), priority)
{
  FlushMessages();
}
PlaneshaderSystem::~PlaneshaderSystem() {}
void PlaneshaderSystem::Process()
{
  if(_f) _f();
  if(!Begin()) return;

  // Iterate over entire scene graph, rendering everything
  _process(mgEngine::Instance()->SceneGraph(), psTransform2D::Zero);

  End();
  mgEngine::Instance()->UpdateDelta();
  FlushMessages();
  fgRoot_Update(fgSingleton(), mgEngine::Instance()->GetDelta()*0.001);
}

void PlaneshaderSystem::_process(mgEntity& root, const psTransform2D& prev)
{
  mgEntity* cur = root.Children();
  psTransform2D parent = prev;

  auto locatable = root.Get<psLocatableComponent>();
  auto solid = root.Get<psSolidComponent>();
  bool cull = false;
  psPass* pass = 0;
  
  if(locatable)
  {
    psLocatable* loc = locatable->Get();
    parent = prev.Push(loc->GetPosition(), loc->GetRotation(), loc->GetPivot());

    if(solid)
    {
      psSolid* s = solid->Get();
      pass = s->GetPass();
      if(!pass) pass = psPass::CurPass;
      if(pass != 0)
        cull = pass->GetCamera()->Cull(s, &prev);
    }
  }

  if((root.childhint&_required) == _required && cur != 0)
  {
    while(cur != 0 && cur->Order() < 0)
    {
      _process(*cur, parent);
      cur = cur->Next();
    }
  }
  if((root.graphcomponents&_required) == _required)
  {
    psRenderable* renderable = root.Get<psRenderableComponent>()->Get();
    auto gui = root.Get<psGUIComponent>();
    if(gui)
    {
      pass = renderable->GetPass();
      if(!pass) pass = psPass::CurPass;
      psTransform2D resolved = pass->GetCamera()->Resolve(parent);
      if(solid)
      {
        psVec pos = resolved.position.xy - resolved.pivot;
        psVec dim = solid->Get()->GetDim();
        fgTransform t = { {pos.x, 0, pos.y, 0, pos.x + dim.x, 0, pos.y + dim.y, 0}, resolved.rotation, { resolved.pivot.x, 0, resolved.pivot.y, 0} };
        gui->element.SetTransform(t);
      }
      else
      {
        fgTransform t = { { resolved.position.x, 0, resolved.position.y, 0, resolved.position.x, 0, resolved.position.y, 0 }, resolved.rotation,{ resolved.pivot.x, 0, resolved.pivot.y, 0 } };
        gui->element.SetTransform(t);
      }
    }
    if(!cull)
      renderable->Render(&prev);
  }
  if((root.childhint&_required) == _required && cur != 0)
  {
    while(cur)
    {
      _process(*cur, parent);
      cur = cur->Next();
    }
  }
}