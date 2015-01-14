logMessage("using BigBall.lua")

BigBall = {}
BigBall.__index = BigBall

setmetatable(BigBall, {
  __index = Ball, -- this is what makes the inheritance work
  __call = function (cls, ...)
    self = setmetatable({}, cls)
    return self
  end,
})

function BigBall:create()

	Ball.create(self, "yellowBall", 0.75)
	
end

function BigBall:update()

	local position = self.go:getPosition()

	if (self.hitBullet) then
		
		logMessage(self.go:getGuid().."dealing with hitBullet")
		
		self.hitBullet = false
		
		objectManager:put(BigBall, self)
		
		local ball1 = objectManager:grab(MediumBall)
		ball1:setInitialPositionAndMovement(position+Vec3(0,0.76,0), Vec3(0, 2.0, 0.0))
		local ball2 = objectManager:grab(MediumBall)
		ball2:setInitialPositionAndMovement(position+Vec3(0,-0.76,0), Vec3(0, -2.0, 0.0))
					
	end
	
	Ball.update(self)
end