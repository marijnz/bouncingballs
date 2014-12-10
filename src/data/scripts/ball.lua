
logMessage("using ball.lua")

local ball = {}
ball.__index = ball

setmetatable(ball, {__index = GameObjectManager:createGameObject("ball"),
	__call = function (cls, ...)
		local self = setmetatable({}, cls)
		self:_init(...)
		return self
	end,
})

function ball:_init(hp)
	local render = ball:createRenderComponent()
	render:setPath("data/models/ball.thModel")
	ball.sc = ball:createScriptComponent()
	ball.sc:setUpdateFunction(ball:update)
	ball.speed = 0.05
	ball:setPosition(Vec3(-3,-3, 0))
	ball.direction = Vec3(1,1,0)
	ball.hitpoints = hp
	
end

ball:update = function (deltaTime)

	self:setPosition(self:getPosition() + Vec3(normalized.x * self.speed, normalized.y * self.speed, normalized.z * self.speed))
	
end