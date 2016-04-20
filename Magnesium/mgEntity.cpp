// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgComponent.h"

using namespace magnesium;
using namespace bss_util;

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