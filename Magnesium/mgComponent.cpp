// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgComponent.h"

using namespace magnesium;
using namespace bss_util;

ComponentID mgComponentCounter::curID = 0;
cDynArray<mgComponentStoreBase*> mgComponentStoreBase::_stores;
mgEntity* mgEntity::_entitylist = 0;

mgEntity::mgEntity() : id(0), components(COMPONENTBITFIELD_EMPTY)
{
  LLAdd<mgEntity>(this, _entitylist);
}
mgEntity::mgEntity(mgEntity&& mov) : id(mov.id), components(mov.components), _componentlist(std::move(mov._componentlist))
{
  prev = mov.prev;
  next = mov.next;
  if(prev) prev->next = this;
  else _entitylist = this;
  if(next) next->prev = this;
  mov.prev = 0;
  mov.next = 0;
  mov.components.bits = 0;
  mov.id = 0;
  for(auto& iter : _componentlist)
    *mgComponentStoreBase::GetStore(iter.first)->GetEntity(iter.second) = this;
}
mgEntity::~mgEntity()
{
  for(const auto& pair : _componentlist)
    mgComponentStoreBase::RemoveComponent(pair.first, pair.second);
  LLRemove<mgEntity>(this, _entitylist);
}

void mgEntity::ComponentListInsert(ComponentID id, size_t index)
{
  _componentlist.Insert(id, index);
}
void mgEntity::ComponentListRemove(ComponentID id)
{
  _componentlist.Remove(id);
}
size_t& mgEntity::ComponentListGet(ComponentID id)
{
  return _componentlist[_componentlist.Get(id)];
}
mgComponentStoreBase::mgComponentStoreBase(ComponentID id) : _id(id), curIteration(0)
{
  if(_stores.Length() <= _id)
  {
    size_t len = _stores.Length();
    _stores.SetLength(_id + 1);
    for(size_t i = len; i < _stores.Length(); ++i)
      _stores[i] = 0;
  }
  _stores[_id] = this;
}
mgComponentStoreBase::~mgComponentStoreBase()
{ // Note: we remove all objects in the destructor above this one because it relies on a virtual function call that gets destroyed by the time we reach this
}

bool mgComponentStoreBase::RemoveComponent(ComponentID id, size_t index)
{
  if(id >= _stores.Length() || !_stores[id])
    return false;
  return _stores[id]->RemoveInternal(id, index);
}
size_t mgComponentStoreBase::MessageComponent(ComponentID id, size_t index, void* msg, ptrdiff_t msgint)
{
  if(id >= _stores.Length() || !_stores[id])
    return 0;
  return _stores[id]->MessageComponent(index, msg, msgint);
}

mgComponentStoreBase* mgComponentStoreBase::GetStore(ComponentID id)
{
  return (id >= _stores.Length()) ? 0 : _stores[id];
}

void* mgComponentStoreBase::dllrealloc(void* p, size_t sz) { return realloc(p, sz); }
void mgComponentStoreBase::dllfree(void* p) { free(p); }