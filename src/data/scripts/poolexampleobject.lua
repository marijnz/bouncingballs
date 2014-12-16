PoolExampleObject = {}
PoolExampleObject.__index = PoolExampleObject

setmetatable(PoolExampleObject, {
  __index = PoolObject, -- this is what makes the inheritance work
  __call = function (cls, ...)
    local self = setmetatable({}, cls)
    return self
  end,
})

function PoolExampleObject:exampleDoSomething()
	PoolObject.doSomething()
end

function PoolExampleObject:update()
-- TODO
end