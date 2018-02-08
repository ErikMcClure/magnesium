// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgLogic.h"

using namespace magnesium;

static_assert(!std::is_polymorphic<mgLogicComponent>::value, "Components shouldn't have a vtable");

LogicSystem::LogicSystem() {}
LogicSystem::~LogicSystem() {}

void LogicSystem::Process()
{
  SimpleIterator<LogicSystem, mgLogicComponent>::Gen<&LogicSystem::Iterator>(this);
}

void LogicSystem::Iterator(mgLogicComponent& logic)
{
  if(logic.onlogic)
    (*logic.onlogic)(*logic.entity);
}
