PoolObject = {}
PoolObject.__index = PoolObject

setmetatable(PoolObject, {
  __call = function (cls, ...)
    local self = setmetatable({}, cls)
    self:_initialize(...)
    return self
  end,
})

-- Here you want to create the gameobject and components, as this won't be possible during runtime :(
-- i.e.:
-- go = GameObjectManager:createGameObject("enemy")
-- go.render = go:createRenderComponent()
-- self.go = go
function PoolObject:_initialize()
    
end

function PoolObject:update()
-- TODO
end

function PoolObject:doSomething()
    print( "Doing something" )
end

