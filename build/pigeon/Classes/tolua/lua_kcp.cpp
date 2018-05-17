//
//  lua_kcp.cpp
//  pigeon
//
//  Created by Vega on 2017/10/26.
//  Copyright © 2017年 gamebase. All rights reserved.
//

#include "lua_kcp.hpp"
#include "LuaBasicConversions.h"
//#include "scripting/lua-bindings/manual/cocos2d/LuaScriptHandlerMgr.h"
//#include "scripting/lua-bindings/manual/CCLuaStack.h"
//#include "scripting/lua-bindings/manual/CCLuaValue.h"
//#include "scripting/lua-bindings/manual/CCLuaEngine.h"
//#include "scripting/lua-bindings/manual/tolua_fix.h"
#include "kcp/UDPServer.h"
#include "tolua/LuaEngine.h"
#include "lua_pigeon.h"

#define COCOS2D_DEBUG 0

static int kcpLuaCall(lua_State* l, int nargs, int nresults) {
	int traceback = 0;
	int functionIndex = -(nargs + 1);
	if (!lua_isfunction(l, functionIndex)) {
		NLOG(NL_DEBUG, "[LUA ERROR] [%s:%d]: Trying to call a non-function.", __FILE__, __LINE__);
		return -1;
	}
	lua_getglobal(l, "__G__TRACKBACK__");                         /* L: ... func arg1 arg2 ... G */
	if (!lua_isfunction(l, -1)) {
		lua_pop(l, 1);                                            /* L: ... func arg1 arg2 ... */
	}
	else {
		lua_insert(l, functionIndex - 1);                         /* L: ... G func arg1 arg2 ... */
		traceback = functionIndex - 1;
	}
	int error = lua_pcall(l, nargs, nresults, traceback);         /* L: ... [G] {rets} */
	if (error) {
		const char* errmsg = lua_tostring(l, -1);
		NLOG(NL_DEBUG, "[LUA ERROR] [%s:%d]:\n%s", __FILE__, __LINE__, errmsg);
		lua_pop(l, (traceback == 0 ? 1 : 2));
	}
	else {
		// remove __G__TRACKBACK__ from stack
		// remove __G__TRACKBACK__ from stack
		//lua_remove(l, -nresults - 1);  /* L: ... {rets} */
		if (traceback != 0) {
			// remove __G__TRACKBACK__ from stack
			lua_remove(l, -nresults - 1);  /* L: ... {rets} */
		}
	}
	return error;
}

int lua_register_UDPServer_create(lua_State* tolua_S)
{
    int argc = 0;
    
#if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
#endif
    
#if COCOS2D_DEBUG >= 1
    if (!tolua_isusertable(tolua_S,1,"UDPServer",0,&tolua_err)) goto tolua_lerror;
#endif
    
    argc = lua_gettop(tolua_S) - 1;
    if (argc == 3)
    {
        bool ok = true;
        std::string arg0 ;
        ok &= luaval_to_std_string(tolua_S,2,&arg0,"UDPServer:create");
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function 'lua_register_UDPServer_create'", nullptr);
            return 0;
        }
        
        int arg1 ;
        ok &= luaval_to_int32(tolua_S,3,&arg1,"UDPServer:create");
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function 'lua_register_UDPServer_create'", nullptr);
            return 0;
        }
        int arg2 ;
        ok &= luaval_to_int32(tolua_S,4,&arg2,"UDPServer:create");
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function 'lua_register_UDPServer_create'", nullptr);
            return 0;
        }
        
        UDPServer* obj = UDPServer::create(arg0,arg1,arg2);
        //object_to_luaval<UDPServer>(tolua_S, ,(UDPServer*));
        tolua_pushusertype(tolua_S, (void*)obj, "KCPServer");
        return 1;
    }
    luaL_error(tolua_S, "%s has wrong number of arguments: %d, was expecting %d\n ", "pc.UDPServer:create",argc, 3);
    return 0;
#if COCOS2D_DEBUG >= 1
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'lua_register_UDPServer_create'.",&tolua_err);
#endif
    return 0;
}
int lua_register_UDPServer_setCryptoConfig(lua_State* tolua_S)
{
    int argc = 0;
    UDPServer* cobj = nullptr;
    bool ok  = true;
    
#if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
#endif
    
    
#if COCOS2D_DEBUG >= 1
    if (!tolua_isusertype(tolua_S,1,"pc.UDPServer",0,&tolua_err)) goto tolua_lerror;
#endif
    
    cobj = (UDPServer*)tolua_tousertype(tolua_S,1,0);
    
#ifndef UNCHECK_PTR
    if (!cobj)
    {
        tolua_error(tolua_S,"invalid 'cobj' in function 'lua_register_UDPServer_setCryptoConfig'", nullptr);
        return 0;
    }
#endif
    argc = lua_gettop(tolua_S)-1;
    if (argc == 2)
    {
        int arg0;
        ok &= luaval_to_int32(tolua_S,2, &arg0, "pc.UDPServer:setCryptoConfig");
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function 'lua_register_UDPServer_setCryptoConfig'", nullptr);
            return 0;
        }
        
        std::string arg1;
        ok &= luaval_to_std_string(tolua_S,3, &arg1, "pc.UDPServer:setCryptoConfig");
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function 'lua_register_UDPServer_setCryptoConfig'", nullptr);
            return 0;
        }
        cobj->setCryptoConfig(arg0, arg1);
        return 0;
    }
    luaL_error(tolua_S, "%s has wrong number of arguments: %d, was expecting %d \n", "pc.UDPServer:setCryptoConfig",argc, 2);
    return 0;
    
#if COCOS2D_DEBUG >= 1
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'lua_register_UDPServer_setCryptoConfig'.",&tolua_err);
#endif
    return 0 ;
}
int lua_register_kcp_setKCPConfig(lua_State* tolua_S)
{
    int argc = 0;
    UDPServer* cobj = nullptr;
    bool ok  = true;
    int arg1 = 0;
    int arg2 = 0;
    int arg3 = 0;
    int arg4 = 0;
    int arg5 = 0;
    int arg6 = 0;
    int arg7 = 0;
    int arg8 = 0;
#if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
#endif
#if COCOS2D_DEBUG >= 1
    if (!tolua_isusertype(tolua_S,1,"pc.UDPServer",0,&tolua_err)) goto tolua_lerror;
#endif
    cobj = (UDPServer*)tolua_tousertype(tolua_S,1,0);
    
#ifndef UNCHECK_PTR
    if (!cobj)
    {
        tolua_error(tolua_S,"invalid 'cobj' in function 'lua_register_EncryptedNumber_setKCPConfig'", nullptr);
        return 0;
    }
#endif
    argc = lua_gettop(tolua_S)-1;

    if (argc >= 1)
    {
        ok &= luaval_to_int32(tolua_S,2, &arg1, "pc.UDPServer:setKCPConfig");
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function 'lua_register_kcp_setKCPConfig'", nullptr);
            return 0;
        }
        UDPSocket* socket = cobj->getSocket();
        socket->setKCPConfig(arg1);
    }
    if (argc >= 2)
    {

        ok &= luaval_to_int32(tolua_S,3, &arg2, "pc.UDPServer:setKCPConfig");
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function 'lua_register_kcp_setKCPConfig'", nullptr);
            return 0;
        }
        UDPSocket* socket = cobj->getSocket();
        socket->setKCPConfig(arg1,arg2);
    }
    if (argc >= 3)
    {

        ok &= luaval_to_int32(tolua_S,4, &arg3, "pc.UDPServer:setKCPConfig");
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function 'lua_register_kcp_setKCPConfig'", nullptr);
            return 0;
        }
        UDPSocket* socket = cobj->getSocket();
        socket->setKCPConfig(arg1,arg2,arg3);
    }
    if (argc >= 4)
    {

        ok &= luaval_to_int32(tolua_S,5, &arg4, "pc.UDPServer:setKCPConfig");
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function 'lua_register_kcp_setKCPConfig'", nullptr);
            return 0;
        }
        UDPSocket* socket = cobj->getSocket();
        socket->setKCPConfig(arg1,arg2,arg3,arg4);
    }
    if (argc >= 5)
    {

        ok &= luaval_to_int32(tolua_S,6, &arg5, "pc.UDPServer:setKCPConfig");
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function 'lua_register_kcp_setKCPConfig'", nullptr);
            return 0;
        }
        UDPSocket* socket = cobj->getSocket();
        socket->setKCPConfig(arg1,arg2,arg3,arg4,arg5);
    }
    if (argc >= 6)
    {

        ok &= luaval_to_int32(tolua_S,7, &arg6, "pc.UDPServer:setKCPConfig");
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function 'lua_register_kcp_setKCPConfig'", nullptr);
            return 0;
        }
        UDPSocket* socket = cobj->getSocket();
        socket->setKCPConfig(arg1,arg2,arg3,arg4,arg5,arg6);
    }
    if (argc >= 7)
    {

        ok &= luaval_to_int32(tolua_S,8, &arg7, "pc.UDPServer:setKCPConfig");
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function 'lua_register_kcp_setKCPConfig'", nullptr);
            return 0;
        }
        UDPSocket* socket = cobj->getSocket();
        socket->setKCPConfig(arg1,arg2,arg3,arg4,arg5,arg6,arg7);
    }
    if (argc == 8)
    {

        ok &= luaval_to_int32(tolua_S,9, &arg8, "pc.UDPServer:setKCPConfig");
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function 'lua_register_kcp_setKCPConfig'", nullptr);
            return 0;
        }
        UDPSocket* socket = cobj->getSocket();
        socket->setKCPConfig(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8);
    }
    if (argc > 8)
    {
        tolua_error(tolua_S,"invalid arguments in function 'lua_register_kcp_setKCPConfig'", nullptr);
    }

    return 0 ;

#if COCOS2D_DEBUG >= 1
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'lua_register_UDPServer_notify'.",&tolua_err);
#endif
    return 0 ;
}

int lua_register_UDPServer_notify(lua_State* tolua_S)
{
    int argc = 0;
    UDPServer* cobj = nullptr;
    bool ok  = true;
    
#if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
#endif
    
    
#if COCOS2D_DEBUG >= 1
    if (!tolua_isusertype(tolua_S,1,"pc.UDPServer",0,&tolua_err)) goto tolua_lerror;
#endif
    
    cobj = (UDPServer*)tolua_tousertype(tolua_S,1,0);
    
#ifndef UNCHECK_PTR
    if (!cobj)
    {
        tolua_error(tolua_S,"invalid 'cobj' in function 'lua_register_UDPServer_notify'", nullptr);
        return 0;
    }
#endif
    
    argc = lua_gettop(tolua_S)-1;
    if (argc == 3)
    {
        int arg0;
        ok &= luaval_to_int32(tolua_S,2, &arg0, "pc.UDPServer:notify");
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function 'lua_register_UDPServer_notify'", nullptr);
            return 0;
        }
        
        std::string arg1;
        ok &= luaval_to_std_string(tolua_S,3, &arg1, "pc.UDPServer:notify");
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function 'lua_register_UDPServer_notify'", nullptr);
            return 0;
        }
        std::string arg2;
        ok &= luaval_to_std_string(tolua_S,4, &arg2, "pc.UDPServer:notify");
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function 'lua_register_UDPServer_notify'", nullptr);
            return 0;
        }
        cobj->sendUDPData(arg0, arg1,arg2);
        return 0;
    }
    luaL_error(tolua_S, "%s has wrong number of arguments: %d, was expecting %d \n", "pc.UDPServer:notify",argc, 3);
    return 0;
    
#if COCOS2D_DEBUG >= 1
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'lua_register_UDPServer_notify'.",&tolua_err);
#endif
    return 0 ;
}

int lua_register_kcp_setLuaCallBack(lua_State* tolua_S)
{
    int argc = 0;
    UDPServer* cobj = nullptr;
    bool ok  = true;
    
#if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
#endif
    
    
#if COCOS2D_DEBUG >= 1
    if (!tolua_isusertype(tolua_S,1,"pc.UDPServer",0,&tolua_err)) goto tolua_lerror;
#endif
    
    cobj = (UDPServer*)tolua_tousertype(tolua_S,1,0);
    
#ifndef UNCHECK_PTR
    if (!cobj)
    {
        tolua_error(tolua_S,"invalid 'cobj' in function 'lua_register_EncryptedNumber_setLuaCallBack'", nullptr);
        return 0;
    }
#endif
    
    argc = lua_gettop(tolua_S)-1;
    if (argc == 1)
    {
#if COCOS2D_DEBUG >= 1
        if (!toluafix_isfunction(tolua_S,2,"LUA_FUNCTION",0,&tolua_err) )
        {
            goto tolua_lerror;
        }
#endif
        int handler = (  toluafix_ref_function(tolua_S,2,0));
        cobj->setUDPCallBack([=](int err,int method,std::string data,float timestamp){
            kcpPerformInLuaThread([=](){
                cocos2d::LuaStack* luaStack = cocos2d::LuaEngine::getInstance()->getLuaStack();
                luaStack->pushInt(err);
                luaStack->pushInt(method);
                luaStack->pushString(data.c_str(),data.length());
                luaStack->pushFloat(timestamp);
                luaStack->executeFunctionByHandler(handler, 4);
            });
        });
        return 0;
    }
    luaL_error(tolua_S, "%s has wrong number of arguments: %d, was expecting %d \n", "pc.UDPServer:setLuaCallBack",argc, 2);
    return 0;
#if COCOS2D_DEBUG >= 1
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'lua_register_EncryptedNumber_setLuaCallBack'.",&tolua_err);
#endif
    return 0 ;
}
int lua_register_kcp_destroyUDPServer(lua_State* tolua_S)
{
    int argc = 0;
    bool ok  = true;
    
#if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
#endif
    
#if COCOS2D_DEBUG >= 1
    if (!tolua_isusertable(tolua_S,1,"pc.UDPServer",0,&tolua_err)) goto tolua_lerror;
#endif
    
    argc = lua_gettop(tolua_S) - 1;
    
    if (argc == 1)
    {

        UDPServer* obj;
        ok &= luaval_to_object<UDPServer>(tolua_S, 2, "pc.UDPServer", &obj,"pc.UDPServer:destroyUDPServer");
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function 'lua_register_kcp_destroyUDPServer'", nullptr);
            return 0;
        }
        if(obj)
        {
            //ScriptHandlerMgr::getInstance()->removeObjectAllHandlers((void*)obj);
            UDPServer::destroyUDPServer(obj);
            return 0;
        }
    }
    luaL_error(tolua_S, "%s has wrong number of arguments: %d, was expecting %d\n ", "pc.UDPServer:destroyUDPServer",argc, 0);
    return 0;
#if COCOS2D_DEBUG >= 1
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'lua_register_kcp_destroyUDPServer'.",&tolua_err);
#endif
    return 0;
}
int lua_register_kcp_reconnect(lua_State* tolua_S)
{
    UDPServer* cobj = nullptr;
    bool ret  = true;
    
#if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
#endif
    
    
#if COCOS2D_DEBUG >= 1
    if (!tolua_isusertype(tolua_S,1,"pc.UDPServer",0,&tolua_err)) goto tolua_lerror;
#endif
    
    cobj = (UDPServer*)tolua_tousertype(tolua_S,1,0);
    
#ifndef UNCHECK_PTR
    if (!cobj)
    {
        tolua_error(tolua_S,"invalid 'cobj' in function 'lua_register_kcp_reconnect'", nullptr);
        return 0;
    }
#endif
    ret = cobj->reconnect();
    lua_pushboolean(tolua_S,ret);
    return 1;
#if COCOS2D_DEBUG >= 1
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'lua_register_kcp_reconnect'.",&tolua_err);
#endif
    return 0;
}

int lua_register_kcp_resetSocket(lua_State* tolua_S)
{
    UDPServer* cobj = nullptr;
    bool ret  = true;
    
#if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
#endif
    
    
#if COCOS2D_DEBUG >= 1
    if (!tolua_isusertype(tolua_S,1,"pc.UDPServer",0,&tolua_err)) goto tolua_lerror;
#endif
    
    cobj = (UDPServer*)tolua_tousertype(tolua_S,1,0);
    
#ifndef UNCHECK_PTR
    if (!cobj)
    {
        tolua_error(tolua_S,"invalid 'cobj' in function 'lua_register_kcp_reconnect'", nullptr);
        return 0;
    }
#endif
    ret = cobj->resetSocket();
    lua_pushboolean(tolua_S,ret);
    return 1;
#if COCOS2D_DEBUG >= 1
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'lua_register_kcp_reconnect'.",&tolua_err);
#endif
    return 0;
}
static int lua_register_kcp_tickcount(lua_State *L) {
    lua_pushnumber(L, pigeonTickCount());
    return 1; /* number of results */
}

extern "C" {

/* ===== OPEN MODULE ===== */
int luaopen_kcp_udp(lua_State* l)
{
    tolua_open(l);
    tolua_usertype(l, "UDPServer");
    tolua_usertype(l, "KCPServer");
    tolua_module(l, NULL,0);
    tolua_beginmodule(l, NULL);
        tolua_cclass(l,"UDPServer","KCPServer","",NULL);
        tolua_beginmodule(l, "UDPServer");
            tolua_function(l,"create", lua_register_UDPServer_create);
            tolua_function(l,"setCryptoConfig", lua_register_UDPServer_setCryptoConfig);
            tolua_function(l,"notify", lua_register_UDPServer_notify);
            tolua_function(l,"setLuaCallBack", lua_register_kcp_setLuaCallBack);
            tolua_function(l,"setKCPConfig", lua_register_kcp_setKCPConfig);
            tolua_function(l,"destroyUDPServer", lua_register_kcp_destroyUDPServer);
            tolua_function(l,"reconnect", lua_register_kcp_reconnect);
            tolua_function(l,"resetSocket", lua_register_kcp_resetSocket);
            tolua_function(l,"tickcount", lua_register_kcp_tickcount);
        tolua_endmodule(l);
    tolua_endmodule(l);

    return 1;
}
}