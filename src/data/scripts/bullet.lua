logMessage("using bullet.lua")

Bullet = {}
Bullet.__index = Bullet

BULLET_SPEED = 13


setmetatable(Bullet, {
  __index = PoolObject, -- this is what makes the inheritance work
  __call = function (cls, ...)
    self = setmetatable({}, cls)
    return self
  end,
})


function Bullet:getRigidBody()
	return self.goCollision.rb
end

function Bullet:create()

	-- Render
	goVisual = GameObjectManager:createGameObject("bullet_visual" .. self.uniqueIdentifier)
	
	goVisual.render = goVisual:createRenderComponent()
	goVisual.render:setPath("data/models/hook-shot.thmodel")

	-- Physics
	goCollision = GameObjectManager:createGameObject("bullet_collision" .. self.uniqueIdentifier)
	
	goCollision.pc = goCollision:createPhysicsComponent()
	cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createBox(0.1, 0.1, 10)
	cinfo.motionType = MotionType.Fixed
	cinfo.mass = 0.00001
	cinfo.restitution = 0
	cinfo.position = Vec3(0,0,0)
	cinfo.maxLinearVelocity = 10
	
	cinfo.isTriggerVolume = true
	
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
	--toPositionVisual = self.goVisual:getPosition()
	--toPositionVisual.z  = toPositionVisual.z + (deltaTime * BULLET_SPEED/2)
	--self.goVisual:setPosition(toPositionVisual)
	
	toPositionCollision = self.goCollision:getPosition()
	toPositionCollision.z  = toPositionCollision.z + (deltaTime * BULLET_SPEED)
	self.goCollision:setPosition(toPositionCollision)
	
	
	if(self.goCollision:getPosition().z > -3) then
		objectManager:put(Bullet, self)
	end
	
	
	toScale = self.goVisual.render:getScale()
	toScale.z = toScale.z + (deltaTime * BULLET_SPEED / 2 * 20)
	self.goVisual.render:setScale(toScale) -- Distorted in X direction
end

function Bullet:setPosition(position)
	self.goVisual:setPosition(position)
	self.goCollision:setPosition(position)
end

function Bullet:setInitialPosition(position)
	position.z  = FLOOR_Z
	self.goVisual:setPosition(position)
	position.z  = position.z + (1) - 11
	self.goCollision:setPosition(position)
	self.goVisual.render:setScale(Vec3(1,1,1))
end