// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __PLANESHADER_H__MG__
#define __PLANESHADER_H__MG__

#include "mgSystem.h"
#include "planeshader\psImage.h"
#include "planeshader\psEngine.h"

namespace magnesium {
  struct MG_DLLEXPORT psRenderableComponent : mgComponent<psRenderableComponent> { planeshader::psRenderable* p; };
  struct MG_DLLEXPORT psLocatableComponent : mgComponent<psLocatableComponent> { planeshader::psLocatable* p; };
  struct MG_DLLEXPORT psSolidComponent : mgComponent<psSolidComponent> { planeshader::psSolid* p; };
  struct MG_DLLEXPORT psImageComponent : planeshader::psImage, mgComponent<psImageComponent, bss_util::CARRAY_MOVE> {
    psImageComponent() {}
    psImageComponent(psImageComponent&& mov) : planeshader::psImage(std::move(mov)) {}
    psImageComponent& operator=(psImageComponent&& mov) { planeshader::psImage::operator=(std::move(mov)); return *this; }
  };

  class MG_DLLEXPORT PlaneshaderSystem : public planeshader::psEngine, public mgSystem<psRenderableComponent, psRenderableComponent>
  {
  public:
    PlaneshaderSystem(const planeshader::PSINIT& init);
    ~PlaneshaderSystem();
    virtual void Process(mgEntity* entity);
    virtual void Postprocess();
  };
}

#endif