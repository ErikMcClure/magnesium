// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgSystemManager.h"
#include "bss-util/literals.h"

using namespace magnesium;

SystemID mgSystemManager::sysid = 0;

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
  system->_register();
}
bool mgSystemManager::RemoveSystem(SystemID id)
{
  return RemoveSystem(_systemhash.Get(id)) != (SystemID)~0;
}
SystemID mgSystemManager::RemoveSystem(mgSystemBase* system)
{
  if(!system)
    return (SystemID)~0;
  system->_manager = 0;
  size_t index = _systems.Get(system);
  if(index >= _systems.Length())
    return (SystemID)~0;

  SystemID id = _systems[index];
  _systemhash.Remove(_systems[index]);
  _systems.RemoveIndex(index);

  for(auto i : system->_activemsgs)
    _messagelist.Remove(i);

  system->_activemsgs.Clear();
  return id;
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
    RunDeferred();
    mgRefCounter::DropAll();
  }
}
void mgSystemManager::RunDeferred()
{
  while(_defer.size() > 0)
  {
    _defer.back()();
    _defer.pop_back();
  }
}
void mgSystemManager::_registerMessage(MessageID msg, SystemID id, void(*f)())
{
  if(khiter_t iter = _messagelist.Iterator(id); _messagelist.ExistsIter(iter))
  {
    if(mgSystemBase* old = GetSystem(_messagelist.GetValue(iter)->first))
    {
      if(size_t index = old->_activemsgs.Find(msg); index != ~0_sz)
        old->_activemsgs.Remove(index);
    }
  }

  if(mgSystemBase* sys = _systemhash[id])
  {
    _messagelist.Insert(msg, std::pair<ComponentID, void(*)()>(id, f));
    sys->_activemsgs.Insert(msg);
  }
}
