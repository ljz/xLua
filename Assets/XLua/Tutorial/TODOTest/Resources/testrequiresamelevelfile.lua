--测试创建其他文件的类的对象 以及 继承的问题
CTestRequireSameLevelClass = Class("CTestRequireSameLevelClass")


function CTestRequireSameLevelClass:test( )
	print("CTestClass:test( ):success")
end


function CTestRequireSameLevelClass:tests2()
	-- print(">>>>>>>>>>>>>>>>>>>>testss222")
end


CSubClass = Class("CSubClass", CTestRequireSameLevelClass)


function CSubClass:test()
 	print("in sub.test")
end