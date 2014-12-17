logMessage("using poolexample.lua")

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

function PoolExampleObject:create()
    logMessage("Created PoolExampleObject ".. self.uniqueIdentifier)
end

function PoolExampleObject:initialize()
     logMessage("Initializing PoolExampleObject ".. self.uniqueIdentifier)
end

function PoolExampleObject:dispose()
    logMessage("Disposing PoolExampleObject ".. self.uniqueIdentifier)
end

function PoolExampleObject:update()
	logMessage("Updating PoolExampleObject ".. self.uniqueIdentifier)
end