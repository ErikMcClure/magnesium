// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgLogic.h"

using namespace magnesium;

LogicSystem::LogicSystem() {}
LogicSystem::~LogicSystem() {}

void LogicSystem::Iterate(mgEntity* entity)
{
  auto& f = entity->Get<mgLogicComponent>()->onlogic;
  if(f) f(entity);
}
