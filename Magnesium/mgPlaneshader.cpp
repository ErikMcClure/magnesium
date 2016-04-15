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

PlaneshaderSystem::PlaneshaderSystem(const PSINIT& init) : psEngine(init)
{
  FlushMessages();
}
PlaneshaderSystem::~PlaneshaderSystem() {}
void PlaneshaderSystem::Process(mgEntity* entity)
{
  entity->Get<psRenderableComponent>()->p->Render();
}
void PlaneshaderSystem::Postprocess()
{
  if(!Begin()) return;
  End();
  mgEngine::Instance()->UpdateDelta();
  FlushMessages();
}
template<typename Cast, typename Arg, typename... Args>
struct mgfindptr { static inline Cast* f(Arg* arg, Args*... args) { return (arg != nullptr) ? static_cast<Cast*>(arg) : mgfindptr<Cast, Args...>::f(args...); } };
template<typename Cast, typename Arg>
struct mgfindptr<Cast, Arg> { static inline Cast* f(Arg* arg) { return static_cast<Cast*>(arg); }; };

template<typename T, typename Cast, typename... Args>
inline void mgPSInitComponent(mgEntity* e, Args*... args)
{
  Cast* p = mgfindptr<Cast, Args...>::f(args...);
  if(p != 0)
  {
    T* r = e->Get<T>();
    if(!r) r = e->Add<T>();
    r->p = p;
  }
}

void PlaneshaderSystem::InitComponents(mgEntity* e)
{
  psImageComponent* img = e->Get<psImageComponent>();
  psTilesetComponent* tile = e->Get<psTilesetComponent>();
  psTextComponent* text = e->Get<psTextComponent>();
  psRenderCircleComponent* circ = e->Get<psRenderCircleComponent>();
  psRoundedRectComponent* rect = e->Get<psRoundedRectComponent>();

  mgPSInitComponent<psRenderableComponent, psRenderable>(e, img, tile, text, circ, rect);
  mgPSInitComponent<psLocatableComponent, psLocatable>(e, img, tile, text, circ, rect);
  mgPSInitComponent<psSolidComponent, psSolid>(e, img, tile, text, circ, rect);
}
