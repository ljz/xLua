--这里可以require一系列的lua文件
--require和 Import 是什麽关系呢？
package.path = package.path .. ';..\\ui\\?.lua'
-- print(package.path)
-- print("233333333333333")
-- print("int importlist.lua")
require("classdefine")
-- print(">>>>>>>>>>>>>>>>>>>>>in importlist")
