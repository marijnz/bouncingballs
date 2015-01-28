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

        local velocity = self.go.rb:getLinearVelocity()
        local ball1Rotation = vec2Rotate(velocity, 90)
        local ball1RotationNormalized = ball1Rotation:normalized() 
        local ball2Rotation = vec2Rotate(velocity, -90)
        local ball2RotationNormalized = ball2Rotation:normalized() 
		
		local ball1 = objectManager:grab(SmallBall)
		ball1:setInitialPositionAndMovement(position + Vec3(ball1RotationNormalized.x * 0.76, ball1RotationNormalized.y * 0.76, 0), Vec3(ball1Rotation.x, ball1Rotation.y, 0.0))
		local ball2 = objectManager:grab(SmallBall)
		ball2:setInitialPositionAndMovement(position + Vec3(ball2RotationNormalized.x * 0.76, ball2RotationNormalized.y * 0.76, 0), Vec3(ball2Rotation.x, ball2Rotation.y, 0.0))
	
		objectManager:put(MediumBall, self)
	end

	Ball.update(self)
end
