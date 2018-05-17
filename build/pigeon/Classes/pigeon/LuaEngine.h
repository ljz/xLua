#ifndef _LUA_ENGINE_H__
#define _LUA_ENGINE_H__

extern "C" {
#include "lua.h"
}

using namespace std;
class LuaEngine
{
public:
    static LuaEngine* getInstance(void);

    virtual ~LuaEngine(void);
    
    void setLuaState(lua_State* l);

    bool init(void);

    void update();

    void addFunction(const std::function<void()> &function);

    void removeAllFunctions();

private:
    static LuaEngine* _instance;
    luaStack *_luaState;
    std::vector<std::function<void()>> _functionsToPerform;
    std::mutex _performMutex;
};

#endif // _LUA_ENGINE_H__
