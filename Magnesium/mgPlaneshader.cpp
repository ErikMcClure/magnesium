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

PlaneshaderSystem::PlaneshaderSystem(const PSINIT& init, int priority) : psEngine(init), mgSystemComplex(psRenderableComponent::GraphID(), priority)
{
  FlushMessages();
}
PlaneshaderSystem::~PlaneshaderSystem() {}
void PlaneshaderSystem::Process()
{
  if(!Begin()) return;

  // Iterate over entire scene graph, rendering everything
  psRectRotateZ base = { };
  _process(mgEntity::SceneGraph(), base);

  End();
  mgEngine::Instance()->UpdateDelta();
  FlushMessages();
}

void PlaneshaderSystem::_process(mgEntity& root, const psRectRotateZ& prev)
{
  int i = 0;
  size_t l = root.NumChildren();
  mgEntity* const* p = root.Children();
  psMatrix m;
  psRectRotateZ rect = prev;

  auto locatable = root.Get<psLocatableComponent>();
  auto solid = root.Get<psSolidComponent>();
  bool cull = false;
  psPass* pass = 0;

  if(locatable)
  {
    psLocatable* loc = locatable->Get();
    rect = prev.RelativeTo(loc->GetPosition(), loc->GetRotation(), loc->GetPivot());
    if(l > 0)
      loc->GetTransform(m);

    if(solid)
    {
      psSolid* s = solid->Get();
      pass = s->GetPass();
      if(!pass) pass = psPass::CurPass;
      if(pass != 0)
        cull = pass->GetCamera()->Cull(rect, s->GetFlags());
    }
  }

  if((root.childhint&_required) == _required && l > 0)
  {
    if(locatable)
      _driver->PushTransform(m);
    while(i < l && p[i]->Order() < 0)
      _process(*p[i++], rect);
    if(locatable)
      _driver->PopTransform();
  }
  if((root.graphcomponents&_required) == _required)
  {
    psRenderable* renderable = root.Get<psRenderableComponent>()->Get();
    auto gui = root.Get<psGUIComponent>();
    if(gui)
    {
      if(solid)
      {
        psRectRotateZ absrect = pass->GetCamera()->Resolve(rect);
        fgTransform t = { {absrect.left, 0, absrect.top, 0, absrect.right, 0, absrect.bottom, 0}, absrect.rotation, {absrect.pivot.x, 0, absrect.pivot.y, 0} };
        gui->element.SetTransform(t);
      }
      else
      {
        pass = renderable->GetPass();
        if(!pass) pass = psPass::CurPass;
        psRectRotateZ absrect = pass->GetCamera()->Resolve(rect);
        fgTransform t = { { absrect.left, 0, absrect.top, 0, absrect.left, 0, absrect.top, 0 }, absrect.rotation, { absrect.pivot.x, 0, absrect.pivot.y, 0 } };
        gui->element.SetTransform(t);
      }
    }
    if(!cull)
      renderable->Render();
  }
  if((root.childhint&_required) == _required && i < l)
  {
    if(locatable)
      _driver->PushTransform(m);
    while(i < l)
      _process(*p[i++], rect);
    if(locatable)
      _driver->PopTransform();
  }
}