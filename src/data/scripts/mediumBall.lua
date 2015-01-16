logMessage("using MediumBall.lua")

MediumBall = {}
MediumBall.__index = MediumBall

setmetatable(MediumBall, {
  __index = ball, -- this is what makes the inheritance work
  __call = function (cls, ...)
    self = setmetatable({}, cls)
    return self
  end,
})

function MediumBall:create()

	ball.create(self, "redBall", 0.5)
	
end

function MediumBall:update()
	
	local position = self.go:getPosition()
	
	if (self.hitHookshot) then
		
		logMessage(self.go:getGuid().."dealing with hitHookshot")
		
		self.hitHookshot = false
		
		objectManager:put(MediumBall, self)
		
		local ball1 = objectManager:grab(SmallBall)
		ball1:setInitialPositionAndMovement(position+Vec3(0,0.76,0), Vec3(0, 2.0, 0.0))
		local ball2 = objectManager:grab(SmallBall)
		ball2:setInitialPositionAndMovement(position+Vec3(0,-0.76,0), Vec3(0, -2.0, 0.0))
					
	end

	ball.update(self)
	
end