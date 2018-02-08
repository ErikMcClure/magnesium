// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __PLANESHADER_H__MG__
#define __PLANESHADER_H__MG__

#include "mgSystem.h"
#include "mgTimeline.h"
#include "planeshader\psImage.h"
#include "planeshader\psTileset.h"
#include "planeshader\psText.h"
#include "planeshader\psVector.h"
#include "planeshader\psRenderGeometry.h"
#include "planeshader\psEngine.h"
#include "planeshader\psLayer.h"

namespace magnesium {
  typedef mgComponentInherit<planeshader::psRenderable, true> psRenderableComponent;
  typedef mgComponentInherit<planeshader::psSolid, true> psSolidComponent;
  typedef mgComponentInherit<planeshader::psLocatable, true> psLocatableComponent;

  template<class T> using psRenderableBind = mgComponentInheritBind<planeshader::psRenderable, T, true>;
  template<class T> using psSolidBind = mgComponentInheritBind<planeshader::psSolid, T, true>;

  template<typename T>
  struct MG_DLLEXPORT psLocatableBind : psLocatableComponent
  {
    explicit psLocatableBind(mgEntity* e = 0) : psLocatableComponent(e, &CastComponent) 
    {
      Event<EVENT_SETPOSITION>::RegisterRaw<psLocatableBind>(e, &psLocatableBind::_setPosition);
      Event<EVENT_SETPOSITION_INTERPOLATE>::RegisterRaw<psLocatableBind>(e, &psLocatableBind::_setPosition);
      Event<EVENT_SETROTATION>::RegisterRaw<psLocatableBind>(e, &psLocatableBind::_setRotation);
    }
    static planeshader::psLocatable* CastComponent(mgEntity* e) { return static_cast<planeshader::psLocatable*>(e->Get<T>()); }
    static void _setPosition(mgComponentCounter* c, float x, float y) { static_cast<psLocatableComponent*>(c)->Get()->SetPosition(x, y); }
    static void _setRotation(mgComponentCounter* c, float r) { static_cast<psLocatableComponent*>(c)->Get()->SetRotation(r); }
  };

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

  struct MG_DLLEXPORT psLayerComponent : planeshader::psLayer, mgComponent<psLayerComponent, false, bss::ARRAY_SAFE, psRenderableBind<psLayerComponent>>
  {
    explicit psLayerComponent(mgEntity* e = 0) : mgComponent(e) {}
    psLayerComponent(psLayerComponent&& mov) : planeshader::psLayer(std::move(mov)), mgComponent(std::move(mov)) {}
    psLayerComponent& operator=(psLayerComponent&& mov) { planeshader::psLayer::operator=(std::move(mov)); mgComponent::operator=(std::move(mov)); return *this; }
  };

  template<typename T, typename... Args>
  class MG_DLLEXPORT psEffectComponent : public mgEffect<T, Args...>, public planeshader::psRenderable, public mgComponent<psEffectComponent<T, Args...>, false, bss::ARRAY_SAFE, psRenderableBind<psEffectComponent<T, Args...>>>
  {
  public:
    psEffectComponent(psEffectComponent&& mov) : mgEffect<T, Args...>(std::move(mov)), planeshader::psRenderable(std::move(mov)), mgComponent(std::move(mov)) {}
    psEffectComponent(const psEffectComponent& copy) = default;
    explicit psEffectComponent(mgEntity* e = 0) : mgComponent(e) {}
    template<typename U, typename... X>
    BSS_FORCEINLINE void Add(size_t id, X... args)
    {
      _instances.AddConstruct<U>(U(args...));
      Bind<U>(&_instances.Back().Get<U>(), id);
    }

    psEffectComponent& operator=(const psEffectComponent& copy) = default;
    psEffectComponent& operator=(psEffectComponent&& mov)
    {
      mgEffect<T, Args...>::operator=(std::move(mov));
      planeshader::psRenderable::operator=(std::move(mov));
      mgComponent::operator=(std::move(mov));
    }

  protected:
    virtual void _render(const planeshader::psTransform2D& parent) override
    {
      for(auto& v : _instances)
        if(!v.convert<planeshader::psRenderable>().Cull(parent))
          v.convert<planeshader::psRenderable>()._render(parent);
    }
  };

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
}

#endif