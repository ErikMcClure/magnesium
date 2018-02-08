// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#include "mgComponent.h"
#include "mgEngine.h"

using namespace magnesium;
using namespace bss;

bss::CachePolicy<char> MagnesiumAllocatorBase::GeneralAlloc;
void* MagnesiumAllocatorBase::allocate(size_t cnt, void* p, size_t old) { return GeneralAlloc.allocate(cnt, reinterpret_cast<char*>(p), old); }
void MagnesiumAllocatorBase::deallocate(void* p, size_t sz) { GeneralAlloc.deallocate(reinterpret_cast<char*>(p), sz); }

mgEntity mgEntity::NIL(true);
bss::LocklessBlockPolicy<mgEntity> mgEntity::EntityAlloc(32);

mgEntity::mgEntity(bool isNIL) : TRB_NodeBase<mgEntity>(&NIL, !isNIL), entityId(0), graphcomponents(0), childhint(0), _order(0), _children(&NIL), _first(0), _last(0), _parent(0), _name(0)
{
}
mgEntity::mgEntity(mgEntity* e, int order, ComponentID hint) : TRB_NodeBase<mgEntity>(&NIL), entityId(0), graphcomponents(0), childhint(0), _componentlist(hint),
  _order(order), _children(&NIL), _first(0), _last(0), _name(0)
{
  _parent = !e ? _getRoot() : e;
  _parent->_addchild(this);
}
mgEntity::mgEntity(mgEntity&& mov) : TRB_NodeBase<mgEntity>(&NIL), entityId(mov.entityId), graphcomponents(mov.graphcomponents), _componentlist(std::move(mov._componentlist)),
childhint(mov.childhint), _parent(mov._parent), _order(mov._order), _children(mov._children), _first(mov._first), _last(mov._last), _name(mov._name)
{
  if(_parent)
  {
    _parent->_removechild(&mov);
    _parent->_addchild(this);
  }
  mov.graphcomponents = 0;
  mov.childhint = 0;
  mov.entityId = 0;
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
  for(auto[id, index] : _componentlist)
    *mgComponentStoreBase::GetStore(id)->GetEntity(index) = this;
}
mgEntity::~mgEntity()
{
  mgEntity* cur = _first;
  while(cur)
  {
    int r;
    mgEntity* hold = cur;
    cur = cur->next;
    if((r = hold->Drop()) > 0)
    {
      MGLOGF(1, "Released child {0} but had {1} references remaining", cur, r);
      assert(false); // Ideally this should never happen
      if(_parent) // However in release mode, we can't crash, so we attempt to move this child to our parent instead
        _parent->_addchild(cur);
      else
        MGLOGF(1, "Child {0} leaked because there was no parent to fall back to!", cur);
    }
  }
  for(auto[id, index] : _componentlist)
    mgComponentStoreBase::RemoveComponent(id, index);
  if(_parent)
    _parent->_removechild(this);
}
void* mgEntity::operator new(std::size_t sz) { assert(sz == sizeof(mgEntity)); assert(mgEngine::Instance()); return EntityAlloc.allocate(1); }
void mgEntity::operator delete(void* ptr, std::size_t sz) { EntityAlloc.deallocate((mgEntity*)ptr, 1); }

mgEntity* mgEntity::_getRoot()
{
  if(mgEngine::Instance())
    return &mgEngine::Instance()->SceneGraph();
  return 0;
}

void mgEntity::SetParent(mgEntity* e)
{
  if(e == _parent)
    return;
  if(_parent)
    _parent->_removechild(this);
  if(this != _getRoot())
  {
    _parent = !e ? _getRoot() : e;
    _parent->_addchild(this);
    _propagateIDs();
  }
}

void mgEntity::SetOrder(int order)
{
  if(order != _order)
  {
    bool invalid = !_parent->_checkchild(this);
    if(invalid)
      _parent->_removechild(this);
    _order = order;
    if(invalid)
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
  return _componentlist(_componentlist.Get(id));
}
mgComponentCounter* mgEntity::GetByID(ComponentID id)
{
  ComponentID index = _componentlist.Get(id);
  return index == (ComponentID)~0 ? nullptr : mgComponentStoreBase::GetComponent(id, _componentlist(index));
}
void mgEntity::Revalidate()
{
  if(!_parent->_checkchild(this))
  {
    _parent->_removechild(this);
    _parent->_addchild(this);
    assert(_parent->_checkchild(this));
  }
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
  assert(child->_parent == this);
}
void mgEntity::_removechild(mgEntity* child)
{
  assert(child->_parent == this);
  TRB_NodeBase<mgEntity>::RemoveNode(child, _children, _first, _last, &NIL);
  if(!_first)
    childhint = 0;
}
bool mgEntity::_checkchild(mgEntity* child)
{ // We have to validate the child in it's parent, because the parent owns the comparison function.
  return child->template Validate<&mgEntity::Comp>();
}
void mgEntity::_registerEvent(EventID event, ComponentID id, void(*f)())
{
  _eventlist.Insert(event, std::pair<ComponentID, void(*)()>(id, f));
}
void mgEntity::_registerHook(EventID event, void(mgEntity::*f)())
{
  _hooklist.Insert(event, f);
}
