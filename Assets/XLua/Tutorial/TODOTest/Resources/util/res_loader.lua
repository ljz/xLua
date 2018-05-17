require("util.mono")
--[[
    用于管理相同生命周期的资源。
    用于缓解Unity从Assetbundle中加载的资源难以追踪其引用的问题。
    应用场景如下：
    
        我们加载出来的资源，最终会被某个GameObject上去，而这个GameObject在Destroy
        时，是一个非常好的清理资源的时机。这些资源的生存期，其实就与GameObject的
        生命期绑定一致。
        
        也可以根据具体应用场景来创建ResLoader，实现不同粒度的资源管理。
        
    ResLoader不提供卸载单个资源的接口，因为这个类只用于实现比较粗放的资源管理。
    
    如果需要更加精细化的管理，可以直接使用ResMgr的接口。
--]]
local super = CMono
CResLoader = Class("CResLoader", super)

function CResLoader:CResLoader()
    self._loaded = {}
    self._resMgr = self:_GetResMgr()
end

function CResLoader:Release()
    self:UnloadAll()
    self._loaded = nil
    self._resMgr = nil
    super.Release(self)
end

function CResLoader:UnloadAll()
    for k, v in pairs(self._loaded) do
        -- self._resMgr:UnloadAsset(v)
    end
    self._loaded = {}
end

function CResLoader:GetTickObj()
    -- return GetApp():GetTickObj()
end

--[[
    加载预加载资源：先调用BeginPreloadFromConfig
--]]
function CResLoader:BeginPreloadFromConfig(setting)
    -- if setting == nil then
    --     return
    -- end
    -- -- 异步加载资源
    -- for _, res in pairs(setting.AsyncRes) do
    --     if type(res) == "string" then
    --         self:LoadAssetAsync(res)
    --     else
    --         self:LoadAssetAsync(res[1], CS.System.Type.GetType(res[2]))
    --     end
    -- end
    -- -- 同步加载资源，等到需要用的时候调用EndPreloadFromConfig
    -- for _, res in pairs(setting.SyncRes) do
    --     if type(res) == "string" then
    --         self:LoadAssetAsync(res)
    --     else
    --         self:LoadAssetAsync(res[1], res[2])
    --     end
    -- end
end

function CResLoader:EndPreloadFromConfig(setting)
    -- for _, res in pairs(setting.SyncRes) do
    --     if type(res) == "string" then
    --         self:LoadAsset(res)
    --     else
    --         self:LoadAsset(res[1], res[2])
    --     end
    -- end    
end

--[[
    同步加载资源
--]]
function CResLoader:LoadAsset(assetPath, utype)
    -- local asset = self:_FindAsset(assetPath, utype)
    -- if asset == nil then
    --     asset = self:_GetResMgr():LoadAsset(assetPath, utype)
    --     self:_AddAsset(assetPath, asset)
    -- end
    -- return asset
end

--[[
    异步加载资源
--]]
function CResLoader:LoadAssetAsync(assetPath, utype, callback)
    -- local asset = self:_FindAsset(assetPath, utype)
    
    -- local resMgr = self._resMgr
    -- if asset ~= nil then    -- 已经加载过了，仍然保证异步返回
    --     if callback ~= nil then -- 不需要回调的，就不需要异步了
    --         local function onAssetLoaded()
    --             callback(asset)
    --         end            
    --         self:Invoke(onAssetLoaded, 0)
    --     end
    -- else
    --     local function onAssetLoaded(assetLoaded)
    --         if self._resMgr == nil then -- ResLoader is already destroyed!
    --             resMgr:UnloadAsset(assetLoaded)
    --         else
    --             self:_AddAsset(assetPath, assetLoaded)
    --             if callback then
    --                 callback(assetLoaded)
    --             end 
    --         end
    --     end        
    --     resMgr:LoadAssetAsync(assetPath, utype, onAssetLoaded)
    -- end
end

function CResLoader:_GetResMgr()
    -- return GetApp():GetResMgr()
end

function CResLoader:_MakeResKey(assetPath, utype)
    -- if utype == nil then
    --     return assetPath
    -- elseif type(utype) == "string" then
    --     return assetPath .. ":" .. utype
    -- else
    --     return assetPath .. ":" .. utype.FullName
    -- end
end

function CResLoader:_FindAsset(assetPath, utype)
    -- local k = self:_MakeResKey(assetPath, utype)
    -- return self._loaded[k]
end

function CResLoader:_AddAsset(assetPath, asset)
    -- local k = self:_MakeResKey(assetPath, utype)
    -- local res = self._loaded[k]
    -- if res == nil then
    --     self._loaded[k] = asset
    -- else
    --     self._resMgr:UnloadAsset(asset)
    -- end
end