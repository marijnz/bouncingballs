logMessage("using MediumBall.lua")

MediumBall = {}
MediumBall.__index = MediumBall

setmetatable(MediumBall, {
  __index = Ball, -- this is what makes the inheritance work
  __call = function (cls, ...)
    self = setmetatable({}, cls)
    return self
  end,
})

function MediumBall:create()
	Ball.create(self, "redBall", 0.5)
end

function MediumBall:update()
	
	local position = self.go:getPosition()

    local lastCollisionData = self.go.rb:getUserData()
	
	if (lastCollisionData.gotHit) then
		
		logMessage(self.go:getGuid().."dealing with hitHookshot")
		
		objectManager:put(MediumBall, self)
		
		local ball1 = objectManager:grab(SmallBall)
		ball1:setInitialPositionAndMovement(position+Vec3(0,0.76,0), Vec3(0, 2.0, 0.0))
		local ball2 = objectManager:grab(SmallBall)
		ball2:setInitialPositionAndMovement(position+Vec3(0,-0.76,0), Vec3(0, -2.0, 0.0))
	end

	Ball.update(self)
end
