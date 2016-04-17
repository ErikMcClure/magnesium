// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __LOGIC_H__MG__
#define __LOGIC_H__MG__

#include "mgSystem.h"

namespace magnesium {
  struct mgLogicComponent : mgComponent<mgLogicComponent, bss_util::CARRAY_MOVE> {
    explicit mgLogicComponent(mgEntity* e = 0) : mgComponent(e) {}
    void (*onlogic)(mgEntity*);
  };

  class MG_DLLEXPORT LogicSystem : public mgSystem<mgLogicComponent, mgLogicComponent>
  {
  public:
    LogicSystem();
    ~LogicSystem();
    virtual void Preprocess() override;
    virtual void Process(mgEntity* entity) override;
    virtual void Postprocess() override;

    std::function<void()> preprocess;
    std::function<void()> postprocess;
  };
}

#endif