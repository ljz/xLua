#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "lua.h"
#include "lauxlib.h"

#if defined(WIN32)
#define snprintf _snprintf
#endif
    
    /* convert a stack index to positive */
#define abs_index(L, i) ((i)>0 || (i)<=LUA_REGISTRYINDEX ? (i) : lua_gettop(L)+(i)+1)


#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502
    /* Compatibility for Lua 5.1.
     *
     * luaL_setfuncs() is used to create a module table where the functions have
     * pigeon_config_t as their first upvalue. Code borrowed from Lua 5.2 source. */
    static void luaL_setfuncs(lua_State* l, const luaL_Reg* reg, int nup)
    {
        int i;
        luaL_checkstack(l, nup, "too many upvalues");
        for (; reg->name != NULL; reg++) {  /* fill the table with given functions */
            for (i = 0; i < nup; i++)  /* copy upvalues to the top */
                lua_pushvalue(l, -nup);
            lua_pushcclosure(l, reg->func, nup);  /* closure with those upvalues */
            lua_setfield(l, -(nup + 2), reg->name);
        }
        lua_pop(l, nup);  /* remove upvalues */
    }
#endif
    
#define LUA_ARGCHECK(L, narg, type) checkargtype(L, lua_is##type(L, narg), narg, #type)
    
    static int checkargtype(lua_State* L, int cond, int narg, const char* expected)
    {
        if (!cond) {
            char extramsg[128];
#if defined(WIN32)
			extramsg[_snprintf(
				extramsg,
				sizeof(extramsg)-1,
				"%s expected, got %s",
				expected,
				lua_typename(L, lua_type(L, narg)))] = 0;
#else
            extramsg[snprintf(
                              extramsg,
                              sizeof(extramsg)-1,
                              "%s expected, got %s",
                              expected,
                              lua_typename(L, lua_type(L, narg)))] = 0;
#endif
            luaL_argerror(L, narg, extramsg);
        }
        return cond;
    }
    
#ifdef __cplusplus
}
#endif

#include "pigeon/pigeon.h"
#include "lua_pigeon.h"
#include "LuaEngine.h"
#ifndef PIGEON_MODNAME
#define PIGEON_MODNAME   "pigeon"
//#define PIGEON_MODNAME   "pcnet"
#endif

static int luaCall(lua_State* l, int nargs, int nresults) {
    int traceback = 0;
    int functionIndex = -(nargs + 1);
    if (!lua_isfunction(l, functionIndex)) {
        NLOG(NL_DEBUG, "[LUA ERROR] [%s:%d]: Trying to call a non-function.", __FILE__, __LINE__);
        return -1;
    }
    lua_getglobal(l, "__G__TRACKBACK__");                         /* L: ... func arg1 arg2 ... G */
    if (!lua_isfunction(l, -1)) {
        lua_pop(l, 1);                                            /* L: ... func arg1 arg2 ... */
    } else {
        lua_insert(l, functionIndex - 1);                         /* L: ... G func arg1 arg2 ... */
        traceback = functionIndex - 1;
    }
    int error = lua_pcall(l, nargs, nresults, traceback);         /* L: ... [G] {rets} */
    if (error) {
        const char* errmsg = lua_tostring(l, -1);
        NLOG(NL_DEBUG, "[LUA ERROR] [%s:%d]:\n%s", __FILE__, __LINE__, errmsg);
        lua_pop(l, (traceback==0? 1: 2));
    } else {
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

struct int_constant_t {
    const char* name; int value;
};

static int register_int_constants(lua_State* l, int t, const struct int_constant_t* constants)
{
    if (constants) {
        int tidx = abs_index(l, t);
        for (int i=0; ; i++) {
            const struct int_constant_t* e = constants+i;
            if (!e->name) { break; }
            lua_pushinteger(l, e->value);
            lua_setfield(l, tidx, e->name);
        }
    }
    return 0;
}

typedef struct pigeon_activity_t_ pigeon_activity_t;

struct pigion_activity_lnode {
    pigeon_activity_t* obj;
    struct pigion_activity_lnode* next;
};

typedef struct {
    NetController* net;
    pigion_activity_lnode* activities;
} pigeon_config_t;

typedef struct pigeon_activity_t_ {
    pigeon_config_t* cfg;
    RPCActivity* activity;
} pigeon_activity_t;

static void cleanup_pigeon_activity(pigeon_activity_t_* obj)
{
    if (!obj || !obj->cfg) {
        return;
    }
    obj->cfg->net->destroyActivity(obj->activity);
    obj->cfg = NULL;
    obj->activity = NULL;
}

static pigeon_config_t* init_pigeon_config(pigeon_config_t* cfg)
{
    if (!cfg) {
        return NULL;
    }
    cfg->net = new NetController();
    cfg->activities = NULL;
    return cfg;
}

static void dispose_pigeon_config(pigeon_config_t* cfg)
{
    if (!cfg) {
        return;
    }
    pigion_activity_lnode* node = cfg->activities;
    while (node) {
        cfg->activities = node->next;
        cleanup_pigeon_activity(node->obj);
        ::free(node);
        node = cfg->activities;
    }
    delete cfg->net;
    cfg->net = NULL;
}

static int pigeon_add_activity(pigeon_config_t* cfg, pigeon_activity_t* activity)
{
    if (!cfg || !activity) {
        return 0;
    }
    pigion_activity_lnode* node = (pigion_activity_lnode*)::malloc(sizeof(pigion_activity_lnode));
    node->obj= activity;
    node->next = cfg->activities;
    cfg->activities = node;
    return 1;
}

static int pigeon_remove_activity(pigeon_config_t* cfg, pigeon_activity_t* activity)
{
    if (!cfg || !activity) {
        return 0;
    }
    pigion_activity_lnode** ptr = &cfg->activities;
    pigion_activity_lnode* node = cfg->activities;
    while (node) {
        if (node->obj==activity) {
            *ptr = node->next;
            cleanup_pigeon_activity(node->obj);
            ::free(node);
            return 1;
        }
        ptr = &node->next;
        node = node->next;
    }
    return 0;
}

/* ===== Net register table  ===== */
static int get_pigeon_reftable(lua_State* l, void* cfg, bool autocreate)
{
    lua_pushlightuserdata(l, cfg);
    lua_gettable(l, LUA_REGISTRYINDEX);
    if (lua_isnil(l, -1) && autocreate) {
        lua_pop(l, 1);
        lua_newtable(l);
        lua_pushlightuserdata(l, cfg);
        lua_pushvalue(l, -2);
        lua_settable(l, LUA_REGISTRYINDEX);
    }
    return 1;
}

static int remove_pigeon_reftable(lua_State* l, void* cfg)
{
    lua_pushlightuserdata(l, cfg);
    lua_gettable(l, LUA_REGISTRYINDEX);
    if (lua_isnil(l, -1)) {
        return 0;
    }
    lua_pushlightuserdata(l, cfg);
    lua_pushnil(l);
    lua_settable(l, LUA_REGISTRYINDEX);
    return 0;
}

static int get_activity_reftable(lua_State* l, void* cfg, void* activity, bool autocreate)
{
    get_pigeon_reftable(l, cfg, autocreate);
    if (lua_isnil(l, -1)) {
        return 1;
    }
    lua_pushlightuserdata(l, activity);
    lua_gettable(l, -2);
    if (lua_isnil(l, -1) && autocreate) {
        lua_pop(l, 1);
        lua_newtable(l);
        lua_pushlightuserdata(l, activity);
        lua_pushvalue(l, -2);
        lua_settable(l, -4); // cfg[activity] = new_table
    }
    lua_remove(l, -2);
    return 1;
}

static int remove_activity_reftable(lua_State* l, void* cfg, void* activity)
{
    get_pigeon_reftable(l, cfg, false);
    if (lua_isnil(l, -1)) {
        lua_pop(l, 1);
        return 0;
    }
    lua_pushlightuserdata(l, activity);
    lua_gettable(l, -2);
    if (!lua_isnil(l, -1)) {
        lua_pushlightuserdata(l, activity);
        lua_pushnil(l);
        lua_settable(l, -4);
    }
    lua_pop(l, 2);
    return 0;
}

/* ===== RPCActivity  ===== */

static pigeon_activity_t* fetch_pigeon_activity(lua_State* l)
{
    pigeon_activity_t* activity;
    activity = (pigeon_activity_t*)lua_touserdata(l, lua_upvalueindex(1));
    if (!activity) {
        luaL_error(l, "BUG: Unable to fetch pigeon activity");
    }
    return activity;
}

static int destroy_activity(lua_State* l)
{
    pigeon_activity_t* client;
    client = (pigeon_activity_t*)lua_touserdata(l, 1);
    if (client->cfg) {
        remove_activity_reftable(l, client->cfg, client->activity);
        pigeon_remove_activity(client->cfg, client);
    }
    client = NULL;
    return 0;
}

static pigeon_activity_t* create_activity(lua_State* l, pigeon_config_t* cfg, int cfgidx)
{
    pigeon_activity_t* activity;
    activity = (pigeon_activity_t*)lua_newuserdata(l, sizeof(*activity));
    
    activity->cfg = cfg;
    activity->activity = NULL;
    pigeon_add_activity(cfg, activity);
    
    // Create metatable for the activity user data
    lua_newtable(l);
    // Prevent `cfg` being freed by the GC
    lua_pushvalue(l, cfgidx);
    lua_setfield(l, -2, "pigeon");
    // Register GC method
    lua_pushcfunction(l, destroy_activity);
    lua_setfield(l, -2, "__gc");
    lua_setmetatable(l, -2);
    
    return activity;
}

/* ===== RPCClient  ===== */

static int rpcclient_connect(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    if (!LUA_ARGCHECK(l, 1, string) ||
        !LUA_ARGCHECK(l, 2, number)) {
        return 0;
    }
    const char* host = lua_tostring(l, 1);
    in_port_t port = lua_tointeger(l, 2);
    long timeout = 0;
    if (lua_gettop(l)>=3 && !lua_isnil(l, 3)) {
        if (!LUA_ARGCHECK(l, 3, number)) {
            return 0;
        }
        timeout = lua_tointeger(l, 3);
    }
    bool result = ((RPCClient*)client->activity)->connect(host, port, timeout);
    lua_pushboolean(l, result);
    return 1;
}

static int rpcclient_repair(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    bool force = false;
    if (lua_gettop(l)>=1 && !lua_isnil(l, 1)) {
        if (!LUA_ARGCHECK(l, 1, boolean)) {
            return 0;
        }
        force = lua_toboolean(l, 1);
    }
    bool result = ((RPCClient*)client->activity)->repair(force);
    lua_pushboolean(l, result);
    return 1;
}

static int rpcclient_close(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    ((RPCClient*)client->activity)->close();
    return 0;
}

static int rpcclient_request(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    if (!LUA_ARGCHECK(l, 1, number) ||
        //!LUA_ARGCHECK(l, 2, number) ||
        !LUA_ARGCHECK(l, 2, string) ||
        !LUA_ARGCHECK(l, 3, function)) {
        return 0;
    }
    int method = (int)lua_tointeger(l, 1);
    //int compr = (int)lua_tointeger(l, 2);
    size_t datalen = 0;
    const char* data = lua_tolstring(l, 2, &datalen);
    // lua_tocfunction(l, 3);
    uint16_t extraRTO = 0;
    if (lua_gettop(l)>=4 && !lua_isnil(l, 4)) {
        if (!LUA_ARGCHECK(l, 4, number)) {
            return 0;
        }
        long a5 = lua_tointeger(l, 4);
        extraRTO = a5>0xffff ? 0xffff : (a5>0? (uint16_t)a5: 0);
    }
    void* refkey = client->cfg;
    get_pigeon_reftable(l, refkey, true);
    lua_pushvalue(l, 3);
    int cbref = luaL_ref(l, -2);
    int result = ((RPCClient*)client->activity)->request(
                                                         method, data, datalen,
                                                         [=](int req, int err, const std::string& resp,float timestamp) {
                                                             pigeonPerformInLuaThread([=]() {
                                                                 get_pigeon_reftable(l, refkey, false);
                                                                 if (lua_isnil(l, -1)) { return; }
                                                                 lua_pushinteger(l, cbref);
                                                                 lua_gettable(l, -2);
                                                                 lua_pushinteger(l, req);
                                                                 lua_pushinteger(l, err);
                                                                 lua_pushlstring(l, resp.c_str(), resp.size());
                                                                 lua_pushnumber(l, timestamp);
                                                                 luaCall(l, 4, 0);
                                                                 luaL_unref(l, -1, cbref);
                                                             });
                                                         }, extraRTO);
    lua_pushinteger(l, result);
    return 1;
}

static int rpcclient_rerequest(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    if (!LUA_ARGCHECK(l, 1, number)) {
        return 0;
    }
    int request = (int)lua_tointeger(l, 1);
    int result = ((RPCClient*)client->activity)->request(request);
    lua_pushinteger(l, result);
    return 1;
}

static int rpcclient_notify(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    if (!LUA_ARGCHECK(l, 1, number) ||
       // !LUA_ARGCHECK(l, 2, number) ||
        !LUA_ARGCHECK(l, 2, string)) {
        return 0;
    }
    int event = (int)lua_tointeger(l, 1);
    //int compr = (int)lua_tointeger(l, 2);
    size_t datalen = 0;
    const char* data = lua_tolstring(l, 2, &datalen);
    int result = ((RPCClient*)client->activity)->notify(event, /*compr,*/ data, datalen);
    lua_pushinteger(l, result);
    return 1;
}

static int rpcclient_respond(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    if (!LUA_ARGCHECK(l, 1, number) ||
        !LUA_ARGCHECK(l, 2, number) ||
        !LUA_ARGCHECK(l, 3, number) ||
        !LUA_ARGCHECK(l, 4, string)) {
        return 0;
    }
    int request = (int)lua_tointeger(l, 1);
    int compr = (int)lua_tointeger(l, 2);
    int err = (int)lua_tointeger(l, 3);
    size_t datalen = 0;
    const char* data = lua_tolstring(l, 4, &datalen);
    int result = ((RPCClient*)client->activity)->respond(request, /*compr,*/ err, data, datalen);
    lua_pushinteger(l, result);
    return 1;
}

static int rpcclient_isvalid(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        lua_pushboolean(l, false);
    } else {
        lua_pushboolean(l, true);
    }
    return 1;
}

static int rpcclient_setHeartbeatInterval(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    if (!LUA_ARGCHECK(l, 1, number)) {
        return 0;
    }
    long value = lua_tointeger(l, 1);
    ((RPCClient*)client->activity)->setHeartbeatInterval(value);
    return 0;
}

static int rpcclient_getHeartbeatInterval(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    long value = ((RPCClient*)client->activity)->getHeartbeatInterval();
    lua_pushnumber(l, value);
    return 1;
}

static int rpcclient_setHeartbeatTimeout(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    if (!LUA_ARGCHECK(l, 1, number)) {
        return 0;
    }
    long value = lua_tointeger(l, 1);
    ((RPCClient*)client->activity)->setHeartbeatTimeout(value);
    return 0;
}

static int rpcclient_getHeartbeatTimeout(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    long value = ((RPCClient*)client->activity)->getHeartbeatTimeout();
    lua_pushnumber(l, value);
    return 1;
}

static int rpcclient_setLoopInterval(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    if (!LUA_ARGCHECK(l, 1, number)) {
        return 0;
    }
    int value = (int)lua_tointeger(l, 1);
    ((RPCClient*)client->activity)->setloopInterval(value);
    return 0;
}

static int rpcclient_setReqRTOBounds(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    if (!LUA_ARGCHECK(l, 1, number) ||
        !LUA_ARGCHECK(l, 2, number)) {
        return 0;
    }
    int lvalue = (int)lua_tointeger(l, 1);
    int uvalue = (int)lua_tointeger(l, 2);
    ((RPCClient*)client->activity)->setReqRTOBounds(lvalue, uvalue);
    return 0;
}

static int rpcclient_setReqTimeout(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    if (!LUA_ARGCHECK(l, 1, number)) {
        return 0;
    }
    int value = (int)lua_tointeger(l, 1);
    ((RPCClient*)client->activity)->setReqTimeout(value);
    return 0;
}

static int rpcclient_getReqTimeout(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    int value = ((RPCClient*)client->activity)->getReqTimeout();
    lua_pushnumber(l, value);
    return 1;
}

static int rpcclient_getMaxReqDelay(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    long value = ((RPCClient*)client->activity)->getMaxReqDelay();
    lua_pushnumber(l, value);
    return 1;
}

static int rpcclient_getAvgReqDelay(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    long value = ((RPCClient*)client->activity)->getAvgReqDelay();
    lua_pushnumber(l, value);
    return 1;
}

static int rpcclient_remoteTimestamp(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    long elb = 0, eub = 0;
    long value = ((RPCClient*)client->activity)->remoteTimestamp(&elb, &eub);
    lua_pushnumber(l, value);
    lua_pushnumber(l, elb);
    lua_pushnumber(l, eub);
    return 3;
}

static int rpcclient_remoteTimeMillis(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    long elb = 0, eub = 0;
    int64_t value = ((RPCClient*)client->activity)->remoteTimeMillis(&elb, &eub);
    lua_pushnumber(l, value);
    lua_pushnumber(l, elb);
    lua_pushnumber(l, eub);
    return 3;
}

static int rpcclient_remoteTimescale(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    double lb = 0, ub = 0;
    double value = ((RPCClient*)client->activity)->remoteTimescale(&lb, &ub);
    lua_pushnumber(l, value);
    lua_pushnumber(l, lb);
    lua_pushnumber(l, ub);
    return 3;
}
static int rpcclient_timeCorrectorReset(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    ((RPCClient*)client->activity)->timeCorrectorReset();
    return 0;
}
static int rpcclient_getCryptoConfig(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    std::string config = ((RPCClient*)client->activity)->getCryptoConfig();
//    lua_pushstring(l, config.c_str());
    lua_pushlstring(l, config.c_str(), config.length());
    return 1;
}
static int rpcclient_getCryptoType(lua_State* l)
{
    pigeon_activity_t* client = fetch_pigeon_activity(l);
    if (!client || !client->activity) {
        return 0;
    }
    int type = ((RPCClient*)client->activity)->getCryptoType();
    lua_pushnumber(l, type);
    return 1;
}
/* ===== NetController ===== */

static pigeon_config_t* fetch_pigeon_config(lua_State* l)
{
    pigeon_config_t* cfg;
    cfg = (pigeon_config_t*)lua_touserdata(l, lua_upvalueindex(1));
    if (!cfg) {
        luaL_error(l, "BUG: Unable to fetch pigeon configuration");
    }
    return cfg;
}

static int pigeon_create_client(lua_State* l)
{
    pigeon_config_t* cfg;
    if (NULL==(cfg = fetch_pigeon_config(l))) {
        return 0;
    }
    
    // pigeon client table
    lua_newtable(l);

    pigeon_activity_t* client;
    client = create_activity(l, cfg, lua_upvalueindex(1));
    client->activity = cfg->net->createClient();
    
    luaL_Reg reg[] = {
        { "connect", rpcclient_connect },
        { "repair", rpcclient_repair },
        { "close", rpcclient_close },
        { "request", rpcclient_request },
        { "rerequest", rpcclient_rerequest },
        { "notify", rpcclient_notify },
        { "respond", rpcclient_respond },
        { "isvalid", rpcclient_isvalid },
        { "setHeartbeatInterval", rpcclient_setHeartbeatInterval },
        { "getHeartbeatInterval", rpcclient_getHeartbeatInterval },
        { "setHeartbeatTimeout", rpcclient_setHeartbeatTimeout },
        { "getHeartbeatTimeout", rpcclient_getHeartbeatTimeout },
        { "setReqRTOBounds", rpcclient_setReqRTOBounds },
        { "setReqTimeout", rpcclient_setReqTimeout },
        { "getReqTimeout", rpcclient_getReqTimeout },
        { "getMaxReqDelay", rpcclient_getMaxReqDelay },
        { "getAvgReqDelay", rpcclient_getAvgReqDelay },
        { "remoteTimestamp", rpcclient_remoteTimestamp },
        { "remoteTimeMillis", rpcclient_remoteTimeMillis },
        { "remoteTimescale", rpcclient_remoteTimescale },
        {"setLoopInterval",rpcclient_setLoopInterval},
        {"timeCorrectorReset",rpcclient_timeCorrectorReset},
        {"getCryptoConfig",rpcclient_getCryptoConfig},
        {"getCryptoType",rpcclient_getCryptoType},
        { NULL, NULL }
    };
    luaL_setfuncs(l, reg, 1);
    
    struct int_constant_t enums[] = {
        { "SERVER_CLOSED", Error_ServerClosed },
        { "CONNECT_FAILED", Error_ConnectFailed },
        { "CONNECT_BROKEN", Error_ConnectBroken },
        { "SESSION_ERROR", Error_SessionError},
//        { "CONNECT_RESET", Error_ConnectReset },
//        { "INACTIVE", Error_Inactive },
        { "REQ_TIMEDOUT", Error_ReqTimedout },
        { "FATAL_ERROR", Error_FatalError },
        
        { "ServerClosedDetail", ErrorTypes_ServerClosed },
        { "ConnectFailedDetail", ErrorTypes_ConnectFailed },
        { "ConnectBrokenDetail", ErrorTypes_ConnectBroken },
        { "WriteErrorDetail", ErrorTypes_WriteError },
        { "ReadErrorDetail", ErrorTypes_ReadError },
        { "CRCCheckFailedDetail", ErrorTypes_CRCCheckFailed },
        { "SessionErrorDetail", ErrorTypes_SessionError },
        { "TimeOutDetail", ErrorTypes_TimeOut },
        { "InactiveDetail", ErrorTypes_Inactive },
        { "EstablishErrorDetail", ErrorTypes_EstablishError },
        
        { "ERR_NO_ERROR", eNetNoError },
        { "ERR_INVALID_OP", eNetInvalidOp },
        { "ERR_UNCOMPR_ERROR", eNetUncomprError },
        { "ERR_DECODE_ERROR", eNetDecodeError },
        { "ERR_UNKOWN_ERROR", eNetUnkownError },

        { NULL, 0 },
    };
    register_int_constants(l, -1, enums);
    
    get_activity_reftable(l, client->cfg, client->activity, true);
    lua_pushvalue(l, -2);
    lua_setfield(l, -2, "self"); // reftable["self"] = client_table
    lua_pop(l, 1);
    
    class LuaClientListener: public RPCClient::Listener
    {
    private:
        lua_State *_luaState;
        void *_cfg, *_activity;
    public:
        LuaClientListener(lua_State* l, void* cfg, void* activity) {
            _luaState = l;
            _cfg = cfg; _activity = activity;
        }
        virtual ~LuaClientListener() {}
        virtual void onBinded(RPCClient* client) {}
        virtual void onUnbinded(RPCClient* client) { delete this; }
        
        bool getListenerMethod(lua_State* l, const char* methodName) {
            get_activity_reftable(l, _cfg, _activity, false);
            if (!lua_istable(l, -1)) { lua_pop(l, 1); return false; }
            lua_getfield(l, -1, "self");
            lua_remove(l, -2);
            if (!lua_istable(l, -1)) { lua_pop(l, 1); return false; }
            lua_getfield(l, -1, "listener");
            if (lua_isfunction(l, -1)) {
                lua_insert(l, -2);
                lua_pushstring(l, methodName);
                lua_insert(l, -2);  /* L: listener, methodName, client_self */
                return true;
            } else if (lua_istable(l, -1)) {
                lua_getfield(l, -1, methodName);
                if (!lua_isfunction(l, -1)) { lua_pop(l, 3); return false; }
                lua_insert(l, -3);
                lua_insert(l, -2);  /* L: method, listener, client_self */
                return true;
            } else {
                return false;
            }
        }
        
        virtual void onError(RPCClient* client, int errcode, int detail) {
            pigeonPerformInLuaThread([=]() {
                if (getListenerMethod(_luaState, "onError")) {
                    lua_pushinteger(_luaState, errcode);
                    lua_pushinteger(_luaState, detail);
                    luaCall(_luaState, 4, 0);
                }
            });
        }
        virtual void onConnected(RPCClient* client, bool firstConnect) {
            pigeonPerformInLuaThread([=]() {
                if (getListenerMethod(_luaState, "onConnected")) {
                    lua_pushboolean(_luaState, firstConnect);
                    luaCall(_luaState, 3, 0);
                }
            });
        }
        virtual void onRequested(RPCClient* client, int request, int method, const std::string& params) {
            pigeonPerformInLuaThread([=]() {
                if (getListenerMethod(_luaState, "onRequested")) {
                    lua_pushinteger(_luaState, request);
                    lua_pushinteger(_luaState, method);
                    lua_pushlstring(_luaState, params.c_str(), params.size());
                    luaCall(_luaState, 5, 0);
                }
            });
        }
        virtual void onNotified(RPCClient* client, int event, const std::string& data,float timestamp) {
            pigeonPerformInLuaThread([=]() {
                if (getListenerMethod(_luaState, "onNotified")) {
                    lua_pushinteger(_luaState, event);
                    lua_pushlstring(_luaState, data.c_str(), data.size());
                    lua_pushnumber(_luaState, timestamp);
                    luaCall(_luaState, 5, 0);
                }
            });
        }
    };
    
    ((RPCClient*)client->activity)->setListener(new LuaClientListener(l, client->cfg, client->activity));
    
    return 1;
}

static int pigeon_update(lua_State* l)
{
	LuaEngine::getInstance()->update();
    return 0;
}

static int pigeon_removeAllFunctions(lua_State* l)
{
	LuaEngine::getInstance()->removeAllFunctions();
	return 0;
}

static int pigeon_destroy_config(lua_State* l)
{
    pigeon_config_t* cfg;
    cfg = (pigeon_config_t*)lua_touserdata(l, 1);
    remove_pigeon_reftable(l, cfg);
    dispose_pigeon_config(cfg);
    cfg = NULL;
    return 0;
}

static void pigeon_create_config(lua_State* l)
{
    pigeon_config_t *cfg;

    cfg = (pigeon_config_t*)lua_newuserdata(l, sizeof(*cfg));
    
    // Initialize cfg object.
    init_pigeon_config(cfg);
    
    /* Create GC method */
    lua_newtable(l);
    lua_pushcfunction(l, pigeon_destroy_config);
    lua_setfield(l, -2, "__gc");
    lua_setmetatable(l, -2);
}

/* Return pigeon module table */
static int lua_pigeon_new(lua_State* l)
{
    luaL_Reg reg[] = {
        { "createClient", pigeon_create_client },
        { "update", pigeon_update },
		{ "removeAllFunctions", pigeon_removeAllFunctions },
        { "new", lua_pigeon_new },
        { NULL, NULL }
    };
    
    /* pigeon module table */
    lua_newtable(l);
    
    /* Register functions with config data as upvalue */
    pigeon_create_config(l);
    luaL_setfuncs(l, reg, 1);
    
    struct int_constant_t enums[] = {
        { "COMPRESS_NONE", RPCTransport::kNonCompressed },
        { "COMPRESS_SNAPPY", RPCTransport::kComprSnappy },
        { NULL, 0 },
    };
    register_int_constants(l, -1, enums);
    
    /* Set module name / version fields */
    lua_pushliteral(l, PIGEON_MODNAME);
    lua_setfield(l, -2, "_NAME");
    // lua_pushliteral(l, PIGEON_VERSION);
    // lua_setfield(l, -2, "_VERSION");
    
    return 1;
}

const char *luaL_findtable(lua_State *L, int idx,
    const char *fname, int szhint) {
    const char *e;
    lua_pushvalue(L, idx);
    do {
        e = strchr(fname, '.');
        if (e == NULL) e = fname + strlen(fname);
        lua_pushlstring(L, fname, e - fname);
        lua_rawget(L, -2);
        if (lua_isnil(L, -1)) {  /* no such field? */
            lua_pop(L, 1);  /* remove this nil */
            lua_createtable(L, 0, (*e == '.' ? 1 : szhint)); /* new table for field */
            lua_pushlstring(L, fname, e - fname);
            lua_pushvalue(L, -2);
            lua_settable(L, -4);  /* set new table into field */
        }
        else if (!lua_istable(L, -1)) {  /* field has a non-table value? */
            lua_pop(L, 2);  /* remove table and value */
            return fname;  /* return problematic part of the name */
        }
        lua_remove(L, -2);  /* remove previous table */
        fname = e + 1;
    } while (*e == '.');
    return NULL;
}

extern "C" {

/* ===== OPEN MODULE ===== */
int luaopen_tcp(lua_State* l)
{
    lua_pigeon_new(l);
    
#if ENABLE_PIGEON_GLOBAL
    /* Register a global "pigeon" table. */
    lua_pushvalue(l, -1);
    lua_setglobal(l, PIGEON_MODNAME);
#endif
    
    luaL_findtable(l, LUA_REGISTRYINDEX, "_LOADED", 1);
    lua_pushvalue(l, -2);
    lua_setfield(l, -2, PIGEON_MODNAME);  /* _LOADED[PIGEON_MODNAME] = pigeon */
    lua_pop(l, 1);
    
    /* Return pigeon table */
    return 1;
}
}