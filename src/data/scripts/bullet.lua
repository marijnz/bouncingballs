logMessage("using bullet.lua")

Bullet = {}
Bullet.__index = Bullet

setmetatable(Bullet, {
  __index = PoolObject, -- this is what makes the inheritance work
  __call = function (cls, ...)
    self = setmetatable({}, cls)
    return self
  end,
})

function Bullet:create()
	go = GameObjectManager:createGameObject("bullet" .. self.uniqueIdentifier)
	
	-- Render
	--go.render = go:createRenderComponent()
	--go.render:setPath("data/models/ball.thmodel")
	
	-- Physics
	go.pc = go:createPhysicsComponent()
	cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createSphere(0.2)
	cinfo.motionType = MotionType.Dynamic
	cinfo.mass = 0.00001
	cinfo.restitution = 0
	cinfo.position = Vec3(0,0,0)
	cinfo.maxLinearVelocity = 10
	go.rb = go.pc:createRigidBody(cinfo)
	
	go.rb:setLinearVelocity(Vec3(0,0,10))
	--self.go.rb:applyLinearImpulse(Vec3(0,0,10000))
	
	go:setPosition(Vec3(100,100,0))
	
	self.go = go
end

function Bullet:initialize()
	 
end

function Bullet:dispose()
	self.go:setPosition(Vec3(100,100,0))
end

function Bullet:update()
	self.go.rb:setLinearVelocity(Vec3(0,0,10))
	if(self.go:getPosition().z > 5) then
		objectManager:put(Bullet, self)
	end
end

function Bullet:setPosition(position)
	self.go:setPosition(position)
end