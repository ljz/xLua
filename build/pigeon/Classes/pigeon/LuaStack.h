#ifndef __LUA_STACK_H_
#define __LUA_STACK_H_

extern "C" {
#include "lua.h"
}

class LuaStack
{
public:
    static LuaStack *create(void);
    virtual ~LuaStack();
    lua_State* getLuaState(void) {
        return _state;
    }
    
    virtual void removeScriptHandler(int nHandler);
    

    virtual int executeString(const char* codes);
    
    /**
     * Execute a script file.
     *
     * @param filename String object holding the filename of the script file that is to be executed.
     * @return the return values by calling executeFunction.
     */
    virtual int executeScriptFile(const char* filename);

    /**
     * Execute a scripted global function.
     * The function should not take any parameters and should return an integer.
     *
     * @param functionName String object holding the name of the function, in the global script environment, that is to be executed.
     * @return The integer value returned from the script function.
     */
    virtual int executeGlobalFunction(const char* functionName);
    
    /**
     * Set the stack top index 0.
     */
    virtual void clean(void);
    
    /**
     * Pushes a integer number with value intValue onto the stack.
     * 
     * @param intValue a integer number.
     */
    virtual void pushInt(int intValue);
    
    /**
     * Pushes a float number with value floatValue onto the stack.
     *
     * @param floatValue a float number.
     */
    virtual void pushFloat(float floatValue);
    
    /**
     * Pushes a long number with value longValue onto the stack.
     * 
     * @param longValue a long number.
     */
    virtual void pushLong(long longValue);
    
    /**
     * Pushes a bool value with boolValue onto the stack.
     * 
     * @param boolValue a bool value.
     */
    virtual void pushBoolean(bool boolValue);
    
    /**
     * Pushes the zero-terminated string pointed to by stringValue onto the stack.
     *
     * @param stringValue a pointer point to a zero-terminated string stringValue.
     */
    virtual void pushString(const char* stringValue);
    
    /**
     * Pushes the string pointed to by stringValue with size length onto the stack.
     *
     * @param stringValue a pointer point to the string stringValue.
     * @param length the size.
     */
    virtual void pushString(const char* stringValue, int length);
    
    /**
     * Pushes a nil value onto the stack.
     */
    virtual void pushNil(void);
    
    /**
     * Pushes a Ref object onto the stack.
     *
     * @see toluafix_pushusertype_ccobject.
     */
    virtual void pushObject(Ref* objectValue, const char* typeName);
    
    /**
     * According to the type of LuaValue, it would called the other push function in the internal.
     *   type                           function
     * LuaValueTypeInt                   pushInt
     * LuaValueTypeFloat                 pushFloat
     * LuaValueTypeBoolean               pushBoolean
     * LuaValueTypeString                pushString
     * LuaValueTypeDict                  pushLuaValueDict
     * LuaValueTypeArray                 pushLuaValueArray
     * LuaValueTypeObject                pushObject
     * 
     * @param value a LuaValue object.
     */
    virtual void pushLuaValue(const LuaValue& value);
    
    /**
     * Pushes a lua table onto the stack.
     * The key of table is the key of LuaValueDict which is std::map.
     * The value of table is according to the type of LuaValue of LuaValueDict by calling pushLuaValue,@see pushLuaValue.
     *
     * @param dict a LuaValueDict object.
     */
    virtual void pushLuaValueDict(const LuaValueDict& dict);
    
    /**
     * Pushes a lua array table onto the stack.
     * The index of array table is begin at 1.
     * The value of array table is according to the type of LuaValue of LuaValueDict by calling pushLuaValue,@see pushLuaValue.
     */
    virtual void pushLuaValueArray(const LuaValueArray& array);
    
    /**
     * Get the lua function pointer from toluafix_refid_function_mapping table by giving nHanlder.
     * If the lua function pointer corresponding to the nHanlder isn't found, it would push nil on the top index of stack, then it would output the error log in the debug model.
     *
     * @return true if get the no-null function pointer otherwise false.
     */
    virtual bool pushFunctionByHandler(int nHandler);
    
    /**
     * Execute the lua function on the -(numArgs + 1) index on the stack by the numArgs variables passed.
     *
     * @param numArgs the number of variables.
     * @return 0 if it happen the error or it hasn't return value, otherwise it return the value by calling the lua function.
     */
    virtual int executeFunction(int numArgs);
    
    /**
     * Execute the lua function corresponding to the nHandler by the numArgs variables passed.
     *
     * @param nHandler the index count corresponding to the lua function.
     * @param numArgs the number of variables.
     * @return the return value is the same as executeFunction,please @see executeFunction.
     */
    virtual int executeFunctionByHandler(int nHandler, int numArgs);
    
    /**
     * Execute the lua function corresponding to the handler by the numArgs variables passed.
     * By calling this function, the number of return value is numResults(may be > 1).
     * All the return values are stored in the resultArray.
     *
     * @param handler the index count corresponding to the lua function.
     * @param numArgs the number of variables.
     * @param numResults the number of return value.
     * @param resultArray a array used to store the return value.
     * @return 0 if it happen error or it hasn't return value, otherwise return 1.
     */
    virtual int executeFunctionReturnArray(int handler,int numArgs,int numResults,__Array& resultArray);
    
    /**
     * Execute the lua function corresponding to the handler by the numArgs variables passed.
     * By calling this function, the number of return value is numResults(may be > 1).
     * All the return values are used in the callback func.
     *
     * @param handler the index count corresponding to the lua function.
     * @param numArgs the number of variables.
     * @param numResults the number of return value.
     * @param func callback function which is called if the numResults > 0.
     * @return 0 if it happen error or it hasn't return value, otherwise return 1.
     */
    virtual int executeFunction(int handler, int numArgs, int numResults, const std::function<void(lua_State*,int)>& func);
    
    /**
     * Handle the assert message.
     *
     * @return return true if current _callFromLua of LuaStack is not equal to 0 otherwise return false.
     */
    virtual bool handleAssert(const char *msg);
    
    /**
     * Set the key and sign for xxtea encryption algorithm.
     *
     * @param key a string pointer
     * @param keyLen the length of key
     * @param sign a string sign
     * @param signLen the length of sign
     */
    virtual void setXXTEAKeyAndSign(const char *key, int keyLen, const char *sign, int signLen);
    
    /**
     * free the key and sign for xxtea encryption algorithm.
     */
    virtual void cleanupXXTEAKeyAndSign();
    
    /**
     * Loads a buffer as a Lua chunk.This function uses lua_load to load the Lua chunk in the buffer pointed to by chunk with size chunkSize.
     * If it supports xxtea encryption algorithm, the chunk and the chunkSize would be processed by calling xxtea_decrypt to the real buffer and buffer size.
     *
     * @param L the current lua_State.
     * @param chunk the buffer pointer.
     * @param chunkSize the size of buffer.
     * @param chunkName the name of chunk pointer.
     * @return 0, LUA_ERRSYNTAX or LUA_ERRMEM:.
     */
    int luaLoadBuffer(lua_State *L, const char *chunk, int chunkSize, const char *chunkName);
    
    /**
     * Load the Lua chunks from the zip file
     * 
     * @param zipFilePath file path to zip file.
     * @return 1 if load successfully otherwise 0.
     */
    int loadChunksFromZIP(const char *zipFilePath);
    
    /**
     * Load the Lua chunks from current lua_State.
     *
     * @param L the current lua_State.
     * @return 1 if load successfully otherwise 0.
     */
    int luaLoadChunksFromZIP(lua_State *L);
    
public:
    // gamebase extentions BEGIN. Added by hailong.
    virtual bool isRef(int ref);
    virtual int ref(int idx);
    virtual void deref(int ref);
    virtual void retainRef(int ref);
    virtual void releaseRef(int ref);
    /**
     * Call function.
     * @param numArgs number of arguments.
     * @param numResults number of return values.
     * @return the number of results on the stack if succeeds, negative if fails.
     */
    virtual int call(int numArgs, int numResults);
    /**
     * Call method of obj. obj[methodName](t, ...)
     * @param obj  the stack position of target object.
     * @param methodName the name of method.
     * @param numArgs number of arguments.
     * @param numResults number of return values.
     * @return the number of results on the stack if succeeds, negative if fails.
     */
    virtual int callMethod(int obj, const char* methodName, int numArgs, int numResults);
    virtual void pop(int n);
    // gamebase extentions END.
protected:
    LuaStack(void)
    : _state(nullptr)
    , _callFromLua(0)
    , _xxteaEnabled(false)
    , _xxteaKey(nullptr)
    , _xxteaKeyLen(0)
    , _xxteaSign(nullptr)
    , _xxteaSignLen(0)
    {
    }
    
    bool init(void);
    bool initWithLuaState(lua_State *L);
    
    lua_State *_state;
    int _callFromLua;
    bool  _xxteaEnabled;
    char* _xxteaKey;
    int   _xxteaKeyLen;
    char* _xxteaSign;
    int   _xxteaSignLen;
};

#endif // __LUA_STACK_H_
