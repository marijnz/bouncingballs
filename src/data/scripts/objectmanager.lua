logMessage("using objectmanager.lua")

-- Singleton is declared at the bottom of this file

ObjectManager = {}
ObjectManager.__index = ObjectManager

setmetatable(ObjectManager, {
  __call = function (cls, ...)
    local self = setmetatable({}, cls)
    self:_initialize()
    return self
  end,
})

function ObjectManager:_initialize(baseType)
	self.poolObjects = {}
	
	Events.Update:registerListener(self.update)
end

function ObjectManager:addPool(baseType, amount)
	logMessage("Adding pool to PoolSystem!")
	self.poolObjects[baseType] = {}
	self.poolObjects[baseType]["count"] = amount
	
	self.poolObjects[baseType]["passive"] = {}
	self.poolObjects[baseType]["active"] = {}
	
	for i=0,amount do
		newPoolObject = baseType()
		
		-- Unique identifier doesn't say anything about position in array later on!
		newPoolObject.uniqueIdentifier = i
		
		newPoolObject:create()
		
		self.poolObjects[baseType]["passive"][i] = newPoolObject
		
	end
end

function ObjectManager:update()
	self = objectManager -- self is not properly set because update is called from engine
	logMessage("Updating")
	for k1, pool in pairs(self.poolObjects) do
		for k2, poolObject in pairs(pool["active"]) do
			if(poolObject ~= nil) then
				poolObject:update()
			end
		end
	end
	return EventResult.Handled
end


function ObjectManager:grab(baseType)
	count = self.poolObjects[baseType]["count"]-1
	poolObject = nil
	logMessage(count)
	-- Grab from passive pool
	for i=0,count do
	
	logMessage("OK")
		poolObject = self.poolObjects[baseType]["passive"][i]
		if(poolObject ~= nil) then
			self.poolObjects[baseType]["passive"][i] = nil
			break
		end
	end
	
	if(poolObject == nil) then
		logMessage("Error: ObjectManager depleted!")
		return
	end
	
	-- Put in active
	for i=0,count do
		tempPoolObject = self.poolObjects[baseType]["active"][i]
		if(tempPoolObject == nil) then
			self.poolObjects[baseType]["active"][i] = poolObject
			break
		end
	end
	
	poolObject:initialize()
	return poolObject
end

function ObjectManager:put(baseType, poolObject)
	count = self.poolObjects[baseType]["count"]-1
	poolObject:dispose()
	
	-- Grab from active pool
	for i=0,count do
		tempPoolObject = self.poolObjects[baseType]["active"][i]
		if( tempPoolObject ~= nil and 
			tempPoolObject.uniqueIdentifier == poolObject.uniqueIdentifier) then
			
			self.poolObjects[baseType]["active"][i] = nil
		end
	end
	
	-- Put in passive
	for i=0,count do
		tempPoolObject = self.poolObjects[baseType]["passive"][i]
		if(tempPoolObject == nil) then
			self.poolObjects[baseType]["passive"][i] = poolObject
		end
	end
end


objectManager = ObjectManager()