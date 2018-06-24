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

using System.Collections.Generic;
using System;

namespace XLua
{
	public class ObjectTranslatorPool
	{
		private Dictionary<RealStatePtr, WeakReference> translators = new Dictionary<RealStatePtr, WeakReference>();
		
		public static ObjectTranslatorPool Instance
		{
			get
			{
				return InternalGlobals.objectTranslatorPool;
			}
		}

#if UNITY_EDITOR || XLUA_GENERAL
		//根据一个真实的状态指针查找转换器
		//就是从对象转换器池子里面去找
        public static ObjectTranslator FindTranslator(RealStatePtr L)
        {
            return InternalGlobals.objectTranslatorPool.Find(L);
        }
#endif

        public ObjectTranslatorPool ()
		{
		}
		
		//添加一个对象转换器到这个池子里面
		//指针是状态指针
		public void Add (RealStatePtr L, ObjectTranslator translator)
		{
			//根据指针从使用luaapi去转换为指针
            var ptr = LuaAPI.xlua_gl(L);
            lastPtr = ptr;
            lastTranslator = translator;
#if !SINGLE_ENV
			//加进来,ptr为key,加进来的是弱引用.
            translators.Add(ptr , new WeakReference(translator));
#endif   
        }

        RealStatePtr lastPtr = default(RealStatePtr);
        ObjectTranslator lastTranslator = default(ObjectTranslator);

		public ObjectTranslator Find (RealStatePtr L)
		{
#if SINGLE_ENV
            return lastTranslator;
#else
            var ptr = LuaAPI.xlua_gl(L);
            if (lastPtr == ptr) return lastTranslator;
            if (translators.ContainsKey(ptr))
            {
                lastPtr = ptr;
                lastTranslator = translators[ptr].Target as ObjectTranslator;
                return lastTranslator;
            }
			
			return null;
#endif
        }
		
		public void Remove (RealStatePtr L)
		{
#if SINGLE_ENV
            lastPtr = default(RealStatePtr);
            lastTranslator = default(ObjectTranslator);
#else
            var ptr = LuaAPI.xlua_gl(L);
            if (!translators.ContainsKey (ptr))
				return;
			
            if (lastPtr == ptr)
            {
                lastPtr = default(RealStatePtr);
                lastTranslator = default(ObjectTranslator);
            }

            translators.Remove(ptr);
#endif
        }
    }
}

