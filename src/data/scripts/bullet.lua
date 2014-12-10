logMessage("using bullet.lua")

Bullet = {}
Bullet.__index = Bullet

setmetatable(Bullet, {
	__call = function (cls, ...)
		return cls.new(...)
	end,
})

function Bullet.new(startPosition)
    self = setmetatable({}, Bullet)
	self.startPosition = startPosition
	go = GameObjectManager:createGameObject("bullet")
	
	-- Render
	go.render = go:createRenderComponent()
	go.render:setPath("data/models/ball.thmodel")
	
	-- Physics
	go.pc = go:createPhysicsComponent()
	cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createSphere(0.2)
	cinfo.motionType = MotionType.Dynamic
	cinfo.mass = 0.00001
	cinfo.restitution = 0
	cinfo.position = startPosition
	cinfo.maxLinearVelocity = 10
	go.rb = go.pc:createRigidBody(cinfo)
	
	-- Register update
	go.sc = go:createScriptComponent()
	go.sc:setUpdateFunction(self.update)
	
	go.rb:setLinearVelocity(Vec3(0,0,10))
	--self.go.rb:applyLinearImpulse(Vec3(0,0,10000))
	
	go:setPosition(startPosition)
	
	self.go = go
	
	return self
end

function Bullet:getStartPosition()
	return self.startPosition
end

function Bullet:update(deltaTime)
	logMessage("guid : " .. self)
	-- logMessage(self.ok)
	--self.rb:setLinearVelocity(Vec3(0,0,10))
end
