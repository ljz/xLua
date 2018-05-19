local tbDefineValue = {}
setmetatable(_G, {
		__index = function(tb, key)
			if not tbDefineValue[key] then
				error(string.format("can not find global：%s", key))
				return nil
			end
			rawget(tab, key)
		end,
		_newindex = function(tb, key, value)
			tbDefineValue[key] = true
			rawset(tb, key, value)
		end
		}
	)

function disableWrite_G()
	setmetatable(_G, {
		__newindex = function(tb, key, value)
			error(string.format("can not define global value：%s in local space", key))
			do return end
		end,
	})
end

c = 12
print(c)

function test2()
	d = 12
	c = 13
end

function test3()
	print(c)
	if a == 12 then
		print(123)
	end 
end

function start()
	disableWrite_G()
	test2()
	test3()
end

start()