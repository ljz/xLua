#include "LuaEngine.h"

LuaEngine* LuaEngine::_instance = nullptr;

LuaEngine* LuaEngine::getInstance(void)
{
    if (!_instance)
    {
        _instance = new (std::nothrow)LuaEngine();
        _instance->init();
    }
    return _instance;
}

LuaEngine::~LuaEngine(void)
{
    _instance = nullptr;
}

void LuaEngine::setLuaState(lua_State* l)
{
    _luaState = l;
}

void LuaEngine::getLuaState()
{
    return _luaState;
}

bool LuaEngine::init(void)
{
    _functionsToPerform.reserve(30);
    return true;
}

void LuaEngine:update()
{
    if( !_functionsToPerform.empty() ) {
        _performMutex.lock();
        auto temp = _functionsToPerform;
        _functionsToPerform.clear();
        _performMutex.unlock();
        for( const auto &function : temp ) {
            function();
        }
    }
}

void LuaEngine::addFunction(const std::function<void ()> &function)
{
    _performMutex.lock();
    _functionsToPerform.push_back(function);
    _performMutex.unlock();
}

void LuaEngine::removeAllFunctions()
{
    _performMutex.lock();
    _functionsToPerform.clear();
    _performMutex.unlock();
}