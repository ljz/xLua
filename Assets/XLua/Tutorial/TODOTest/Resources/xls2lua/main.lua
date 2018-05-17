--lfs.dll:lua file system
-- require("lfs")

--luaiconv.dll:window gbk to utf 8


--protected  call 
pcall(require, "luaiconv")

-- search c path/dll path 
package.cpath = package.cpath .. ";./bin/?.dll"

--current script path
-- lfs = {}
-- lsf.currentdir = function()
-- 	return  "hsdajkf"
-- end


SCRIPT_PATH = "/Users/apple/Desktop/unity学习文件夹/xlua/Assets/XLua/Tutorial/TODOTest/Resources/xls2lua/main.lua"--lfs.currentdir()
-- lfs.chdir("..")--go to up dir 


--remove temp file
local REMOVE_TMP_FILE = true


--find file by path
-- replace '.'  to '/' and add '.lua'
-- "a.b.c.d"->"a/b/c/d.lua"
function FindFile(self, path)
    return string.gsub(path, "%.", "/") .. ".lua"
end

function log(...)
    local info = debug.getinfo(2, "Sl")
    local params = {...}
    local msg
    if "string" == type(params[1]) and string.find(params[1], "%%") then
        msg = string.format(...)
        print(string.format("L%d%s", info.currentline, info.source))
        print(msg)
    else
        io.write(string.format("L%d%s", info.currentline, info.source))
        print(...)
    end
end

_G.ToolFindFile = FindFile
_G["LOG"] = log
_G["SysLoader"] = {LoadScript = loadfile}
_G["IsPetTool"] = true


--loadfile:load & build file, but not run file 
--dofile:run file 
--require:run only once,if required while do nothing.
dofile("classdefine.lua")
dofile("module.lua")



Import "globalcommon.setting.excel.xls2lua_config"
Import "globalcommon.util.lua_util"

-- 全局变量
local CFG = xls2lua_config.gXlsFiles
--lfs.currentdir()
local CWD = "/Users/apple/Desktop/unity学习文件夹/xlua/Assets/XLua/Tutorial/TODOTest/Resources/xls2lua/main.lua"
local DEFAULTS = {["INT"] = 1, ["FLOAT"] = 1.0, ["STRING"] = "", ["TABLE"] = "{}"}
local ALL_XLS = {}

function Convert(from, to)
	return from
    -- if iconv == nil then
    --     return function(txt)
    --         return txt
    --     end
    -- else
    --     local cd = iconv.new(to, from)
    --     return function (txt)
    --         local nstr, err = cd:iconv(txt)
    --         if err == nil then
    --             return nstr
    --         else
    --             return txt
    --         end
    --     end
    -- end
end

UTF2GBK = Convert("UTF-8", "GBK")

function FileExists(path)
    local fp = io.open(path, "r")
    if fp then
        fp:close()
        return true
    else
        return false
    end
end

function GetXLSPath(path)
    return CWD .. "/globalcommon/setting/excel/" .. path
end

function GetFileName(path)
    local last = nil
    local slash = string.find(path, "/")
    while slash do
        last = slash
        slash = string.find(path, "/", last + 1)
    end
    if last then
        return string.sub(path, last + 1)
    else
        return path
    end
end

function GetExtension(filename)
    local last = nil
    local dot = string.find(filename, "%.")
    while dot do
        last = dot
        dot = string.find(filename, "%.", dot + 1)
    end
    if last then
        return string.sub(filename, 1, last - 1), string.sub(filename, last + 1)
    end
    return filename, ""
end

function WriteHeader(fp, filename, desc)
    fp:write(string.format("-- File: %s\n", filename))
    fp:write(string.format("-- Desc: %s\n", desc))
end

function WriteFunction(fp, clsName, functionName, args, content)
    if args then
        fp:write(string.format("function %s:%s(%s)\n", clsName, functionName, args))
    else
        fp:write(string.format("function %s:%s()\n", clsName, functionName))
    end
    fp:write(string.format("%s\n", content))
    fp:write("end\n\n")
end

function GenCfgRowCls(path, cfgFileName, cfgClsName, idName)
    local fp = io.open(path, "w+")
    assert(fp)
    WriteHeader(fp, cfgFileName, string.format("Config class for config file: %s", string.gsub(cfgFileName, "cls%.lua", "cfg%.lua")))
    fp:write(string.format('%s = Class("%s")\n\n', cfgClsName, cfgClsName))
    local content = "\t-- 导表依赖此函数，不要删除\n"..
                    "\t-- sheet.LUA_DATA 中的数据会被写到最终的配置表\n" ..
                    "\t-- sheet.EXPORT_COLS[i] 标识某一列是否需要导出到配置表，默认第一列不导出\n" ..
                    "\treturn true"
    WriteFunction(fp, cfgClsName, "PreProcess", "xls, sheet, rowData", content)
    fp:write(string.format("--function %s:PostProcess(xls, sheet, rowData)\n", cfgClsName))
    content = "\t-- 可以在此函数对 sheet.LUA_DATA (最终会导出此表) 中的数据做一些合法性检查\n" ..
              "\t-- return true\n" ..
              "--end\n"
    fp:write(content)
    fp:flush()
    fp:close()
    fp = nil
end

function CheckCfgCls(cfgClsFileName, cfgClsName, idName)
    local path = string.format("globalcommon/settingcls/%s", cfgClsFileName)
    local cfgClsFileNameWithoutExt, ext = GetExtension(cfgClsFileName)
    local modPath = string.format("globalcommon.settingcls.%s", cfgClsFileNameWithoutExt)
    if FileExists(path) then
        cfgRowCls = lua_util.GetClsByClsStr(modPath, cfgClsName)
        if cfgRowCls and cfgRowCls.PreProcess then
            return cfgRowCls
        end
    end
    GenCfgRowCls(path, cfgClsFileName, cfgClsName, idName)
    return lua_util.GetClsByClsStr(modPath, cfgClsName)
end

function GenCfgFilename(xlsName, sheetName)
    return string.format("%s_%s_cfg.lua", xlsName, sheetName)
end

function GenCfgClsFilename(xlsName, sheetName)
    return string.format("%s_%s_cls.lua", xlsName, sheetName)
end

function GenCfgClsName(xlsName, sheetName)
    return string.format("C%s_%s", xlsName, sheetName)
end

function WriteValue(fp, v, append)
    local stype = type(v)
    if "number" == stype then
        fp:write(v)
    elseif "string" == stype then
        fp:write(string.format("%q", v))
    elseif "table" == stype then
        if IsArray(v) then
            WriteArray(fp, v)
        else
            WriteTable(fp, v)
        end
    end
    if type(append) == "string" then
        fp:write(append)
    end
end

function IsArray(tbl)
    if "table" ~= type(tbl) then
        return false
    end
    local cnt = 0
    for k, v in pairs(tbl) do
        cnt = cnt + 1
    end
    local icnt = 0
    for k, v in ipairs(tbl) do
        icnt = icnt + 1
    end
    return cnt == icnt
end

function WriteTable(fp, tbl)
    fp:write("{")
    for k, v in pairs(tbl) do
        fp:write(string.format("%s=", FormatTableKey(k)))
        WriteValue(fp, v, ", ")
    end
    fp:write("}")
end

function WriteArray(fp, tbl)
    fp:write("{")
    for i, value in ipairs(tbl) do
        WriteValue(fp, value, ", ")
    end
    fp:write("}")
end

function FormatTableKey(k)
    if type(k) == "number" then
        return string.format("[%s]", k)
    elseif type(k) == "string" then
        if string.match(k, "^[%a_][%w_]*$") then
            return k
        else
            return string.format("[%q]", k)
        end
    else
        assert(false, string.format("Unsupported table key type=%s, please contact guokun", type(k)))
    end
end

function ProcessSingleRow(key, cls, sheet, rowData)
    local rowInst = cls()
    if nil == sheet.ROW_INST then
        sheet.ROW_INST = {}
    end
    sheet.ROW_INST[key] = rowInst
    return rowInst:PreProcess(xls, sheet, rowData)
end

function ProcessSingleSheet(xls, sheet)
    local xlsName = xls.XLS_NAME
    local sheetName = sheet.NAME
    -- 配置类文件名(cfgClsFileName)：xlsName_sheetName_cfg_cls.lua
    -- 配置类名(cfgClsName): CxlsName_sheetName
    local cfgClsFileName = string.lower(GenCfgClsFilename(xlsName, sheetName))
    local cfgClsName = GenCfgClsName(xlsName, sheetName)
    -- 检查对应的配置文件类是否存在，不存在则创建
    cfgRowCls = CheckCfgCls(cfgClsFileName, cfgClsName, sheet.LOGICNAME[1])
    if nil == cfgRowCls then
        error(string.format("Check row cls fail! excel=%s, sheet=%s", xls.XLS_NAME, sheetName))
        return false
    end
    for i, rowData in pairs(sheet.DATA) do -- 不能用 ipairs，估计需要加个排序
        if not ProcessSingleRow(i, cfgRowCls, sheet, rowData) then
            return false
        end
    end
    return true
end

function ErrorAtCell(sheet, rowKey, columnIndex, errmsg)
    error(UTF2GBK(string.format("Export ERROR! Sheet[%s], Row_ID=%s, Col=%d, \nError: %s", sheet.NAME, tostring(rowKey), columnIndex, errmsg)))
end

function ProtectedCheck(check, v, sheet, rowKey, columnIndex)
    local bOK, b, errmsg = pcall(check, v)
    if not bOK then
        ErrorAtCell(sheet, rowKey, columnIndex, b)
    else
        if b then
            return errmsg
        else
            ErrorAtCell(sheet, rowKey, columnIndex, errmsg)
        end
    end
end

function TypeCheck(sheet)
    -- local checks = {}
    -- for _, typeName in ipairs(sheet.TYPE) do
    --     local checker = Import("xls2lua.typecheck.check" .. typeName)
    --     table.insert(checks, checker["check"..typeName])
    -- end
    -- DATA = {}
    -- for key, rowData in pairs(sheet.DATA) do
    --     local row = {}
    --     local key_chk = ProtectedCheck(checks[1], key, sheet, key, 1)
    --     DATA[key_chk] = row
    --     for i, cellData in ipairs(rowData) do
    --         local v_chk = ProtectedCheck(checks[i], cellData, sheet, key, i)
    --         table.insert(row, v_chk)
    --     end
    -- end
    -- sheet.DATA = DATA
end

function ProcessSingleXls(xls)
    io.write("\t     Sheets: ")
    local sheetNames = {}
    for sheetName, sheet in pairs(xls.SHEETS) do
        sheetNames[#sheetNames + 1] = sheetName
        io.write(string.format("%s | ", sheetName))
    end
    io.write("\n\t     Result: ")
    for sheetName, sheet in pairs(xls.SHEETS) do
        local fmt = string.format("%%%ds | ", string.len(sheetName))
        if not ProcessSingleSheet(xls, sheet) then
            io.write(string.format(fmt, "Fail"))
            io.write(string.format("\tOops!!! sheet=%s\n", sheetName))
            return false
        else
            io.write(string.format(fmt, "OK"))
        end
    end
    io.write("\n")
    return true
end

function PostProcessSingleSheet(xls, sheet)
    if nil == sheet.LUA_DATA then
        sheet.LUA_DATA = sheet.DATA
    end

    for key, rowData in pairs(sheet.LUA_DATA) do -- 不能用 ipairs，估计需要加个排序
        rowInst = sheet.ROW_INST[key]
        if rowInst.PostProcess and not rowInst:PostProcess(xls, sheet, rowData) then
            return false
        end
    end

    local xlsName = xls.XLS_NAME
    local sheetName = sheet.NAME
    -- 配置文件名(cfgFileName): xlsName_sheetName_cfg.lua
    local cfgFileName = GenCfgFilename(xlsName, sheetName)
    local cfgFilePath = string.lower(string.format("globalcommon/setting/%s", cfgFileName))
    local fp = io.open(cfgFilePath, "w")
    assert(fp, string.format("Fail to pen file to write! Filename=%s", cfgFilePath))
    fileNameWithoutExt, ext = GetExtension(cfgFileName)
    local desc = string.format("Xls: %s, Sheet: %s 配置", xlsName, sheetName)

    WriteHeader(fp, cfgFileName, desc)
    fp:write(string.format('__GLOBALCONFIGTABLE__["%s_%s"] = {\n', xlsName, sheetName))
    
    -- ENUMERATE
    fp:write("\t__ENUMERATE__ =  {\n")
    local cnt = 1
    for i, field in ipairs(sheet.LOGICNAME) do
        if sheet.EXPORT_COLS[i] then
            fp:write(string.format("\t\t%s = %d,\n", field, cnt))
            cnt = cnt + 1
        end
    end
    fp:write("\t},\n")
    
    -- ALL_KEYS
    local all_keys = {}
    for key, rowData in pairs(sheet.LUA_DATA) do
        table.insert(all_keys, key)
    end
    table.sort(all_keys)
    fp:write("\t__ALL_KEYS__ = ")
    WriteArray(fp, all_keys)
    fp:write(",\n")
    
    local keys = {}
    for k, row in pairs(sheet.LUA_DATA) do
        keys[#keys + 1] = row[1]
    end
    table.sort(keys)
    for i, k in ipairs(keys) do
        local row = sheet.LUA_DATA[k]
        fp:write(string.format("\t%s = {", FormatTableKey(row[1])))
        for i, v in ipairs(row) do
            if sheet.EXPORT_COLS[i] then
                WriteValue(fp, v, ", ")
            end
        end
        fp:write("},\n")
    end
    fp:write("}\n")
    fp:flush()
    io.close(fp)
    fp = nil
    return true
end

function PostProcessSingleXls(xls)
    io.write("\t     Sheets: ")
    local sheetNames = {}
    for sheetName, sheet in pairs(xls.SHEETS) do
        sheetNames[#sheetNames + 1] = sheetName
        io.write(string.format("%s | ", sheetName))
    end
    io.write("\n\t     Result: ")
    for sheetName, sheet in pairs(xls.SHEETS) do
        local fmt = string.format("%%%ds | ", string.len(sheetName))
        if not PostProcessSingleSheet(xls, sheet) then
            io.write(string.format(fmt, "Fail"))
            io.write(string.format("\nOops!!! sheet=%s\n", sheetName))
            return false
        else
            io.write(string.format(fmt, "OK"))
        end
    end
    io.write("\n")
    return true
end

function PreProcess()
    -- xls 导成中间数据
    -- 根据 xls 名字存储在 _G 中
    local executable = string.format("%s/python.exe %s/xlsreader.py", SCRIPT_PATH, SCRIPT_PATH)
    --lfs.currentdir())
    local xlsPath = string.format("%s/globalcommon/setting/excel", "/Users/apple/Desktop/unity学习文件夹/xlua/Assets/XLua/Tutorial/TODOTest/Resources/xls2lua/main.lua")
    local startTime = os.time()
    local succCnt = 0
    local failCnt = 0
    print(string.format("> [%s] Start preprocessing ...", os.date("%H:%M:%S", startTime)))
    for _, path in pairs(CFG) do
        local xlsFilename = GetFileName(path)
        local xlsName, ext = GetExtension(xlsFilename)
        xlsName = string.lower(xlsName)
        local luaFile = string.format("%s/%s.lua", xlsPath, xlsName)
        local cmd = string.format("%s %s/%s %s", executable, xlsPath, path, luaFile)
        io.write(string.format("\t - preprocessing %-15s ", xlsFilename))
        local ret = os.execute(cmd)
        if 0 == ret then
            local modPath = string.format("globalcommon.setting.excel.%s", xlsName)
            Import(modPath) -- 导入到 _G 里面
            
            for sheetName, sheet in pairs(_G[xlsName].SHEETS) do
                TypeCheck(sheet)
            end
            
            succCnt = succCnt + 1
            ALL_XLS[succCnt] = xlsName -- 保存一个所有配置的列表
            io.write(string.format("\tSucc!\n"))
        else
            failCnt = failCnt + 1
            io.write(string.format("\tFail!\n"))
        end
        if REMOVE_TMP_FILE and FileExists(luaFile) then
            os.remove(luaFile)
        end
    end
    local stopTime = os.time()
    print(string.format("> [%s] Finish preprocessing %d excels, succ = %d, fail = %d", os.date("%H:%M:%S", stopTime), succCnt + failCnt, succCnt, failCnt))
    return 0 == failCnt
end

function Process()
    -- 根据中间数据处理成需要的格式
    local startTime = os.time()
    print(string.format("\n> [%s] Start processing ...", os.date("%H:%M:%S", startTime)))
    for _, xlsName in pairs(ALL_XLS) do
        print(string.format("\t - processing %s", xlsName))
        if not ProcessSingleXls(_G[xlsName]) then
            return false
        end
    end
    local stopTime = os.time()
    print(string.format("> [%s] Finish processing", os.date("%H:%M:%S", stopTime)))
    return true
end

function PostProcess()
    -- 将处理结果写到文件
    local startTime = os.time()
    print(string.format("\n> [%s] Start postprocessing ...", os.date("%H:%M:%S", startTime)))
    for _, xlsName in pairs(ALL_XLS) do
        print(string.format("\t - processing %s", xlsName))
        if not PostProcessSingleXls(_G[xlsName]) then
            return false
        end
    end
    local stopTime = os.time()
    print(string.format("> [%s] Finish postprocessing", os.date("%H:%M:%S", stopTime)))
    return true
end

function main()
    -- print(arg) -- 命令行参数
    local startTime = os.time()
    local xlsCnt = 0
    local totalSheetCnt = 0
    if PreProcess() and Process() and PostProcess() then
    	-- lfs.currentdir())
        local xlsPath = string.format("%s/globalcommon/setting/excel", "/Users/apple/Desktop/unity学习文件夹/xlua/Assets/XLua/Tutorial/TODOTest/Resources/xls2lua/main.lua")
        local cmd = string.format("%s/python %s/gencfglist.py %s/.. .lua preloadsetting.lua", SCRIPT_PATH, SCRIPT_PATH, xlsPath)
        os.execute(cmd)
    end
end

main()