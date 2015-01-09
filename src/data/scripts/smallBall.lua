logMessage("using SmallBall.lua")

SmallBall = {}
SmallBall.__index = SmallBall

setmetatable(SmallBall, {
  __index = ball, -- this is what makes the inheritance work
  __call = function (cls, ...)
    self = setmetatable({}, cls)
    return self
  end,
})

function SmallBall:create()
	go = GameObjectManager:createGameObject("SmallBall" .. self.uniqueIdentifier)
	
	-- Render
	--go.render = go:createRenderComponent()
	--go.render:setPath("data/models/balls/smallBall.thModel")
	
	-- Physics
	go.pc = go:createPhysicsComponent()
	cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createSphere(0.25)
	cinfo.motionType = MotionType.Dynamic
	cinfo.mass = 1.0
	cinfo.friction = 0.0
	cinfo.angularDamping = 0.0
	cinfo.linearDamping = 0.0
	cinfo.restitution = 0.0
	cinfo.position = Vec3(0,0,0)
	
	go.pc:getContactPointEvent():registerListener(self.ballCollision)
	
	go.rb = go.pc:createRigidBody(cinfo)
	
	go.rb:setLinearVelocity(Vec3(0,0,10))
	
	go:setPosition(Vec3(100,100,0))
	
	go:setComponentStates(ComponentState.Inactive)
		
	self.go = go
	
	rawset(balls, "SmallBall" .. self.uniqueIdentifier, self)
	
	logMessage("smallBall created")
	
end

function SmallBall:update()
	
		if (self.hitBullet) then
		
		self.hitBullet = false
		
		objectManager:put(SmallBall, self)
				
		end
	
		if (self.hitFloor) then
		
		self.go.rb:applyLinearImpulse(Vec3(0, 0, floorBounciness))
		
		self.hitFloor = false
		
		end
		
		if (self.hitWall1) then
		
		self.go.rb:applyLinearImpulse(Vec3(-wallBounciness, 0, 0))
		
		self.hitWall1 = false
		
		end
		
		if (self.hitWall2) then
		
		self.go.rb:applyLinearImpulse(Vec3(wallBounciness, 0 , 0))
		
		self.hitWall2 = false
		
		end
		
		if (self.hitWall3) then
		
		self.go.rb:applyLinearImpulse(Vec3(0, -wallBounciness, 0))
		
		self.hitWall3 = false
		
		end
		
		if (self.hitWall4) then
		
		self.go.rb:applyLinearImpulse(Vec3(0, wallBounciness, 0))
		
		self.hitWall4 = false
		
		end
	
end