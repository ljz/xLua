--弱引用
local weakref_meta = {
  __mode = "v",
  __call = function (self)
    return self.obj
  end
}

function weakref(o)
  local ref = {obj = o}
  setmetatable(ref, weakref_meta)
  return ref
end

