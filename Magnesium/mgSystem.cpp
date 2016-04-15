// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgSystem.h"

using namespace magnesium;

mgSystemBase::mgSystemBase(ComponentBitfield required, ComponentID iterator, int priority) : _required(required), _iterator(iterator), _priority(priority)
{

}
mgSystemBase::~mgSystemBase() { if(_manager) _manager->RemoveSystem(this); }
void mgSystemBase::Preprocess() {}
void mgSystemBase::Postprocess() {}

mgSystemManager::mgSystemManager()
{
}
mgSystemManager::~mgSystemManager()
{
  for(mgSystemBase* s : _systems)
    s->_manager = 0;
}
void mgSystemManager::AddSystem(mgSystemBase* system)
{
  _systems.Insert(system);
}
bool mgSystemManager::RemoveSystem(mgSystemBase* system)
{
  system->_manager = 0;
  return _systems.Remove(_systems.Find(system));
}
void mgSystemManager::Process()
{
  for(mgSystemBase* s : _systems)
  {
    mgRefCounter::GrabAll();
    s->Preprocess();
    mgComponentStoreBase* store = mgComponentStoreBase::GetStore(s->_iterator);
    if(store != 0)
    {
      auto& entities = store->GetEntityArray();
      store->curIteration = 0;

      for(size_t i = 0; i < entities.Length(); ++i)
      {
        ++store->curIteration;
        s->Process(entities[i]);
      }
      store->FlushBuffer(); // Delete all the entities whose deletion was postponed because we had already iterated over them.
    }
    s->Postprocess();
    mgRefCounter::DropAll();
  }
}