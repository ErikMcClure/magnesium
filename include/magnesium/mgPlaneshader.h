// Copyright ©2017 Black Sphere Studios
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
  typedef mgComponentInherit<planeshader::psRenderable, true> psRenderableComponent;
  typedef mgComponentInherit<planeshader::psLocatable, true> psLocatableComponent;
  typedef mgComponentInherit<planeshader::psSolid, true> psSolidComponent;

  template<class T> using psRenderableBind = mgComponentInheritBind<planeshader::psRenderable, T, true>;
  template<class T> using psLocatableBind = mgComponentInheritBind<planeshader::psLocatable, T, true>;
  template<class T> using psSolidBind = mgComponentInheritBind<planeshader::psSolid, T, true>;

  template<class T>
  struct MG_DLLEXPORT psGenericComponent : T, mgComponent<psGenericComponent<T>, false, bss::ARRAY_SAFE,
    psRenderableBind<psGenericComponent<T>>,
    psLocatableBind<psGenericComponent<T>>,
    psSolidBind<psGenericComponent<T>>>
  {
    explicit psGenericComponent(mgEntity* e = 0) : mgComponent(e) {}
    psGenericComponent(psGenericComponent&& mov) : T(std::move(mov)), mgComponent(std::move(mov)) {}
    psGenericComponent& operator=(psGenericComponent&& mov) { T::operator=(std::move(mov)); mgComponent::operator=(std::move(mov)); return *this; }
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
    virtual const char* GetName() const override { return "Planeshader"; }
    template<class F>
    inline void SetPreprocess(F && f) { _f = std::forward<F>(f); }

  protected:
    virtual void _process(mgEntity& root, const planeshader::psTransform2D& prev);

    std::function<void()> _f;
  };

  struct MG_DLLEXPORT psGUIComponent : mgComponent<psGUIComponent, true>
  {
    explicit psGUIComponent(mgEntity* e = 0) : mgComponent(e) {}
    fgElement element;
  };
}

#endif