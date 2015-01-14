logMessage("using Ball.lua")

FLOOR_Z = 0.19
CEILING_Z = 4.6 

Ball = {}
Ball.__index = Ball

balls = {}

setmetatable(Ball, {
  __index = PoolObject, -- this is what makes the inheritance work
  __call = function (cls, ...)
    self = setmetatable({}, cls)
    return self
  end,
})

function Ball:create(model, size)

		go = GameObjectManager:createGameObject(model .. "-" .. self.uniqueIdentifier)
	
	-- Render
	go.render = go:createRenderComponent()
	go.render:setPath("data/models/Balls/" .. model .. ".thModel")
	go.render:setScale(Vec3(0.1*size, 0.1*size, 0.1*size))
	
	-- Physics
	go.pc = go:createPhysicsComponent()
	cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createSphere(size)
	cinfo.motionType = MotionType.Dynamic
	cinfo.mass = 1.0
	cinfo.friction = 0.0
	cinfo.angularDamping = 0.0
	cinfo.linearDamping = 0.0
	cinfo.restitution = 0.0
	cinfo.position = Vec3(0,0,0)
	go.rb = go.pc:createRigidBody(cinfo)
	
	go.pc:getContactPointEvent():registerListener(self.BallCollision)

	go:setComponentStates(ComponentState.Inactive)
	
	self.go = go

    -- Shadow of the Ball
    shadow = GameObjectManager:createGameObject("Ball" .. self.uniqueIdentifier .. "-shadow")
    shadow.render = shadow:createRenderComponent()
    shadow.render:setPath("data/models/shadow.thModel")
	shadow.size = 1.1 * size
    shadow.render:setScale(Vec3(0.55, 0.55, 0.55))
    self.shadow = shadow
	self.shadow:setComponentStates(ComponentState.Inactive)
	
	rawset(balls, "Ball" .. self.uniqueIdentifier, self)
	
	logMessage(go:getGuid().." created")
	
end

function Ball:getRigidBody()

	return self.go.rb

end

function Ball:initialize()
	 
	 self.go:setComponentStates(ComponentState.Active)
	 
	 logMessage(self.go:getGuid().."initialized")
	 
	 self.shadow:setComponentStates(ComponentState.Active)
	 
end

function Ball.BallCollision(event)
	local self = event:getBody(CollisionArgsCallbackSource.A)
	local other = event:getBody(CollisionArgsCallbackSource.B)
	
    for k, v in pairs(balls) do
        if (self:equals(v.go.rb)) then
            if (other:equals(floor.rb)) then
                v.hitFloor = true
            end
            if (other:equals(wall1.rb)) then
                v.hitWall1 = true
            end
            if (other:equals(wall2.rb)) then
                v.hitWall2 = true
            end
            if (other:equals(wall3.rb)) then
                v.hitWall3 = true
            end
            if (other:equals(wall4.rb)) then
                v.hitWall4 = true
            end
				for keys, value in pairs(bullets) do			
					if (other:equals(value:getRigidBody())) then
						v.hitBullet = true
					end					
				end				
				for keys, value in pairs(balls) do				
					if (other:equals(value:getRigidBody())) then					
						v.hitBall = true					
						logMessage(v.go:getGuid().."hit Ball")	
						v.newVel = value:getRigidBody():getLinearVelocity()
					end					
				end	
            break
        end
    end

end

function Ball:setInitialPositionAndMovement(position, LinearVelocity)
	 self:setPosition(position)
	 self:setLinearVelocity(LinearVelocity)
     self.shadow:setPosition(position)
end

function Ball:dispose()
	self.go:setPosition(Vec3(100,100,0))
	self.go:setComponentStates(ComponentState.Inactive)
	
	self.shadow:setComponentStates(ComponentState.Inactive)
	
end

function Ball:freeze()
	self.go.pc:setState(ComponentState.Inactive)
end

function Ball:update()

    local position = self.go:getPosition()

	-- Put the shadow on the same position
    self.shadow:setPosition(Vec3(position.x, position.y, FLOOR_Z))

    -- Calculate the shadow size of the Ball based on how far the Ball is from the floor and ceiling
    local shadowScale = self.shadow.size * (1 - (position.z / ((CEILING_Z - FLOOR_Z) * 1.75)));
    self.shadow.render:setScale(Vec3(shadowScale, shadowScale, shadowScale))

    if (self.hitFloor) then
        self.go.rb:applyLinearImpulse(Vec3(0, 0, floorBounciness))
        self.hitFloor = false
    end
	
	if (self.hitBall) then
		self.go.rb:setLinearVelocity(self.newVel)
		self.hitBall = false
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

function Ball:setPosition(position)
	self.go:setPosition(position)
end

function Ball:setLinearVelocity(linearVelocity)
	self.go.rb:setLinearVelocity(linearVelocity)
end
