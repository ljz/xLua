/****************************************************************************
 Copyright (c) 2013-2017 Chukong Technologies Inc.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#ifndef __COCOS2DX_SCRIPTING_LUA_COCOS2DXSUPPORT_LUABAISCCONVERSIONS_H__
#define __COCOS2DX_SCRIPTING_LUA_COCOS2DXSUPPORT_LUABAISCCONVERSIONS_H__

#include <unordered_map>
#include <string>

extern "C" {
#include "lua.h"
#include "tolua++.h"
}
//#include "scripting/lua-bindings/manual/tolua_fix.h"

//#include "scripting/lua-bindings/manual/Lua-BindingsExport.h"
//#include "editor-support/cocostudio/CocosStudioExtension.h"
 /*
#include "2d/CCLabel.h"
#include "2d/CCSprite.h"
#include "3d/CCBundle3D.h"
#include "base/CCValue.h"
#include "base/ccTypes.h"
#include "deprecated/CCArray.h"
#include "deprecated/CCDictionary.h"
#include "physics/CCPhysicsContact.h"
#include "physics/CCPhysicsJoint.h"
#include "physics/CCPhysicsShape.h"
#include "physics/CCPhysicsWorld.h"
#include "renderer/CCGLProgram.h"
 */

//using namespace cocos2d;
using namespace std;

//std::unordered_map<std::string, std::string>  g_luaType;
//std::unordered_map<std::string, std::string>  g_typeCast;

#if COCOS2D_DEBUG >=1
void luaval_to_native_err(lua_State* L,const char* msg,tolua_Error* err, const char* funcName = "");
#endif

#define LUA_PRECONDITION( condition, ...) if( ! (condition) ) {														\
cocos2d::log("lua: ERROR: File %s: Line: %d, Function: %s", __FILE__, __LINE__, __FUNCTION__ );                                                         \
cocos2d::log(__VA_ARGS__);                                                  \
}                                                                           \

/**
 * @addtogroup lua
 * @{
 */

/**
 * If the typename of userdata at the given acceptable index of stack is equal to type it return true, otherwise return false.
 * If def != 0, lo could greater than the top index of stack, return value is true.
 * If the value of the given index is nil, return value also is true.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param type the typename used to judge.
 * @param def whether has default value.
 * @return Return true if the typename of userdata at the given acceptable index of stack is equal to type, otherwise return false.
 */
extern bool luaval_is_usertype(lua_State* L,int lo,const char* type, int def);
// to native

/**
 * @name luaval_to_native
 * The following function are all used to convert the Lua values at the given acceptable index to the corresponding c++ values.
 * If the Lua values can be converted the return value is true, otherwise return false.
 * If it happens error during the conversion process, it outputs the error msg in the console to provide information about the name of calling function, the typename of value at the given acceptable index, and so on.
 * @{
 **/


/**
 * Get a unsigned long value from the given acceptable index of stack.
 * If the value at the given acceptable index of stack is a number or a string convertible to a number it returns true, otherwise returns false.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param outValue the pointer to store the value converted from the Lua value.
 * @param funcName the name of calling function, it is used for error output in the debug model.
 * @return Return true if the value at the given acceptable index of stack is a number or a string convertible to a number, otherwise return false.
 */
extern bool luaval_to_ulong(lua_State* L,int lo, unsigned long* outValue, const char* funcName="");

/**
 * Get a unsigned short value from the given acceptable index of stack.
 * If the value at the given acceptable index of stack is a number or a string convertible to a number it returns true, otherwise returns false.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param outValue the pointer to store the unsigned short value converted from the Lua value.
 * @param funcName the name of calling function, it is used for error output in the debug model.
 * @return Return true if the value at the given acceptable index of stack is a number or a string convertible to a number, otherwise return false.
 */
extern bool luaval_to_ushort(lua_State* L, int lo, unsigned short* outValue, const char* funcName = "");

/**
 * Get a int value from the given acceptable index of stack.
 * If the value at the given acceptable index of stack is a number or a string convertible to a number it returns true, otherwise returns false.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param outValue the pointer to store the int value converted from the Lua value.
 * @param funcName the name of calling function, it is used for error output in the debug model.
 * @return Return true if the value at the given acceptable index of stack is a number or a string convertible to a number, otherwise return false.
 */
extern bool luaval_to_int32(lua_State* L,int lo,int* outValue, const char* funcName = "");

/**
 * Get a boolean value from the given acceptable index of stack.
 * If the value at the given acceptable index of stack has type boolean it returns true, otherwise returns false.
 * Any Lua value is different from false and nil, the value of conversion is true, otherwise the value is false.
 * If the lo is non-valid index, the value of conversion also is false.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param outValue the pointer to store the boolean value converted from the Lua value.
 * @param funcName the name of calling function, it is used for error output in the debug model.
 * @return Return true if the value at the given acceptable index of stack is a number or a string convertible to a number, otherwise return false.
 */
extern bool luaval_to_boolean(lua_State* L,int lo,bool* outValue, const char* funcName = "");

/**
 * Get a double value from the given acceptable index of stack.
 * If the value at the given acceptable index of stack is a number or a string convertible to a number it returns true, otherwise returns false.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param outValue the pointer to store the double value converted from the Lua value.
 * @param funcName the name of calling function, it is used for error output in the debug model.
 * @return Return true if the value at the given acceptable index of stack is a number or a string convertible to a number, otherwise return false.
 */
extern bool luaval_to_number(lua_State* L,int lo,double* outValue, const char* funcName = "");

/**
 * Get a long long value from the given acceptable index of stack.
 * If the value at the given acceptable index of stack is a number or a string convertible to a number it returns true, otherwise returns false.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param outValue the pointer to store the long long value converted from the Lua value.
 * @param funcName the name of calling function, it is used for error output in the debug model.
 * @return Return true if the value at the given acceptable index of stack is a number or a string convertible to a number, otherwise return false.
 */
extern bool luaval_to_long_long(lua_State* L,int lo,long long* outValue, const char* funcName = "");

/**
 * Get a std::string value from the given acceptable index of stack.
 * If the value at the given acceptable index of stack is a string or a number convertible to a string it returns true, otherwise returns false.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param outValue the pointer to store std::string value converted from the Lua value.
 * @param funcName the name of calling function, it is used for error output in the debug model.
 * @return Return true if the value at the given acceptable index of stack is a string or a number convertible to a string, otherwise return false.
 */
extern bool luaval_to_std_string(lua_State* L, int lo, std::string* outValue, const char* funcName = "");

/**
 * Get a long value from the given acceptable index of stack.
 * If the value at the given acceptable index of stack is a number or a string convertible to a number it returns true, otherwise returns false.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param outValue the pointer to store the long value converted from the Lua value.
 * @param funcName the name of calling function, it is used for error output in the debug model.
 * @return Return true if the value at the given acceptable index of stack is a number or a string convertible to a number, otherwise return false.
 */
extern bool luaval_to_long(lua_State* L,int lo, long* outValue, const char* funcName = "");

/**
 * Get a ssize_t value from the given acceptable index of stack.
 * If the value at the given acceptable index of stack is a number or a string convertible to a number it returns true, otherwise returns false.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param outValue the pointer to store the ssize_t value converted from the Lua value.
 * @param funcName the name of calling function, it is used for error output in the debug model.
 * @return Return true if the value at the given acceptable index of stack is a number or a string convertible to a number, otherwise return false.
 */
//extern bool luaval_to_ssize(lua_State* L,int lo, ssize_t* outValue, const char* funcName = "");

/**
 * Get a Size object value from the given acceptable index of stack.
 * If the value at the given acceptable index of stack is a table it returns true, otherwise returns false.
 * If the table has the `width` and `height` key and the corresponding values are not nil, this function would assign the values to the corresponding members of outValue. Otherwise, the value of members of outValue would be 0.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param outValue the pointer to a Size object which stores the values from the Lua table.
 * @param funcName the name of calling function, it is used for error output in the debug model.
 * @return Return true if the value at the given acceptable index of stack is a table, otherwise return false.
 */
//extern bool luaval_to_size(lua_State* L,int lo,Size* outValue, const char* funcName = "");

/**
 * Get a Rect object value from the given acceptable index of stack.
 * If the value at the given acceptable index of stack is a table it returns true, otherwise returns false.
 * If the table has the `x`,`y`,`width` and `height` keys and the corresponding values are not nil, this function would assign the values to the corresponding members of outValue. Otherwise, the value of members of outValue would be 0.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param outValue the pointer to a Rect object which stores the values from the Lua table.
 * @param funcName the name of calling function, it is used for error output in the debug model.
 * @return Return true if the value at the given acceptable index of stack is a table, otherwise return false.
 */
//extern bool luaval_to_rect(lua_State* L,int lo,Rect* outValue, const char* funcName = "");

/**
 * Get a Color3B object value from the given acceptable index of stack.
 * If the value at the given acceptable index of stack is a table it returns true, otherwise returns false.
 * If the table has the `r`,`g` and `b` keys and the corresponding values are not nil, this function would assign the values to the corresponding members of outValue. Otherwise, the value of members of outValue would be 0.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param outValue the pointer to a Color3B object which stores the values from the Lua table.
 * @param funcName the name of calling function, it is used for error output in the debug model.
 * @return Return true if the value at the given acceptable index of stack is a table, otherwise return false.
 */
//extern CC_LUA_DLL bool luaval_to_color3b(lua_State* L,int lo,Color3B* outValue, const char* funcName = "");

/**
 * Get a Color4B object value from the given acceptable index of stack.
 * If the value at the given acceptable index of stack is a table it returns true, otherwise returns false.
 * If the table has the `r`,`g`, `b` and 'a' keys and the corresponding values are not nil, this function would assign the values to the corresponding members of outValue. Otherwise, the value of members of outValue would be 0.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param outValue the pointer to a Color4B object which stores the values from the Lua table.
 * @param funcName the name of calling function, it is used for error output in the debug model.
 * @return Return true if the value at the given acceptable index of stack is a table, otherwise return false.
 */
//extern bool luaval_to_color4b(lua_State* L,int lo,Color4B* outValue, const char* funcName = "");

/**
 * Get a Color4F object value from the given acceptable index of stack.
 * If the value at the given acceptable index of stack is a table it returns true, otherwise returns false.
 * If the table has the `r`,`g`, `b` and 'a' keys and the corresponding values are not nil, this function would assign the values to the corresponding members of outValue. Otherwise, the value of members of outValue would be 0.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param outValue the pointer to a Color4F object which stores the values from the Lua table.
 * @param funcName the name of calling function, it is used for error output in the debug model.
 * @return Return true if the value at the given acceptable index of stack is a table, otherwise return false.
 */
//extern bool luaval_to_color4f(lua_State* L,int lo,Color4F* outValue, const char* funcName = "");
#if CC_USE_PHYSICS

/**
 * Get a PhysicsMaterial object value from the given acceptable index of stack.
 * If the value at the given acceptable index of stack is a table it returns true, otherwise returns false.
 * If the table has the `density`,`restitution` and 'friction' keys and the corresponding values are not nil, this function would assign the values to the corresponding members of outValue. Otherwise, the value of members of outValue would be 0.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param outValue the pointer to a cocos2d::PhysicsMaterial object which stores the values from the Lua table.
 * @param funcName the name of calling function, it is used for error output in the debug model.
 * @return Return true if the value at the given acceptable index of stack is a table, otherwise return false.
 */
extern bool luaval_to_physics_material(lua_State* L,int lo, cocos2d::PhysicsMaterial* outValue, const char* funcName = "");
#endif //#if CC_USE_PHYSICS

/**
 * Get a cocos2d::Vector of type T objects by the argc numbers of Lua values in the stack.
 *
 * @param L the current lua_State.
 * @param argc the numbers of Lua values in the stack.
 * @param ret a cocos2d::Vector of type T objects.
 * @return Return false if argc equal to 0 , L equal to nullptr or the Lua userdata at the index isn't `cc.Ref` type, otherwise return true.
 */
template <class T>
bool luavals_variadic_to_ccvector( lua_State* L, int argc, std::vector<T>* ret)
{
    if (nullptr == L || argc == 0 )
        return false;

    bool ok = true;

    for (int i = 0; i < argc; i++)
    {
        if (lua_isuserdata(L, i + 2))
        {
            tolua_Error err;

            if (!tolua_isusertype(L, i + 2, "cc.Ref", 0, &err))
            {
                ok = false;
                break;
            }
            T obj = static_cast<T>(tolua_tousertype(L, i + 2, nullptr));
            ret->pushBack(obj);
        }
    }

    return ok;
}

/**
 * Get a Type T object from the given acceptable index of stack.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param type a string pointer points to the type name.
 * @param ret the pointer points to a Type T object.
 * @return Return true if the type of Lua value at the index is a Lua userdata, otherwise return false.
 */
template <class T>
bool luaval_to_object(lua_State* L, int lo, const char* type, T** ret, const char* funcName = "")
{
    if(nullptr == L || lua_gettop(L) < lo)
        return false;

    if (!luaval_is_usertype(L, lo, type, 0))
        return false;

    *ret = static_cast<T*>(tolua_tousertype(L, lo, 0));

    if (nullptr == *ret)
        //CCLOG("Warning: %s argument %d is invalid native object(nullptr)", funcName, lo);

    return true;
}

/**
 * Get a cocos2d::MeshVertexAttrib object value from the given acceptable index of stack.
 * If the value at the given acceptable index of stack is a table it returns true, otherwise returns false.
 * If the table has the `size`, `type`, `vertexAttrib`, `vertexAttrib` and `attribSizeBytes` keys and the corresponding values are not nil, this function would assign the values to the corresponding members of outValue.
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param ret the pointer to a cocos2d::MeshVertexAttrib object which stores the values from the Lua table.
 * @param funcName the name of calling function, it is used for error output in the debug model.
 * @return Return true if the value at the given acceptable index of stack is a table, otherwise return false.
 */
//extern bool luaval_to_mesh_vertex_attrib(lua_State* L, int lo, cocos2d::MeshVertexAttrib* ret, const char* funcName = "");

/**
 * Get a pointer points to a std::vector<float> from a Lua array table in the stack.
 *
 * @param L the current lua_State.
 * @param lo the given acceptable index of stack.
 * @param ret a pointer points to a std::vector<float>.
 * @param funcName the name of calling function, it is used for error output in the debug model.
 * @return Return true if the value at the given acceptable index of stack is a table, otherwise return false.
 */
extern bool luaval_to_std_vector_float(lua_State* L, int lo, std::vector<float>* ret, const char* funcName = "");

/**
 * Get the real typename for the specified typename.
 * Because all override functions wouldn't be bound,so we must use `typeid` to get the real class name.
 *
 * @param ret the pointer points to a type T object.
 * @param type the string pointer points to specified typename.
 * @return return the pointer points to the real typename, or nullptr.
 */
template <class T>
const char* getLuaTypeName(T* ret,const char* type)
{
    if (nullptr != ret)
    {
        std::string hashName = typeid(*ret).name();
        auto iter =  g_luaType.find(hashName);
        if(g_luaType.end() != iter)
        {
            return iter->second.c_str();
        }
        else
        {
            return type;
        }
    }

    return nullptr;
}

/**
* Push the native object by userdata format into the Lua stack by typename.
*
* @param L the current lua_State.
* @param type the string pointer points to the typename.
* @param ret the native object pointer.
*/
template <class T>
void object_to_luaval(lua_State* L, const char* type, T* ret)
{
	if (nullptr != ret)
	{
		//if (std::is_base_of<cocos2d::Ref, T>::value)
		//{
			// use c style cast, T may not polymorphic
		//	cocos2d::Ref* dynObject = (cocos2d::Ref*)(ret);
		//	int ID = (int)(dynObject->_ID);
		//	int* luaID = &(dynObject->_luaID);
		//	toluafix_pushusertype_ccobject(L, ID, luaID, (void*)ret, type);
		//}
		//else
		//{
			//tolua_pushusertype(L, (void*)ret, getLuaTypeName(ret, type));
		//}
	}
	else
	{
		lua_pushnil(L);
	}
}

// end group
/// @}
#endif //__COCOS2DX_SCRIPTING_LUA_COCOS2DXSUPPORT_LUABAISCCONVERSIONS_H__
