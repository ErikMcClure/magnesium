// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgComponent.h"
#include "mgEngine.h"

using namespace magnesium;
using namespace bss_util;

mgEntity mgEntity::root(false);
mgEntity mgEntity::NIL(true);

mgEntity::mgEntity(bool isNIL) : TRB_NodeBase<mgEntity>(&NIL, !isNIL), id(0), graphcomponents(0), childhint(0), _order(0), _children(&NIL), _first(0), _last(0), _parent(0)
{
}
mgEntity::mgEntity(mgEntity* parent, int order) : TRB_NodeBase<mgEntity>(&NIL), id(0), graphcomponents(0), childhint(0), _order(order), _children(&NIL), _first(0), _last(0)
{
  _parent = !parent ? &root : parent;
  _parent->_addchild(this);
}
mgEntity::mgEntity(mgEntity&& mov) : TRB_NodeBase<mgEntity>(&NIL), id(mov.id), graphcomponents(mov.graphcomponents), _componentlist(std::move(mov._componentlist)),
childhint(mov.childhint), _parent(mov._parent), _order(mov._order), _children(mov._children), _first(mov._first), _last(mov._last)
{
  if(_parent)
  {
    _parent->_removechild(&mov);
    _parent->_addchild(this);
  }
  mov.graphcomponents = 0;
  mov.childhint = 0;
  mov.id = 0;
  mov._children = 0;
  mov._first = 0;
  mov._last = 0;
  mgEntity* cur = _first;
  while(cur)
  {
    assert(cur->_parent == &mov);
    cur->_parent = this;
    cur = cur->next;
  }
  for(auto& iter : _componentlist)
    *mgComponentStoreBase::GetStore(iter.first)->GetEntity(iter.second) = this;
}
mgEntity::~mgEntity()
{
  mgEntity* cur = _first;
  while(cur)
  {
    int r;
    if((r = cur->Drop()) > 0)
    {
      MGLOGF(1, "Released child {0} but had {1} references remaining", cur, r);
      assert(false); // Ideally this should never happen
      if(_parent) // However in release mode, we can't crash, so we attempt to move this child to our parent instead
        _parent->_addchild(cur);
      else
        MGLOGF(1, "Child {0} leaked because there was no parent to fall back to!", cur);
    }
    cur = cur->next;
  }
  for(const auto& pair : _componentlist)
    mgComponentStoreBase::RemoveComponent(pair.first, pair.second);
  if(_parent)
    _parent->_removechild(this);
}

void mgEntity::SetParent(mgEntity* parent)
{
  if(parent == _parent)
    return;
  if(_parent)
    _parent->_removechild(this);
  if(this != &root)
  {
    _parent = !parent ? &root : parent;
    _parent->_addchild(this);
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
  TRB_NodeBase<mgEntity>::InsertNode<&mgEntity::Comp>(child, _children, _first, _last, &NIL);
}
void mgEntity::_removechild(mgEntity* child)
{
  assert(child->_parent == this);
  TRB_NodeBase<mgEntity>::RemoveNode(child, _children, _first, _last, &NIL);
  if(!_first)
    childhint = 0;
}
