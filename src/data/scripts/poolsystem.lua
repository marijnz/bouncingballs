PoolSystem = {}
PoolSystem_mt = { __index = PoolSystem }

setmetatable(PoolSystem, {
  __call = function (cls, ...)
    local self = setmetatable({}, cls)
   --self:_initialize()
    return self
  end,
})

function PoolSystem:_initialize()


end

function PoolSystem:getBasePoolObject()
	poolObject = self._createObject(PoolObject)
	-- Add object to pool and stuff
	return poolObject
end


function PoolSystem:_createObject(baseObject)
    -- The following lines are equivalent to the SimpleClass example:

    -- Create the table and metatable representing the class.
    local new_class = {}
    local class_mt = { __index = new_class }

    -- Note that this function uses class_mt as an upvalue, so every instance
    -- of the class will share the same metatable.
    --
    function new_class:create()
        local newinst = {}
        setmetatable( newinst, class_mt )
        return newinst
    end

    -- The following is the key to implementing inheritance:

    -- The __index member of the new class's metatable references the
    -- base class.  This implies that all methods of the base class will
    -- be exposed to the sub-class, and that the sub-class can override
    -- any of these methods.
    --
    if baseClass then
        setmetatable( new_class, { __index = baseClass } )
    end

    return new_class
end