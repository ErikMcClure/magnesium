// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgSystem.h"

using namespace magnesium;

SystemID mgSystemManager::sysid = 0;

mgSystemBase::mgSystemBase(int priority) : _priority(priority), _manager(0)
{
}
mgSystemBase::mgSystemBase(mgSystemBase&& mov) : _priority(mov._priority), _manager(0)
{
  if(mov._manager)
  {
    SystemID id = mov._manager->RemoveSystem(&mov);
    if(id != (SystemID)~0)
      mov._manager->AddSystem(this, id);
  }
}
mgSystemBase::~mgSystemBase() { if(_manager) _manager->RemoveSystem(this); }

mgSystemBase& mgSystemBase::operator=(mgSystemBase&& mov)
{
  if(_manager)
    _manager->RemoveSystem(this);
  const_cast<int&>(_priority) = mov._priority; // This is the only line of code allowed to change _priority after being constructed
  if(mov._manager)
  {
    SystemID id = mov._manager->RemoveSystem(&mov);
    if(id != (SystemID)~0)
      mov._manager->AddSystem(this, id);
  }
  return *this;
}

mgSystemSimple::mgSystemSimple(ComponentID iterator, int priority) : mgSystemBase(priority), _iterator(iterator) {}
mgSystemSimple::mgSystemSimple(mgSystemSimple&& mov) : mgSystemBase(std::move(mov)), _iterator(mov._iterator) {}
mgSystemSimple& mgSystemSimple::operator=(mgSystemSimple&& mov)
{
  const_cast<ComponentID&>(_iterator) = mov._iterator;
  mgSystemBase::operator=(std::move(mov));
  return *this;
}

mgSystemComplex::mgSystemComplex(size_t required, int priority) : mgSystemBase(priority), _required(required) {}
mgSystemComplex::mgSystemComplex(mgSystemComplex&& mov) : mgSystemBase(std::move(mov)), _required(mov._required) {}
mgSystemComplex& mgSystemComplex::operator=(mgSystemComplex&& mov)
{
  const_cast<size_t&>(_required) = mov._required;
  mgSystemBase::operator=(std::move(mov));
  return *this;
}

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
  _systemhash.Insert(id, system);
  if(const char* name = system->GetName())
    _systemname.Insert(name, system);
  system->_manager = this;
}
bool mgSystemManager::RemoveSystem(SystemID id)
{
  return RemoveSystem(_systemhash.Get(id)) != (SystemID)~0;
}
SystemID mgSystemManager::RemoveSystem(mgSystemBase* system)
{
  if(!system) return false;
  system->_manager = 0;
  size_t index = _systems.Get(system);
  if(index >= _systems.Length())
    return (SystemID)~0;

  SystemID id = _systems[index];
  _systemhash.Remove(_systems[index]);
  _systems.RemoveIndex(index);
  return id;
}
mgSystemBase::mgMessageResult mgSystemManager::MessageSystem(SystemID id, ptrdiff_t m, void* p)
{
  mgSystemBase* system = _systemhash.Get(id);
  return (!system) ? mgSystemBase::mgMessageResult{ 0 } : system->Message(m, p);
}
mgSystemBase* mgSystemManager::GetSystem(SystemID id) const
{
  return _systemhash[id];
} 
mgSystemBase* mgSystemManager::GetSystem(const char* name) const
{
  return _systemname[name];
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

mgSystemState::mgSystemState(void(*process)(), const char* name, SystemID id, int priority, mgMessageResult(*message)(ptrdiff_t m, void* p)) : 
  mgSystemBase(priority), _process(process), _name(name), _id(id), _message(message)
{}
mgSystemState::mgSystemState(mgSystemState&& mov) : mgSystemBase(std::move(mov)), _process(mov._process), _name(mov._name),
  _id(mov._id), _message(mov._message) 
{}
mgSystemState& mgSystemState::operator=(mgSystemState&& mov)
{
  _process = mov._process;
  _name = mov._name;
  const_cast<SystemID&>(_id) = mov._id;
  _message = mov._message;
  mgSystemBase::operator=(std::move(mov));
  return *this;
}