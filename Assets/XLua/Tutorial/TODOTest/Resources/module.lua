-- Lua模块

GlobalENV = _G
local string = string--字符串
local pairs = pairs
local table = table
local setmetatable = setmetatable--设置元表的函数
local debug = debug
local rawset = rawset--往表里面增加一条内容
local rawget = rawget--从表里面获取一条内容
local getmetatable = getmetatable--获取元表

--模块写锁的key,暂时不知道干嘛的.
local ModuleWriteLockKey = "__bRegModuleGlobalObj__"

--字符串编码默认环境,暂时不知道干嘛的
local StringCodeDefaultEnv = nil

--加一条数据.刚开始如果没有就设置为{},这个用来存储Import进来的模块.
GlobalENV.__ImportedModule__ = GlobalENV.__ImportedModule__ or {}
local __ImportedModule__ = GlobalENV.__ImportedModule__

--Import的全局模块,和上面这个有什么区别?不知.
GlobalENV.__ImportedGlobalModule__ = GlobalENV.__ImportedGlobalModule__ or {}
local __ImportedGlobalModule__ = GlobalENV.__ImportedGlobalModule__

--暂时理解为是否注册了注册全局对象
local bRegGlobalObj = false

--不能够全局环境写入
function DisableGlobalENVWrite()
    local globalENVMT = {
    	__index = function(t,k) return rawget(t,k) end,
        __newindex = function(t, k, v)
        	if bRegGlobalObj or IsPetTool then
        		rawset(t, k, v)
        		return
        	end
        	local dataInfo = string.format("dataInfo:(key=%s,value=%s)\n",k,v)
        	local errorMsg = "attempt to update _G table!!!!!\n"..dataInfo..debug.traceback()
            error(errorMsg)
        end
    }
 	setmetatable(GlobalENV, globalENVMT)
end

--锁住模块写功能.
function LockModuleWrite(t)
	rawset(t,ModuleWriteLockKey,false)
end

function UnlockModuleWrite(t)
	rawset(t,ModuleWriteLockKey,true)
end

function ModuleNewindexFunc(t, k, v)
	if not rawget(t,"__IsModule__") or IsPetTool then
		rawset(t, k, v)
		return
	end
	
	if rawget(t,ModuleWriteLockKey) then
		rawset(t, k, v)
		return
	end
	
	local dataInfo = string.format("dataInfo:moduleName=%s,key=%s,value=%s\n",rawget(t,"__MODULENAME__"),k,v)
	local errorMsg = "attempt to update module table!!!!!\n"..dataInfo..debug.traceback()
    error(errorMsg)
end

--register to _G with key = modulename, value = moduleobject
function RegGlobalObj(InKey,InObj)
	if InKey == nil then
		return
	end

	rawset(GlobalENV, InKey,InObj)
end

function UnRegGlobalObj(InKey)
	rawset(GlobalENV, InKey,nil)
end

--注册模块全局对象
function RegModGlobalObj(InModObj,InKey,InObj)
	if InModObj == nil or InKey == nil then
		return
	end
	rawset(InModObj, InKey,InObj)
end

--删除模块全局对象
function UnRegModGlobalObj(InModObj,InKey)
	rawset(InModObj, InKey,nil)
end

-- 加载模块时调用模块的Init函数
function CallModuleInit(InModule)
	local initFunc = rawget(InModule, "__Init__")
	if initFunc then
		initFunc(InModule)
	end
end

-- 释放模块时调用模块的Release函数
function CallModuleRelease(InOldModule)
	local releaseFunc = rawget(InOldModule, "__Release__")
	if releaseFunc then
		releaseFunc(InOldModule)
	end
	
	local oldModuleMT = getmetatable(InOldModule)
	if oldModuleMT then
		oldModuleMT.__newindex = nil 
		oldModuleMT.__index = nil
		oldModuleMT.__PATH__ = InPath
		oldModuleMT.__MODULENAME__ = nil
		oldModuleMT.__IsModule__ = nil
	end
end

-- 更新模块时调用模块的Update函数.
function CallModuleUpdate(InModule)
	local updateFunc = rawget(InModule, "__update__")
	if updateFunc then
		updateFunc(InModule)
	end
end


-- InModule可能为nil
function OnModuleLoaded(InModule)
	-- 判断一下，可能没有加载enum模块
	if enum then
		-- 枚举定义处理
		enum.EndEnum()
	end
	--加载之后就可以写,不锁了?难道是这样?
	rawset(InModule,ModuleWriteLockKey,false)
end

--得到新的模块环境table
function GetNewModuleEnvTbl(InPath,InModuleName)
	local newEnv = {}
	newEnv.__PATH__ = InPath
	newEnv.__MODULENAME__ = InModuleName
	newEnv.__IsModule__ = true
	newEnv.__LockModuleWrite__ = LockModuleWrite
	newEnv.__UnlockModuleWrite__ = UnlockModuleWrite
	newEnv[ModuleWriteLockKey] = true
	
	local ModuleEnvMT = {
		__index = GlobalENV,
		__newindex = ModuleNewindexFunc,
	}
	setmetatable(newEnv,ModuleEnvMT)
	return newEnv
end

--加载文件
function __loadfile__(bGlobal,InPath,InModuleDict)
	-- require("xls2lua.main")

	-- print("__loadfile__.InPath =========================================================")
	-- print(InPath)
	-- print("::::::")
	-- print("11liaojiangzhenggg")
	-- print(tostring(InPath))
	-- if InPath == "common.platform" then
	-- 	print("164444444444444444")
	-- 	print("-__loadfile__>>>>bGlobal=")
	-- 	print(bGlobal)
	-- 	print("InPath ===".. InPath)
	-- 	print("InModuleDict == ")
	-- 	print(InModuleDict)
	-- 	print("IsClient>>")
	-- 	print(IsClient)
	-- 	print("::::::::::>>>>172")
	-- 	print("1733333333333333333333")
	-- else
	-- 	print(" >>>>in else")
	-- end
	-- print("177777777")
	-- print("174444444")
	local path = nil
	print("23?33333333333333333")
	-- if IsClient and IsClient() then
	-- if 1 == 1 then
	-- 	print("183333333333333333333")
	-- 	print("ToolFindFile ====")
	-- 	-- print(tostring(ToolFindFile))
	-- 	print(":::::::")
		if ToolFindFile then
			-- print("ToolFindFile is not false or nil")
			path = ToolFindFile(InPath)
		else
			-- print("ToolFindFile is nil or false")
			path = InPath
		end
	-- else
	-- 	-- print("1944444444444444444444")
 --        path = string.gsub(InPath, "%.", "/") .. ".lua"
	-- end
	-- if InPath == "common.platform" then
	-- 	print(">>>>>>>true lua path ========="..path)
	-- end
	if path == "app.main" then
		print("path ================================================"..path)
		-- path = "/Users/apple/Desktop/unity学习文件夹/xlua/Assets/XLua/Tutorial/TODOTest/Resources/app/main.lua"
	end
	local result = require(path)
	print("result ===")
	print(result)
	return result
	-- if bGlobal then
	-- 	return SysLoader.LoadScript(path,"bt")
	-- else
	-- 	print("path == ")
	-- 	print(path)
	-- 	-- print("")
	-- 	-- local result =  SysLoader.LoadScript(path,"bt",InModuleDict)
	-- 	if  result ~= nil then
	-- 		print("result =========="..result)
	-- 	else
	-- 		print("result === nil ")
	-- 	end
	-- 	return result
	-- end
end


-- function __loadfile__(bGlobal, InPath, InModuleDict)
-- 	-- if InPath == "commo?"

-- end

--不能直接用
function __InnerImport__(InModuleName,InPath,isNew)
	if InModuleName == "platform" then
		print("__InnerImport__ args is ")
		print("InModuleName == ")
		print(InModuleName)
		print( "InPath == ")
		print(InPath)
		print("isNew ====")
		print(isNew)
	end
	if InPath == nil or InModuleName == nil then
		LOGE("加载Lua脚本出错",InPath,InModuleName)
		return nil
	end
	
	if not isNew then
		local OldModule = __ImportedModule__[InModuleName]
		if InModuleName == "platform" then
			print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>is not New")
			print("OldModule =========================")
			print(OldModule)
			print("#__ImportedModule__ === "..#__ImportedModule__)
		end
		if OldModule then
			return OldModule
		end
	end

	-- 预先放入路径名和模块名,部分模块会使用
	local NewModule = GetNewModuleEnvTbl(InPath,InModuleName)
	
	if not isNew then
		RegGlobalObj(InModuleName,NewModule)
		if InModuleName == "platform" then
			print("NewModule ===")
			print(NewModule)
			print("206>>>>>>>>>>>>>>>>>>>>>>>>>>>>>is not New")
			print("_G.platform ===")
			print(_G.platform)
			print("::::::")
		end
		-- 先添加到_G再执行Func()
	end
	
	-- 第三个参数作为模块闭包函数的env
	local Func,Err = __loadfile__(false,InPath,NewModule)
	if InModuleName == "platform" then
		print("Func, Err ===", Func, Err)
	end
	print("Func ===========")
	print(type(Func))
	print(">>>>")
	if Func == nil then
		local Msg = string.format("error msg:path=%s\n%s\nStatckInfo=%s\n",InPath,tostring(Err),debug.traceback()) 
		if LOGE then
			LOGE(Msg)
		else
			print(Msg)
		end
		return nil
	end
	
	-- Func()
	
	if not isNew then
		__ImportedModule__[InModuleName] = NewModule
	end
	
	CallModuleInit(NewModule)
	
	-- 模块加载完回调
	OnModuleLoaded(NewModule)
	
	return NewModule
end

--不能直接用
function __InnerImportGlobal__(InModuleName,InPath)
	if InPath == nil or InModuleName == nil then
		LOGE("加载Lua脚本出错",InPath,InModuleName)
		return nil
	end
	
	local IsOK = __ImportedGlobalModule__[InModuleName]
	if IsOK then
		return nil
	end
	
	local Func,Err = __loadfile__(true,InPath)
	
	if Func == nil then
		local Msg = string.format("\nLoad script file(%s) failed\n error msg:%s\nStatckInfo:%s\n",InPath,tostring(Err),debug.traceback())
		assert(false,Msg)
		return nil
	end
	
	Func()

	__ImportedGlobalModule__[InModuleName] = true
	
	-- 模块加载完回调
	OnModuleLoaded(nil)
	
	-- 没有返回值
end

--get moudle name by path
function __GetModuleNameInner__(InPath)
	local ModuleName
	if SysFileSystem then
		ModuleName = SysFileSystem.GetLuaImportModuleName(InPath)
	else
		local dummyCount = 20000 -- 这个数够了吧
		InPath = string.gsub(InPath, "%.", "/")
		local last = nil
		local slash = string.find(InPath, "/")
		for i=1,dummyCount do
			if not slash then
				break
			end
			last = slash
			slash = string.find(InPath, "/", last + 1)
		end
		if last then
			ModuleName = string.sub(InPath, last + 1)
		else
			ModuleName = InPath
		end
	end
	return ModuleName
end


--Import module function
function Import(InPath)
	--get moudule name by name
	local ModuleName = __GetModuleNameInner__(InPath)
	--inner import module name 
	local Mod = __InnerImport__(ModuleName, InPath,false)
	--return module object
	if InPath == "common.platform" then
		print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>wocaonima,zheshishenme")
		print("modulename ===="..ModuleName)
		print("Mod ====="..tostring(Mod))
	end
	return Mod
end

-- reload用,其他情况不要用
function ImportNew(InPath)
	local ModuleName = __GetModuleNameInner__(InPath)
	return __InnerImport__(ModuleName,InPath,true)
end

function ImportGlobal(InPath)
	local ModuleName = __GetModuleNameInner__(InPath)
	__InnerImportGlobal__(ModuleName, InPath)
end

function GetModuleNameByPath(InPath)
	return __GetModuleNameInner__(InPath)
end

function CheckModule(InObj)
	if type(InObj) == "table" and InObj.__IsModule__ then
		return true
	else
		return false
	end
end

function ImportStringCode(InStringCode,InNewModuleEnv,InChunkName)
	local dummyName = "dummy"
	if InChunkName == nil then
		InChunkName = dummyName
	end
	
	if StringCodeDefaultEnv == nil then
		StringCodeDefaultEnv = GetNewModuleEnvTbl(dummyName,dummyName)
	end
	
	local NewModule
	if InNewModuleEnv then
		NewModule = InNewModuleEnv
	else
		NewModule = StringCodeDefaultEnv
	end
	
	return SysLoader.LoadStringScript(InChunkName,InStringCode,"bt",NewModule)
end
