logMessage("using camera.lua")

Tweener = {}
Tweener.__index = Tweener

setmetatable(Tweener, {
  __index = PoolObject, -- this is what makes the inheritance work
  __call = function (cls, ...)
    self = setmetatable({}, cls)
    return self
  end,
})

function Tweener:create()
	
end

function Tweener:startTween(easeType, transform, toPosition, timeToTake, onComplete)
	if(self.isMoving) then
		return
	end
	self.easeType = easeType
	self.fromPosition = transform:getPosition()
	self.transform = transform
	self.toPosition = toPosition
	self.timeSinceMoving = 0
	self.timeToTake = timeToTake
	self.isMoving = true
	self.onComplete = onComplete
end

function Tweener:update(deltaTime)
	if(self.isMoving) then
		self.timeSinceMoving = self.timeSinceMoving + deltaTime
		if(self.timeSinceMoving >= self.timeToTake) then self.timeSinceMoving = self.timeToTake end
		to = easingVec3(self.easeType, self.timeSinceMoving,self.fromPosition,self.toPosition - self.fromPosition, self.timeToTake)
		
		self.transform:setPosition(to)
		
		if(self.timeSinceMoving >= self.timeToTake) then
			self.isMoving = false
			objectManager:put(Tweener, self)
			self.onComplete()
		end
	end
end

function Tweener:initialize() 
	self.isMoving = false
end

function Tweener:dispose() end




