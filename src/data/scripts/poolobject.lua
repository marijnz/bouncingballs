logMessage("using poolobject.lua")

PoolObject = {}
PoolObject.__index = PoolObject

setmetatable(PoolObject, {
  __call = function (cls, ...)
    local self = setmetatable({}, cls)
    self:_create(...)
    return self
  end,
})

-- Here you want to create the gameobject and components, as this won't be possible during runtime :(
-- i.e.:
-- go = GameObjectManager:createGameObject("enemy" .. self.uniqueIdentifier)
-- go.render = go:createRenderComponent()
-- self.go = go
function PoolObject:create()
    logMessage("CREATED".. self.uniqueIdentifier)
end

function PoolObject:initialize()
    
end

function PoolObject:dispose()
    
end

function PoolObject:update()
	
end

function PoolObject:doSomething()
    print( "Doing something" )
end

