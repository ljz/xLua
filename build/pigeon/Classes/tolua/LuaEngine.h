#ifndef _LUA_ENGINE_H__
#define _LUA_ENGINE_H__

extern "C" {
#include "lua.h"
}
#include <vector>
#include <mutex>

class luaStack;
using namespace std;
class LuaEngine
{
public:
    static LuaEngine* getInstance(void);

    virtual ~LuaEngine(void);
    
    void setLuaState(lua_State* l);

	lua_State* getLuaState();

    bool init(void);

    void update();

    void addFunction(const std::function<void()> &function);

    void removeAllFunctions();

private:
    static LuaEngine* _instance;
	lua_State *_luaState;
    std::vector<std::function<void()>> _functionsToPerform;
    std::mutex _performMutex;
};

#endif // _LUA_ENGINE_H__
