// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgPlaneshader.h"
#include "mgEngine.h"

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
  _process(mgEntity::SceneGraph());

  End();
  mgEngine::Instance()->UpdateDelta();
  FlushMessages();
}
void PlaneshaderSystem::Iterate(mgEntity& entity)
{

}

void PlaneshaderSystem::_process(mgEntity& root)
{
  int i = 0;
  size_t l = root.NumChildren();
  mgEntity* const* p = root.Children();

  auto r = root.Get<psRenderableComponent>();
  ResolveComponent<psRenderableComponent>(root)->Render();

  if(root.childhint&_required)
  {
    while(i < l && p[i]->Order() < 0)
      _process(*p[i++]);
  }
  if(root.graphcomponents&_required)
    Iterate(root);
  if(root.childhint&_required)
  {
    while(i < l)
      _process(*p[i++]);
  }
}
