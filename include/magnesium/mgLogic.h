// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __LOGIC_H__MG__
#define __LOGIC_H__MG__

#include "mgSystem.h"
#include <functional>

namespace magnesium {
  struct mgLogicComponent : mgComponent<mgLogicComponent, false, bss::ARRAY_MOVE> {
    explicit mgLogicComponent(mgEntity* e = 0) : mgComponent(e), onlogic(0) {}
    void (*onlogic)(mgEntity&);
  };

  class MG_DLLEXPORT LogicSystem : public mgSystem<mgLogicComponent>
  {
  public:
    LogicSystem();
    ~LogicSystem();
    virtual void Process() override;
    virtual const char* GetName() const override { return "Logic"; }
    void Iterator(mgLogicComponent& logic);

    std::function<void()> process;
  };
}

#endif