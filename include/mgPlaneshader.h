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
  struct MG_DLLEXPORT psRenderableComponent : mgComponent<psRenderableComponent> { ComponentID id; typedef planeshader::psRenderable TYPE; };
  struct MG_DLLEXPORT psLocatableComponent : mgComponent<psLocatableComponent> { ComponentID id; typedef planeshader::psLocatable TYPE; };
  struct MG_DLLEXPORT psSolidComponent : mgComponent<psSolidComponent> { ComponentID id; typedef planeshader::psSolid TYPE; };

  template<class T, class D>
  struct psGenericComponent : D, mgComponent<T, bss_util::CARRAY_MOVE>
  {
    psGenericComponent() {}
    psGenericComponent(psGenericComponent&& mov) : D(std::move(mov)) {}
    psGenericComponent& operator=(psGenericComponent&& mov) { D::operator=(std::move(mov)); return *this; }
    size_t Message(void* msg, ptrdiff_t msgint)
    {
      if(msgint == psRenderableComponent::ID())
        return reinterpret_cast<size_t>(static_cast<psRenderable*>(this));
      if(msgint == psLocatableComponent::ID())
        return reinterpret_cast<size_t>(static_cast<psLocatable*>(this));
      if(msgint == psSolidComponent::ID())
        return reinterpret_cast<size_t>(static_cast<psSolid*>(this));
      return 0; 
    }
    void Construct(mgEntity* e) { PlaneshaderSystem::InitComponents(e); }
  };

  struct MG_DLLEXPORT psImageComponent : psGenericComponent<psImageComponent, planeshader::psImage> { };
  struct MG_DLLEXPORT psTilesetComponent : psGenericComponent<psTilesetComponent, planeshader::psTileset> { };
  struct MG_DLLEXPORT psTextComponent : psGenericComponent<psTextComponent, planeshader::psText> { };
  struct MG_DLLEXPORT psRenderCircleComponent : psGenericComponent<psRenderCircleComponent, planeshader::psRenderCircle> { };
  struct MG_DLLEXPORT psRoundedRectComponent : psGenericComponent<psRoundedRectComponent, planeshader::psRoundedRect> { };

  class MG_DLLEXPORT PlaneshaderSystem : public planeshader::psEngine, public mgSystem<psRenderableComponent, psRenderableComponent>
  {
  public:
    PlaneshaderSystem(const planeshader::PSINIT& init, int priority = 0);
    ~PlaneshaderSystem();
    virtual void Process(mgEntity* entity);
    virtual void Postprocess();
    static void InitComponents(mgEntity* entity);

    template<typename C>
    static typename C::TYPE* ResolveComponent(mgEntity* entity)
    {
      C* c = entity->Get<C>();
      if(c)
        return reinterpret_cast<typename C::TYPE*>(mgComponentStoreBase::MessageComponent(c->id, entity->ComponentListGet(c->id), 0, C::ID()));
      return 0;
    }
  };
}

#endif