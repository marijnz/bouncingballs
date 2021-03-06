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

function ObjectManager:_initialize()
	self.poolObjects = {}
	
	self.totalCount = 0
	
	go = GameObjectManager:createGameObject("objectManager")
	
	go.sc = go:createScriptComponent()
	go.sc:setUpdateFunction(self.update)
	self.go = go
end


function ObjectManager:getActiveFromPool(baseTypeArray) 
 local activeObjects={}
	for i, baseType in pairs(baseTypeArray) do
		for k, activeObject in pairs(self.poolObjects[baseType]["active"]) do
			if activeObject~=nil then 
				activeObjects[k] = activeObject 
				logMessage(activeObject.go:getGuid().."gottenFromPool")
			end
		end
	end
return activeObjects
end


function ObjectManager:addPool(baseType, amount)
	logMessage("Adding pool to PoolSystem! "..amount)
	self.poolObjects[baseType] = {}
	self.poolObjects[baseType]["count"] = amount
	
	self.poolObjects[baseType]["passive"] = {}
	self.poolObjects[baseType]["active"] = {}
	
	for i=1,amount do
		newPoolObject = baseType()
		
		newPoolObject.uniqueIdentifier = self.totalCount
		
		newPoolObject:create()
		
		self.totalCount = self.totalCount + 1
		
		self.poolObjects[baseType]["passive"][i] = newPoolObject
		
	end
end

function ObjectManager:update(deltaTime)
	self = objectManager -- self is not properly set because update is called from engine
	
	for k1, pool in pairs(self.poolObjects) do
		for k2, poolObject in pairs(pool["active"]) do
				poolObject:update(deltaTime)
		end
	end
	return EventResult.Handled
end


function ObjectManager:grab(baseType)
	count = self.poolObjects[baseType]["count"]
		
	poolObject = nil
	-- Grab from passive pool
	for i=0,count do
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
			logMessage(poolObject, "went active")
			break
		end
	end
	
	poolObject:initialize()
	return poolObject
end

function ObjectManager:put(baseType, poolObject)
	count = self.poolObjects[baseType]["count"]
	poolObject:dispose()
	
	-- Remove from active pool
	for i=0,count do
		tempPoolObject = self.poolObjects[baseType]["active"][i]
		local foundObject = tempPoolObject ~= nil and tempPoolObject.uniqueIdentifier == poolObject.uniqueIdentifier
		if(foundObject) then
			self.poolObjects[baseType]["active"][i] = nil
			break
		end
	end
	
	-- Put in passive
	for i=0,count do
		tempPoolObject = self.poolObjects[baseType]["passive"][i]
		if(tempPoolObject == nil) then
			self.poolObjects[baseType]["passive"][i] = poolObject
			break
		end
	end
end


objectManager = ObjectManager()
