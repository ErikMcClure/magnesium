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

PlaneshaderSystem::PlaneshaderSystem(const PSINIT& init, int priority) : psEngine(init), mgSystem(priority)
{
  FlushMessages();
}
PlaneshaderSystem::~PlaneshaderSystem() {}
void PlaneshaderSystem::Process(mgEntity* entity)
{
  ResolveComponent<psRenderableComponent>(entity)->Render();
}
void PlaneshaderSystem::Postprocess()
{
  if(!Begin()) return;
  End();
  mgEngine::Instance()->UpdateDelta();
  FlushMessages();
}