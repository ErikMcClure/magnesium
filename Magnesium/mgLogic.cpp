// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgLogic.h"

using namespace magnesium;

LogicSystem::LogicSystem() {}
LogicSystem::~LogicSystem() {}

void LogicSystem::Preprocess() { if(preprocess) preprocess(); }
void LogicSystem::Process(mgEntity* entity)
{
  auto& f = entity->Get<mgLogicComponent>()->onlogic;
  if(f) f(entity);
}
void LogicSystem::Postprocess() { if(postprocess) postprocess(); }