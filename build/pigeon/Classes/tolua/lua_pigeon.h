//
//  pigeon.h
//  pigeon
//
//  Created by gamebase on 20/8/22.
//  Copyright © 2016年 gamebase. All rights reserved.
//
#ifndef __lua_pigeon_h
#define __lua_pigeon_h

#define ENABLE_PIGEON_GLOBAL 1

// Perform the function in the Lua thread.
// Anyone who uses this module MUST implement this function. Otherwise he will get a link error.
extern void pigeonPerformInLuaThread(const std::function<void()> &function);

//int luaopen_pigeon(lua_State *l);

#endif
