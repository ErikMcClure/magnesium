// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgComponent.h"

using namespace magnesium;
using namespace bss;

ComponentID mgComponentCounter::curID = 0;
ComponentID mgComponentCounter::curGraphID = 1;
DynArray<mgComponentStoreBase*> mgComponentStoreBase::_stores;

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

mgComponentCounter* mgComponentStoreBase::GetComponent(ComponentID id, size_t index)
{
  if(id >= _stores.Length() || !_stores[id])
    return nullptr;
  return _stores[id]->GetBase(index); // This virtual call can be eliminated if a copy of the base array pointer is maintained in the base component
}

mgComponentStoreBase* mgComponentStoreBase::GetStore(ComponentID id)
{
  return (id >= _stores.Length()) ? 0 : _stores[id];
}

void* mgComponentStoreBase::dllrealloc(void* p, size_t sz) { return realloc(p, sz); }
void mgComponentStoreBase::dllfree(void* p) { free(p); }

Hash<const char*, ComponentID> mgComponentIDNameHash;

ComponentID GetComponentID(const char* name)
{
  khiter_t i = mgComponentIDNameHash.Iterator(name);
  if(mgComponentIDNameHash.ExistsIter(i))
    return mgComponentIDNameHash.GetValue(i);
  return (ComponentID)~0;
}
void RegisterComponentID(ComponentID id, const char* name)
{
  mgComponentIDNameHash.Insert(name, id);
}