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
  typedef mgComponentInheritBase<planeshader::psRenderable, true> psRenderableComponent;
  typedef mgComponentInheritBase<planeshader::psLocatable, true> psLocatableComponent;
  typedef mgComponentInheritBase<planeshader::psSolid, true> psSolidComponent;

  template<class T>
  struct MG_DLLEXPORT psGenericComponent : T, mgComponent<psGenericComponent<T>, false, bss::ARRAY_SIMPLE,
    mgComponentInheritInit<psGenericComponent<T>, planeshader::psRenderable, true>,
    mgComponentInheritInit<psGenericComponent<T>, planeshader::psLocatable, true>,
    mgComponentInheritInit<psGenericComponent<T>, planeshader::psSolid, true>> {
    explicit psGenericComponent(mgEntity* e = 0) : mgComponent(e) {}
  };

  typedef psGenericComponent<planeshader::psImage> psImageComponent;
  typedef psGenericComponent<planeshader::psTileset> psTilesetComponent;
  typedef psGenericComponent<planeshader::psText> psTextComponent;
  typedef psGenericComponent<planeshader::psRenderCircle> psRenderCircleComponent;
  typedef psGenericComponent<planeshader::psRoundRect> psRoundRectComponent;

  class MG_DLLEXPORT PlaneshaderSystem : public planeshader::psEngine, public mgSystemComplex
  {
    typedef void(*PROCESSFN)(mgEntity&, const planeshader::psRectRotateZ&, void(*)(mgEntity&, const planeshader::psRectRotateZ&, void*));

  public:
    PlaneshaderSystem(const planeshader::PSINIT& init, int priority = 0);
    ~PlaneshaderSystem();
    virtual void Process() override;

  protected:
    virtual void _process(mgEntity& root, const planeshader::psParent& prev);
  };

  struct MG_DLLEXPORT psGUIComponent : mgComponent<psGUIComponent, true>
  {
    explicit psGUIComponent(mgEntity* e = 0) : mgComponent(e) {}
    fgElement element;
  };
}

#endif