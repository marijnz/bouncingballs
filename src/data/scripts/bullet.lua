logMessage("using bullet.lua")

Bullet = {}
Bullet.__index = Bullet

BULLET_SPEED = 8

setmetatable(Bullet, {
  __index = PoolObject, -- this is what makes the inheritance work
  __call = function (cls, ...)
    self = setmetatable({}, cls)
    return self
  end,
})

function Bullet:create()

	-- Render
	goVisual = GameObjectManager:createGameObject("bullet_visual" .. self.uniqueIdentifier)
	
	goVisual.render = goVisual:createRenderComponent()
	goVisual.render:setPath("data/models/ball.thmodel")

	-- Physics
	goCollision = GameObjectManager:createGameObject("bullet_collision" .. self.uniqueIdentifier)
	
	goCollision.pc = goCollision:createPhysicsComponent()
	cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createBox(0.3, 0.3, 0.02)
	cinfo.motionType = MotionType.Fixed
	cinfo.mass = 0.00001
	cinfo.restitution = 0
	cinfo.position = Vec3(0,0,0)
	cinfo.maxLinearVelocity = 10
	goCollision.rb = goCollision.pc:createRigidBody(cinfo)
	
	self.goVisual = goVisual
	self.goCollision = goCollision

	-- Set initial position
	self:setPosition(Vec3(100,100,0))
	
end

function Bullet:initialize()
	 
end

function Bullet:dispose()
	self:setPosition(Vec3(100,100,0))
	self.goVisual.render:setScale(Vec3(1,1,1))
end

function Bullet:update(deltaTime)
	toPositionVisual = self.goVisual:getPosition()
	toPositionVisual.z  = toPositionVisual.z + (deltaTime * BULLET_SPEED/2)
	self.goVisual:setPosition(toPositionVisual)
	
	toPositionCollision = self.goCollision:getPosition()
	toPositionCollision.z  = toPositionCollision.z + (deltaTime * BULLET_SPEED)
	self.goCollision:setPosition(toPositionCollision)
	
	
	if(self.goVisual:getPosition().z > 3) then
		objectManager:put(Bullet, self)
	end
	
	
	toScale = self.goVisual.render:getScale()
	toScale.z = toScale.z + (deltaTime * BULLET_SPEED / 2)
	self.goVisual.render:setScale(toScale) -- Distorted in X direction
end

function Bullet:setPosition(position)
	self.goVisual:setPosition(position)
	self.goCollision:setPosition(position)
end

function Bullet:setInitialPosition(position)
	self.goVisual:setPosition(position)
	position.z  = position.z + (1)
	self.goCollision:setPosition(position)
end