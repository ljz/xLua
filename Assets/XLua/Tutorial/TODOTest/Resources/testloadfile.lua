--这里可以require一系列的lua文件
--require和 Import 是什麽关系呢？

require("test2")
require("test3")
require("sub.test4")

print("int testloadfile")
	

print(">>>>> this is in lua")


local testobj = CS.UnityEngine.GameObject("testobj")
print(testobj)



