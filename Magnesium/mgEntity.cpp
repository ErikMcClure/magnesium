// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgComponent.h"

using namespace magnesium;
using namespace bss_util;

mgEntity mgEntity::_root(0);

mgEntity::mgEntity(mgEntity* parent, int order) : id(0), graphcomponents(0), childhint(0), _order(order)
{
  if(this == &_root)
  {
    _parent = 0;
  }
  else
  {
    _parent = !parent ? &_root : parent;
    _parent->Grab();
    _parent->_addchild(this);
  }
}
mgEntity::mgEntity(mgEntity&& mov) : id(mov.id), graphcomponents(mov.graphcomponents), _componentlist(std::move(mov._componentlist)), childhint(mov.childhint),
  _children(std::move(mov._children)), _parent(mov._parent), _order(mov._order)
{
  if(_parent)
  {
    _parent->_removechild(&mov);
    _parent->_addchild(this);
  }
  mov.graphcomponents = 0;
  mov.childhint = 0;
  mov.id = 0;
  for(auto& iter : _componentlist)
    *mgComponentStoreBase::GetStore(iter.first)->GetEntity(iter.second) = this;
}
mgEntity::~mgEntity()
{
  assert(!_children.Length());
  for(const auto& pair : _componentlist)
    mgComponentStoreBase::RemoveComponent(pair.first, pair.second);
  if(_parent)
  {
    _parent->_removechild(this);
    _parent->Drop();
  }
}

void mgEntity::SetParent(mgEntity* parent)
{
  if(parent == _parent)
    return;
  if(_parent)
  {
    _parent->_removechild(this);
    _parent->Drop();
  }
  if(this != &_root)
  {
    _parent = !parent ? &_root : parent;
    _parent->_addchild(this);
    _parent->Grab();
    _propagateIDs();
  }
}

void mgEntity::SetOrder(int order)
{
  if(order != _order)
  {
    _parent->_removechild(this);
    _order = order;
    _parent->_addchild(this);
  }
}
void mgEntity::ComponentListInsert(ComponentID id, ComponentID graphid, size_t index)
{
  _componentlist.Insert(id, index);
  if(graphid)
  {
    graphcomponents |= graphid;
    _propagateIDs();
  }
}
void mgEntity::ComponentListRemove(ComponentID id, ComponentID graphid)
{
  graphcomponents &= (~graphid);
  _componentlist.Remove(id);
}
size_t& mgEntity::ComponentListGet(ComponentID id)
{
  return _componentlist[_componentlist.Get(id)];
}

void mgEntity::_propagateIDs()
{
  mgEntity* p = _parent;
  while(p)
  {
    p->childhint |= graphcomponents;
    p = p->_parent;
  }
}

void mgEntity::_addchild(mgEntity* child)
{
  assert(child->_parent == this);

  if(_children.Empty())
    _children.Add(child);
  else
  {
    ptrdiff_t loc;
    if(mgEntity::Comp(child, _children.Front()) < 0) loc = 0;
    else if(mgEntity::Comp(child, _children.Back()) >= 0) loc = _children.Length();
    else loc = binsearch_after<mgEntity*, ptrdiff_t, &mgEntity::Comp>(_children.begin(), _children.Length(), child);
    _children.Insert(child, loc);
  }
}
void mgEntity::_removechild(mgEntity* child)
{
  assert(child->_parent == this);

  ptrdiff_t i = binsearch_exact<mgEntity*, mgEntity*, ptrdiff_t, &mgEntity::Comp>(_children.begin(), child, 0, _children.Length());

  if(i >= 0)
    _children.Remove(i);
  if(!_children.Length())
    childhint = COMPONENTBITFIELD_EMPTY;
}
