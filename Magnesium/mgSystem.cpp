// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgSystem.h"

using namespace magnesium;

mgSystemBase::mgSystemBase(int priority) : _priority(priority)
{

}
mgSystemBase::~mgSystemBase() { if(_manager) _manager->RemoveSystem(this); }

mgSystemSimple::mgSystemSimple(ComponentID iterator, int priority) : mgSystemBase(priority), _iterator(iterator) {}
mgSystemComplex::mgSystemComplex(size_t required, int priority) : mgSystemBase(priority), _required(required) {}

mgSystemManager::mgSystemManager()
{
}
mgSystemManager::~mgSystemManager()
{
  for(auto& s : _systems)
    s.first->_manager = 0;
}
void mgSystemManager::AddSystem(mgSystemBase* system, SystemID id)
{
  _systems.Insert(system, id);
}
bool mgSystemManager::RemoveSystem(SystemID id)
{
  return RemoveSystem(_systemhash.Get(id));
}
bool mgSystemManager::RemoveSystem(mgSystemBase* system)
{
  if(!system) return false;
  system->_manager = 0;
  size_t index = _systems.Get(system);
  if(index >= _systems.Length())
    return false;

  _systemhash.Remove(_systems[index]);
  _systems.RemoveIndex(index);
  return true;
}
mgSystemBase::mgMessageResult mgSystemManager::MessageSystem(SystemID id, ptrdiff_t m, void* p)
{
  mgSystemBase* system = _systemhash.Get(id);
  return (!system) ? mgSystemBase::mgMessageResult{ 0 } : system->Message(m, p);
}

void mgSystemManager::Process()
{
  for(auto& s : _systems)
  {
    mgRefCounter::GrabAll();
    s.first->Process();
    mgRefCounter::DropAll();
  }
}