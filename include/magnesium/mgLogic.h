// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __LOGIC_H__MG__
#define __LOGIC_H__MG__

#include "mgSystem.h"

namespace magnesium {
  struct mgLogicComponent : mgComponent<mgLogicComponent, false, bss::ARRAY_MOVE> {
    explicit mgLogicComponent(mgEntity* e = 0) : mgComponent(e) {}
    void (*onlogic)(mgEntity&);
  };

  class MG_DLLEXPORT LogicSystem : public mgSystem<mgLogicComponent>
  {
  public:
    LogicSystem();
    ~LogicSystem();
    virtual void Process() override;

    static void Iterator(mgSystemBase*, mgEntity& entity);
    std::function<void()> process;
  };
}

#endif