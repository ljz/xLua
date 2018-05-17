//
//  lua_kcp.hpp
//  pigeon
//
//  Created by Vega on 2017/10/26.
//  Copyright © 2017年 gamebase. All rights reserved.
//

#ifndef lua_kcp_hpp
#define lua_kcp_hpp

#include <stdio.h>
#include <functional>
#ifdef __cplusplus
extern "C" {
#endif
#include "tolua++.h"
#ifdef __cplusplus
}
#endif

extern void pigeonPerformInLuaThread(const std::function<void()> &function);

#endif /* lua_kcp_hpp */
