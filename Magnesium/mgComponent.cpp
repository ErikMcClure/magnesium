// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgComponent.h"

using namespace magnesium;
using namespace bss_util;

ComponentID mgComponentCounter::curID = 0;
cDynArray<mgComponentStoreBase*> mgComponentStoreBase::_stores;

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