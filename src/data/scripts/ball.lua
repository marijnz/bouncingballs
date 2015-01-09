logMessage("using ball.lua")

FLOOR_Z = 0.19
CEILING_Z = 4.6 
BALL_MEDIUM_SHADOW_SIZE = 0.55

ball = {}
ball.__index = ball

balls = {}

setmetatable(ball, {
  __index = PoolObject, -- this is what makes the inheritance work
  __call = function (cls, ...)
    self = setmetatable({}, cls)
    return self
  end,
})

function ball:create()
	go = GameObjectManager:createGameObject("ball" .. self.uniqueIdentifier)
	
	-- Render
	go.render = go:createRenderComponent()
	go.render:setPath("data/models/balls/mediumBall.thModel")
	
	-- Physics
	go.pc = go:createPhysicsComponent()
	cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createSphere(0.5)
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

    -- Shadow of the ball
    shadow = GameObjectManager:createGameObject("ball" .. self.uniqueIdentifier .. "-shadow")
    shadow.render = shadow:createRenderComponent()
    shadow.render:setPath("data/models/shadow.thModel")
    shadow.render:setScale(Vec3(0.55, 0.55, 0.55))
    self.shadow = shadow
	
	rawset(balls, "ball" .. self.uniqueIdentifier, self)
	
end

function ball:initialize()
	 
	 self.go:setComponentStates(ComponentState.Active)
	 
	 logMessage(self.go:getGuid().."initialized")
	 
end

function ball.ballCollision(event)
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
				
					if (other:equals(value.go.rb)) then
					
					v.hitBullet = true
					
					logMessage(v.go:getGuid().."hit Bullet")
					
					end
					
				end
			
            break
        end
    end

end

function ball:setInitialPositionAndMovement(position, LinearVelocity)
	 self:setPosition(position)
	 self:setLinearVelocity(LinearVelocity)
     self.shadow:setPosition(position)
end

function ball:dispose()
	self.go:setPosition(Vec3(100,100,0))
end


function ball:update()

    local position = self.go:getPosition()

    -- Put the shadow on the same position
    self.shadow:setPosition(Vec3(position.x, position.y, FLOOR_Z))

    -- Calculate the shadow size of the ball based on how far the ball is from the floor and ceiling
    local shadowScale = BALL_MEDIUM_SHADOW_SIZE * (1 - (position.z / ((CEILING_Z - FLOOR_Z) * 1.75)));
    self.shadow.render:setScale(Vec3(shadowScale, shadowScale, shadowScale))

    if (self.hitBullet) then
        self.hitBullet = false
        --todo: get smaller balls here
        objectManager:put(ball, self)
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

function ball:setPosition(position)
	self.go:setPosition(position)
end

function ball:setLinearVelocity(linearVelocity)
	self.go.rb:setLinearVelocity(linearVelocity)
end
