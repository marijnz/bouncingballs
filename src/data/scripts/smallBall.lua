logMessage("using SmallBall.lua")

SmallBall = {}
SmallBall.__index = SmallBall

setmetatable(SmallBall, {
  __index = Ball, -- this is what makes the inheritance work
  __call = function (cls, ...)
    self = setmetatable({}, cls)
    return self
  end,
})

function SmallBall:create()

	Ball.create(self, "blueBall", 0.25)
	
end

function SmallBall:update()

	local position = self.go:getPosition()

	if (self.hitBullet) then
		self.hitBullet = false
		objectManager:put(SmallBall, self)
	end
	
	Ball.update(self)
end