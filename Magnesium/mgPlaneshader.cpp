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
template<typename Arg, typename... Args>
struct mgfindptr { static inline ComponentID f(Arg* arg, Args*... args) { return (arg != nullptr) ? Arg::ID() : mgfindptr<Args...>::f(args...); } };
template<typename Arg>
struct mgfindptr<Arg> { static inline ComponentID f(Arg* arg) { return (arg != nullptr) ? Arg::ID() : (ComponentID)~0; }; };

template<typename T, typename... Args>
inline void mgPSInitComponent(mgEntity* e, Args*... args)
{
  ComponentID p = mgfindptr<Args...>::f(args...);
  if(p != (ComponentID)~0)
  {
    T* r = e->Get<T>();
    if(!r) r = e->Add<T>();
    r->id = p;
  }
}

void PlaneshaderSystem::InitComponents(mgEntity* e)
{
  psImageComponent* img = e->Get<psImageComponent>();
  psTilesetComponent* tile = e->Get<psTilesetComponent>();
  psTextComponent* text = e->Get<psTextComponent>();
  psRenderCircleComponent* circ = e->Get<psRenderCircleComponent>();
  psRoundedRectComponent* rect = e->Get<psRoundedRectComponent>();

  mgPSInitComponent<psRenderableComponent>(e, img, tile, text, circ, rect);
  mgPSInitComponent<psLocatableComponent>(e, img, tile, text, circ, rect);
  mgPSInitComponent<psSolidComponent>(e, img, tile, text, circ, rect);
}
