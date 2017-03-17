// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgPlaneshader.h"
#include "mgEngine.h"
#include "planeshader/psPass.h"
#include "planeshader/psCamera.h"

using namespace magnesium;
using namespace planeshader;

#ifdef BSS_COMPILER_MSC
#if defined(BSS_DEBUG) && defined(BSS_CPU_x86_64)
#pragma comment(lib, "../lib/PlaneShader_d.lib")
#elif defined(BSS_CPU_x86_64)
#pragma comment(lib, "../lib/PlaneShader.lib")
#elif defined(BSS_DEBUG)
#pragma comment(lib, "../lib/PlaneShader32_d.lib")
#else
#pragma comment(lib, "../lib/PlaneShader32.lib")
#endif
#endif

PlaneshaderSystem::PlaneshaderSystem(const PSINIT& init, int priority) : psEngine(init, &mgEngine::Instance()->GetLog().GetStream()), mgSystemComplex(psRenderableComponent::GraphID(), priority)
{
  FlushMessages();
}
PlaneshaderSystem::~PlaneshaderSystem() {}
void PlaneshaderSystem::Process()
{
  if(!Begin()) return;

  // Iterate over entire scene graph, rendering everything
  _process(mgEntity::SceneGraph(), psParent::Zero);

  End();
  mgEngine::Instance()->UpdateDelta();
  FlushMessages();
}

void PlaneshaderSystem::_process(mgEntity& root, const psParent& prev)
{
  int i = 0;
  size_t l = root.NumChildren();
  mgEntity* const* p = root.Children();
  psParent parent = prev;

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

  if((root.childhint&_required) == _required && l > 0)
  {
    while(i < l && p[i]->Order() < 0)
      _process(*p[i++], parent);
  }
  if((root.graphcomponents&_required) == _required)
  {
    psRenderable* renderable = root.Get<psRenderableComponent>()->Get();
    auto gui = root.Get<psGUIComponent>();
    if(gui)
    {
      pass = renderable->GetPass();
      if(!pass) pass = psPass::CurPass;
      psParent resolved = pass->GetCamera()->Resolve(parent);
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
  if((root.childhint&_required) == _required && i < l)
  {
    while(i < l)
      _process(*p[i++], parent);
  }
}