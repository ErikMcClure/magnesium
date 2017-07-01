// Copyright ©2017 Black Sphere Studios

#include "mgLua.h"
#include "mgEngine.h"

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

int LuaSystem::Load(std::istream& s, const char* name)
{
  int r = lua_load(_l, &_luaStreamReader, &s, name);
  if(!name)
    return lua_pcall(_l, 0, LUA_MULTRET, 0);
  else
    lua_setfield(_l, LUA_GLOBALSINDEX, name);
  return r;
}

const char* LuaSystem::_luaStreamReader(lua_State *L, void *data, size_t *size)
{
  static char buf[CHUNKSIZE];
  reinterpret_cast<std::istream*>(data)->read(buf, CHUNKSIZE);
  *size = reinterpret_cast<std::istream*>(data)->gcount();
  return buf;
}

int LuaSystem::lua_Print(lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  int i;
  lua_getglobal(L, "tostring");
  std::ostream& out = mgEngine::Instance()->GetLog().GetStream();
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