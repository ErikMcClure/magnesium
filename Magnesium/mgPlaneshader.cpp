// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgPlaneshader.h"
#include "mgEngine.h"
#include "planeshader/psLayer.h"
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
  auto layer = root.Get<psLayerComponent>();
  
  if(locatable)
  {
    psLocatable* loc = locatable->Get();
    parent = prev.Push(loc->GetPosition(), loc->GetRotation(), loc->GetPivot());
  }

  if(layer)
    layer->Push(parent);

  if((root.childhint&_required) == _required && cur != 0)
  {
    while(cur != 0 && cur->Order() < 0)
    {
      _process(*cur, parent);
      cur = cur->Next();
    }
  }
  if(!layer && (root.graphcomponents&_required) == _required) // Do not render layers, as they will be rendered when we call Pop()
  {
    psRenderable* renderable = root.Get<psRenderableComponent>()->Get();
    renderable->Render(prev);
  }
  if((root.childhint&_required) == _required && cur != 0)
  {
    while(cur)
    {
      _process(*cur, parent);
      cur = cur->Next();
    }
  }

  if(layer)
    layer->Pop();
}