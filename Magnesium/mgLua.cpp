// Copyright ©2017 Black Sphere Studios

#include "mgLua.h"
#include "mgEngine.h"
#include "mgPlaneshader.h"
#include <sstream>

using namespace magnesium;

LuaSystem::LuaSystem(int priority)
{
  _l = lua_open();
  luaL_openlibs(_l);
  lua_register(_l, "print", &LuaSystem::lua_Print);
}
LuaSystem::~LuaSystem()
{
  lua_gc(_l, LUA_GCCOLLECT, 0);
  lua_close(_l);
  _l = 0;
}
void LuaSystem::Process()
{
}

const char* LuaSystem::_getError()
{
  const char* m = lua_tostring(_l, -1);
  return !m ? "[NULL]" : m;
}

void LuaSystem::_popError()
{
  if(lua_gettop(_l) > 0)
    lua_pop(_l, 1);
}

void LuaSystem::WriteError(int r, const char* name)
{
  if(!r)
    return;
  if(r == LUA_ERRSYNTAX)
  {
    if(name)
      MGLOG(2, "Syntax error in ", name, ": ", _getError());
    else
      MGLOG(2, "Syntax error: ", _getError());
    _popError();
  }
  else if(r == LUA_ERRRUN)
  {
    if(name)
      MGLOG(2, "Runtime error in ", name, ": ", _getError());
    else
      MGLOG(2, "Runtime error: ", _getError());
    _popError();
  }
  else if(!lua_isnil(_l, -1))
  {
    if(name)
      MGLOG(2, "Error ", r, " loading (", name, "): ", _getError());
    else
      MGLOG(2, "Error ", r, ": ", _getError());
    _popError();
  }
  else if(name)
    MGLOG(2, "Error loading lua chunk (", name, "): ", r);
  else
    MGLOG(2, "Error loading lua chunk: ", r);
}
int LuaSystem::Load(std::istream& s, const char* name)
{
  int r = lua_load(_l, &_luaStreamReader, &s, name);
  
  if(!r)
  {
    if(!name)
      r = lua_pcall(_l, 0, LUA_MULTRET, 0);
    else
      lua_setfield(_l, LUA_GLOBALSINDEX, name);
  }

  WriteError(r, name);
  return r;
}

int LuaSystem::Load(std::istream& s, std::ostream& out)
{
  int r = lua_load(_l, &_luaStreamReader, &s, 0);

  if(!r)
  {
    r = lua_pcall(_l, 0, LUA_MULTRET, 0);
    if(!r)
      _print(_l, out);
  }

  WriteError(r, 0);
  return r;
}
int LuaSystem::Load(const char* script, std::ostream& out)
{
  std::istringstream ss(script);
  return Load(ss, out);
}
int LuaSystem::Load(const char* script)
{
  std::istringstream ss(script);
  return Load(ss, 0);
}

int LuaSystem::Require(const char *name) {
  lua_getglobal(_l, "require");
  lua_pushstring(_l, name);
  int r = lua_pcall(_l, 1, 1, 0);
  if(!r)
    lua_pop(_l, 1);
  WriteError(r, name);
  return r;
}
int LuaSystem::AppendPath(const char* path)
{
  lua_getglobal(_l, "package");
  lua_getfield(_l, -1, "path"); // get field "path" from table at top of stack (-1)
  std::string npath = path;
  npath.append(";");
  npath.append(lua_tostring(_l, -1)); // grab path string from top of stack
  lua_pop(_l, 1);
  lua_pushstring(_l, npath.c_str());
  lua_setfield(_l, -2, "path"); // set the field "path" in table at -2 with value at top of stack
  lua_pop(_l, 1); // get rid of package table from top of stack
  return 0;
}
const char* LuaSystem::_luaStreamReader(lua_State *L, void *data, size_t *size)
{
  static char buf[CHUNKSIZE];
  reinterpret_cast<std::istream*>(data)->read(buf, CHUNKSIZE);
  *size = reinterpret_cast<std::istream*>(data)->gcount();
  return buf;
}

int LuaSystem::_print(lua_State *L, std::ostream& out) {
  int n = lua_gettop(L);  /* number of arguments */
  if(!n)
    return 0;
  int i;
  lua_getglobal(L, "tostring");
  for(i = 1; i <= n; i++)
  {
    const char *s;
    lua_pushvalue(L, -1);  /* function to be called */
    lua_pushvalue(L, i);   /* value to print */
    lua_call(L, 1, 1);
    s = lua_tostring(L, -1);  /* get result */
    if(s == NULL)
      return luaL_error(L, LUA_QL("tostring") " must return a string to "
        LUA_QL("print"));
    if(i > 1) out << "\t";
    out << s;
    lua_pop(L, 1);  /* pop result */
  }
  out << std::endl;
  return 0;
}

int LuaSystem::lua_Print(lua_State *L) {
  return _print(L, mgEngine::Instance()->GetLog().GetStream());
}

SystemID LuaSystem::RegisterSystem(void(*process)(), int priority, const char* name)
{
  size_t i = _systems.AddConstruct(process, name, mgSystemManager::GenerateSystemID(), priority);
  _manager->AddSystemState(&_systems[i]);
  return _systems[i].ID();
}
mgSystemBase::mgMessageResult LuaSystem::MessageSystem(const char* name, ptrdiff_t m, void* p)
{
  if(mgSystemBase* sys = mgEngine::Instance()->GetSystem(name))
    return sys->Message(m, p);
  return mgMessageResult{ 0 };
}
mgComponentCounter* LuaSystem::GetEntityComponent(mgEntity* e, const char* component)
{
  return !e ? nullptr : e->GetByID(GetComponentID(component));
}