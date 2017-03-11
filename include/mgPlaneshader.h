// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __PLANESHADER_H__MG__
#define __PLANESHADER_H__MG__

#include "mgSystem.h"
#include "planeshader\psImage.h"
#include "planeshader\psTileset.h"
#include "planeshader\psText.h"
#include "planeshader\psVector.h"
#include "planeshader\psRenderGeometry.h"
#include "planeshader\psEngine.h"

namespace magnesium {
  struct MG_DLLEXPORT psRenderableComponent : mgComponent<psRenderableComponent> { explicit psRenderableComponent(mgEntity* e = 0) : mgComponent(e) {} ComponentID id; typedef planeshader::psRenderable TYPE; };
  struct MG_DLLEXPORT psLocatableComponent : mgComponent<psLocatableComponent> { explicit psLocatableComponent(mgEntity* e = 0) : mgComponent(e) {} ComponentID id; typedef planeshader::psLocatable TYPE; };
  struct MG_DLLEXPORT psSolidComponent : mgComponent<psSolidComponent> { explicit psSolidComponent(mgEntity* e = 0) : mgComponent(e) {} ComponentID id; typedef planeshader::psSolid TYPE; };

  template<class T, class D>
  struct psGenericComponent : mgComponent<T, bss_util::CARRAY_MOVE>, D
  {
    psGenericComponent(mgEntity* e) : mgComponent<T, bss_util::CARRAY_MOVE>(e)
    {
      PlaneshaderSystem::InitComponent<psRenderableComponent>(e, ID());
      PlaneshaderSystem::InitComponent<psLocatableComponent>(e, ID());
      PlaneshaderSystem::InitComponent<psSolidComponent>(e, ID());
    }
    psGenericComponent(psGenericComponent&& mov) : mgComponent<T, bss_util::CARRAY_MOVE>(std::move(mov)), D(std::move(mov)) {}
    psGenericComponent& operator=(psGenericComponent&& mov) { mgComponent::operator=(std::move(mov)); D::operator=(std::move(mov)); return *this; }
    size_t Message(void* msg, ptrdiff_t msgint)
    {
      if(msgint == psRenderableComponent::ID())
        return reinterpret_cast<size_t>(static_cast<planeshader::psRenderable*>(this));
      if(msgint == psLocatableComponent::ID())
        return reinterpret_cast<size_t>(static_cast<planeshader::psLocatable*>(this));
      if(msgint == psSolidComponent::ID())
        return reinterpret_cast<size_t>(static_cast<planeshader::psSolid*>(this));
      return 0; 
    }
  };

  struct MG_DLLEXPORT psImageComponent : psGenericComponent<psImageComponent, planeshader::psImage> { explicit psImageComponent(mgEntity* e=0) : psGenericComponent(e) {} };
  struct MG_DLLEXPORT psTilesetComponent : psGenericComponent<psTilesetComponent, planeshader::psTileset> { explicit psTilesetComponent(mgEntity* e=0) : psGenericComponent(e) {} };
  struct MG_DLLEXPORT psTextComponent : psGenericComponent<psTextComponent, planeshader::psText> { explicit psTextComponent(mgEntity* e=0) : psGenericComponent(e) {} };
  struct MG_DLLEXPORT psRenderCircleComponent : psGenericComponent<psRenderCircleComponent, planeshader::psRenderCircle> { explicit psRenderCircleComponent(mgEntity* e=0) : psGenericComponent(e) {} };
  struct MG_DLLEXPORT psRoundRectComponent : psGenericComponent<psRoundRectComponent, planeshader::psRoundRect> { explicit psRoundRectComponent(mgEntity* e=0) : psGenericComponent(e) {} };

  class MG_DLLEXPORT PlaneshaderSystem : public planeshader::psEngine, public mgSystem<void, psRenderableComponent>
  {
  public:
    PlaneshaderSystem(const planeshader::PSINIT& init, int priority = 0);
    ~PlaneshaderSystem();
    virtual void Process() override;

    template<typename T>
    inline static void InitComponent(mgEntity* e, ComponentID p)
    {
      if(p != (ComponentID)~0)
      {
        T* r = e->Get<T>();
        if(!r) r = e->Add<T>();
        r->id = p;
      }
    }

    template<typename C>
    static typename C::TYPE* ResolveComponent(mgEntity* entity)
    {
      C* c = entity->Get<C>();
      if(c)
        return reinterpret_cast<typename C::TYPE*>(mgComponentStoreBase::MessageComponent(c->id, entity->ComponentListGet(c->id), 0, C::ID()));
      return 0;
    }
  };

  struct MG_DLLEXPORT RawRenderableComponent : mgComponent<RawRenderableComponent, bss_util::CARRAY_MOVE>, planeshader::psRenderable
  {
    explicit RawRenderableComponent(mgEntity* e = 0) : mgComponent(e) { PlaneshaderSystem::InitComponent<psRenderableComponent>(e, ID()); }
    RawRenderableComponent(RawRenderableComponent&& mov) : mgComponent(std::move(mov)), planeshader::psRenderable(std::move(mov)) {}
    RawRenderableComponent& operator=(RawRenderableComponent&& mov) { mgComponent::operator=(std::move(mov)); planeshader::psRenderable::operator=(std::move(mov)); return *this; }
    size_t Message(void* msg, ptrdiff_t msgint)
    {
      if(msgint == psRenderableComponent::ID())
        return reinterpret_cast<size_t>(static_cast<planeshader::psRenderable*>(this));
      return 0;
    }
    virtual void _render() { if(render) render(entity); }
    void(*render)(mgEntity* e);
  };
}

#endif