// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgSystem.h"

using namespace magnesium;

mgSystemBase::mgSystemBase(int priority) : _priority(priority)
{

}
mgSystemBase::~mgSystemBase() { if(_manager) _manager->RemoveSystem(this); }

mgSystemSimple::mgSystemSimple(ComponentID iterator, int priority) : mgSystemBase(priority), _iterator(iterator)
{

}
mgSystemSimple::~mgSystemSimple() { if(_manager) _manager->RemoveSystem(this); }
void mgSystemSimple::Process()
{
  mgComponentStoreBase* store = mgComponentStoreBase::GetStore(_iterator);
  if(store != 0)
  {
    auto& entities = store->GetEntities();
    store->curIteration = 0;

    while(entities)
    {
      ++store->curIteration;
      Iterate(**entities);
      ++entities;
    }
    store->FlushBuffer(); // Delete all the entities whose deletion was postponed because we had already iterated over them.
  }
}

mgSystemComplex::mgSystemComplex(size_t required, int priority) : mgSystemBase(priority), _required(required)
{

}
mgSystemComplex::~mgSystemComplex() { if(_manager) _manager->RemoveSystem(this); }
void mgSystemComplex::Process()
{
  _process(mgEntity::SceneGraph());
}
void mgSystemComplex::_process(mgEntity& root)
{
  int i = 0;
  size_t l = root.NumChildren();
  mgEntity* const* p = root.Children();

  if(root.childhint&_required)
  {
    while(i < l && p[i]->Order() < 0)
      _process(*p[i++]);
  }
  if(root.graphcomponents&_required)
    Iterate(root);
  if(root.childhint&_required)
  {
    while(i < l)
      _process(*p[i++]);
  }
}

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
    s->Process();
    mgRefCounter::DropAll();
  }
}