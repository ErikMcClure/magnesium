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
  struct MG_DLLEXPORT psRenderableComponent : mgComponent<psRenderableComponent> { planeshader::psRenderable* p; };
  struct MG_DLLEXPORT psLocatableComponent : mgComponent<psLocatableComponent> { planeshader::psLocatable* p; };
  struct MG_DLLEXPORT psSolidComponent : mgComponent<psSolidComponent> { planeshader::psSolid* p; };
  struct MG_DLLEXPORT psImageComponent : planeshader::psImage, mgComponent<psImageComponent, bss_util::CARRAY_MOVE> {
    psImageComponent() {}
    psImageComponent(psImageComponent&& mov) : planeshader::psImage(std::move(mov)) {}
    psImageComponent& operator=(psImageComponent&& mov) { planeshader::psImage::operator=(std::move(mov)); return *this; }
  };

  struct MG_DLLEXPORT psTilesetComponent : planeshader::psTileset, mgComponent<psTilesetComponent, bss_util::CARRAY_MOVE> {
    psTilesetComponent() {}
    psTilesetComponent(psTilesetComponent&& mov) : planeshader::psTileset(std::move(mov)) {}
    psTilesetComponent& operator=(psTilesetComponent&& mov) { planeshader::psTileset::operator=(std::move(mov)); return *this; }
  };

  struct MG_DLLEXPORT psTextComponent : planeshader::psText, mgComponent<psTextComponent, bss_util::CARRAY_MOVE> {
    psTextComponent() {}
    psTextComponent(psTextComponent&& mov) : planeshader::psText(std::move(mov)) {}
    psTextComponent& operator=(psTextComponent&& mov) { planeshader::psText::operator=(std::move(mov)); return *this; }
  };

  struct MG_DLLEXPORT psRenderCircleComponent : planeshader::psRenderCircle, mgComponent<psRenderCircleComponent, bss_util::CARRAY_MOVE> {
    psRenderCircleComponent() {}
    psRenderCircleComponent(psRenderCircleComponent&& mov) : planeshader::psRenderCircle(std::move(mov)) {}
    psRenderCircleComponent& operator=(psRenderCircleComponent&& mov) { planeshader::psRenderCircle::operator=(std::move(mov)); return *this; }
  };

  struct MG_DLLEXPORT psRoundedRectComponent : planeshader::psRoundedRect, mgComponent<psRoundedRectComponent, bss_util::CARRAY_MOVE> {
    psRoundedRectComponent() {}
    psRoundedRectComponent(psRoundedRectComponent&& mov) : planeshader::psRoundedRect(std::move(mov)) {}
    psRoundedRectComponent& operator=(psRoundedRectComponent&& mov) { planeshader::psRoundedRect::operator=(std::move(mov)); return *this; }
  };

  class MG_DLLEXPORT PlaneshaderSystem : public planeshader::psEngine, public mgSystem<psRenderableComponent, psRenderableComponent>
  {
  public:
    PlaneshaderSystem(const planeshader::PSINIT& init);
    ~PlaneshaderSystem();
    virtual void Process(mgEntity* entity);
    virtual void Postprocess();
    static void InitComponents(mgEntity* entity);
  };
}

#endif