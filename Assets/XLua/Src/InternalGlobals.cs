/*
 * Tencent is pleased to support the open source community by making xLua available.
 * Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.
 * Licensed under the MIT License (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#if USE_UNI_LUA
using LuaAPI = UniLua.Lua;
using RealStatePtr = UniLua.ILuaState;
using LuaCSFunction = UniLua.CSharpFunctionDelegate;
#else
using LuaAPI = XLua.LuaDLL.Lua;
using RealStatePtr = System.IntPtr;
using LuaCSFunction = XLua.LuaDLL.lua_CSFunction;
#endif

using System;
using System.Collections.Generic;
using System.Reflection;

namespace XLua
{
    //这个就是全局空间的变量.XLua使用的变量统一放到这个里面
    //partial:表示这个类可以写成几个部分
    //内部的全局方法,只能在程序集中访问
    internal partial class InternalGlobals
    {
        //字符串缓存,往lua虚拟机里面推之后,都会用这个变换缓存一下.
        internal static byte[] strBuff = new byte[256];

        //从Array里面Get/Set类型.
        internal delegate bool TryArrayGet(Type type, RealStatePtr L, ObjectTranslator translator, object obj, int index);
        internal delegate bool TryArraySet(Type type, RealStatePtr L, ObjectTranslator translator, object obj, int array_idx, int obj_idx);
        internal static TryArrayGet genTryArrayGetPtr = null;
        internal static TryArraySet genTryArraySetPtr = null;

        //对象转换器池
        //对象转换器有什么卵用?
        internal static volatile ObjectTranslatorPool objectTranslatorPool = new ObjectTranslatorPool();

        //Lua注册索引的默认值
        internal static int LUA_REGISTRYINDEX = -10000;

        //支持的操作,如果是发现是处理一个操作符,则需要判断是否是这里支持的,如果不支持就不处理,只有支持的才处理.
        internal static Dictionary<string, string> supportOp = new Dictionary<string, string>()
        {
            { "op_Addition", "__add" },
            { "op_Subtraction", "__sub" },
            { "op_Multiply", "__mul" },
            { "op_Division", "__div" },
            { "op_Equality", "__eq" },
            { "op_UnaryNegation", "__unm" },
            { "op_LessThan", "__lt" },
            { "op_LessThanOrEqual", "__le" },
            { "op_Modulus", "__mod" },
            { "op_BitwiseAnd", "__band" },
            { "op_BitwiseOr", "__bor" },
            { "op_ExclusiveOr", "__bxor" },
            { "op_OnesComplement", "__bnot" },
            { "op_LeftShift", "__shl" },
            { "op_RightShift", "__shr" },
        };

        //扩展方法字典
        internal static Dictionary<Type, IEnumerable<MethodInfo>> extensionMethodMap = null;

#if GEN_CODE_MINIMIZE
    //CSharpWraper调用者
        internal static LuaDLL.CSharpWrapperCaller CSharpWrapperCallerPtr = new LuaDLL.CSharpWrapperCaller(StaticLuaCallbacks.CSharpWrapperCallerImpl);
#endif

        //懒惰的反射wraper对象,lua调用CSharp函数.我猜测是如果没有到处接口,就是用反射的方式来找到需要调用的接口.
        //这种方式是比较低效率的.
        //根据Utils.LazyReflectionCall对象来创建的LuaCSFunction对象.
        internal static LuaCSFunction LazyReflectionWrap = new LuaCSFunction(Utils.LazyReflectionCall);
    }

}

