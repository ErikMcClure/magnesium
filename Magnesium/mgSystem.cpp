// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgSystemManager.h"

using namespace magnesium;

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

mgSystemState::mgSystemState(void(*process)(), const char* name, SystemID id, int priority) : 
  mgSystemBase(priority), _process(process), _name(name), _id(id)
{}
mgSystemState::mgSystemState(mgSystemState&& mov) : mgSystemBase(std::move(mov)), _process(mov._process), _name(mov._name),
  _id(mov._id) 
{}
mgSystemState& mgSystemState::operator=(mgSystemState&& mov)
{
  _process = mov._process;
  _name = mov._name;
  const_cast<SystemID&>(_id) = mov._id;
  mgSystemBase::operator=(std::move(mov));
  return *this;
}
