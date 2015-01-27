logMessage("using Ball.lua")

Ball = {}
Ball.__index = Ball


setmetatable(Ball, {  __index = PoolObject, -- this is what makes the inheritance work
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
	cinfo.restitution = 1.0
	cinfo.position = Vec3(0,0,0)
	go.rb = go.pc:createRigidBody(cinfo)
    go.rb:setUserData({type = USERDATA_TYPE_BALL})
	
	go.pc:getContactPointEvent():registerListener(self.BallCollision)

	go:setComponentStates(ComponentState.Inactive)
	
	self.go = go
    self.speedZ = 9

    -- Shadow of the Ball
    shadow = GameObjectManager:createGameObject("Ball" .. self.uniqueIdentifier .. "-shadow")
    shadow.render = shadow:createRenderComponent()
    shadow.render:setPath("data/models/shadow.thModel")
	shadow.size = 1.1 * size
    shadow.render:setScale(Vec3(0.55, 0.55, 0.55))
    self.shadow = shadow
	self.shadow:setComponentStates(ComponentState.Inactive)
	
	levelManager:addBall(self)	
	logMessage(go:getGuid().." created")

    -- We have to set a default value otherwise there is a crash as soon as you 'get the data'
    self.go.rb:setUserData({})
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

    local otherCollisionData = other:getUserData()
    local newBallCollisionData = {type = USERDATA_TYPE_BALL}

    -- Fix speed of ball when bouncing on floor by giving an impulse
    --[[
    if (otherCollisionData.type == USERDATA_TYPE_FLOOR) then
        print("HITFLOOR")
        local velocity = self:getLinearVelocity()
        local direction = velocity
        local impulse = Vec3(0, 0, direction.z * 0.276)
        event:accessVelocities(CollisionArgsCallbackSource.A)
        self:applyLinearImpulse(impulse)
        event:updateVelocities(CollisionArgsCallbackSource.A)
    end
    ]]--

    newBallCollisionData.resetVelocityZ = otherCollisionData.type == USERDATA_TYPE_FLOOR

    -- If boucing against a ball or a wall, reset the velocity next frame
    newBallCollisionData.resetVelocityXY = otherCollisionData.type == USERDATA_TYPE_WALL or otherCollisionData.type == USERDATA_TYPE_BALL

    -- If a hookshot collides with a ball
    newBallCollisionData.gotHit = otherCollisionData.type == USERDATA_TYPE_HOOKSHOT

    -- Save data in the rigid body
    self:setUserData(newBallCollisionData)

    return EventResult.Handled
end

function Ball:setInitialPositionAndMovement(position, LinearVelocity)
	self:setPosition(position)
	self:setLinearVelocity(LinearVelocity)
    self.speedXY = vec2Length(LinearVelocity)
    self.shadow:setPosition(position)
end

function Ball:dispose()
	levelManager:removeBall(self)
	
	self.go:setComponentStates(ComponentState.Inactive)
	
	self.shadow:setComponentStates(ComponentState.Inactive)
end

function Ball:freeze()
	self.go.pc:setState(ComponentState.Inactive)
end

function Ball:unfreeze()
	self.go.pc:setState(ComponentState.Active)
end

function Ball:update()

    local position = self.go:getPosition()

	-- Put the shadow on the same position
    self.shadow:setPosition(Vec3(position.x, position.y, FLOOR_Z))

    -- Calculate the shadow size of the Ball based on how far the Ball is from the floor and ceiling
    local shadowScale = self.shadow.size * (1 - (position.z / ((CEILING_Z - FLOOR_Z) * 1.75)));
    self.shadow.render:setScale(Vec3(shadowScale, shadowScale, shadowScale))

    local lastCollisionData = self.go.rb:getUserData()
    local newCollisionData = {type = USERDATA_TYPE_BALL}

    if (lastCollisionData.resetVelocityZ) then
        print("resetVelocityZ")
        local velocity = self.go.rb:getLinearVelocity()
        velocity = Vec3(velocity.x, velocity.y, self.speedZ)
        self.go.rb:setLinearVelocity(velocity)
    end

    if (lastCollisionData.resetVelocityXY) then
        local velocity = self.go.rb:getLinearVelocity()
        local velocityNormalized = vec2Normalize(velocity)
        velocity = Vec3(velocityNormalized.x * self.speedXY, velocityNormalized.y * self.speedXY, velocity.z)
        self.go.rb:setLinearVelocity(velocity)
    end

    self.go.rb:setUserData(newCollisionData)
end

function Ball:setPosition(position)
	self.go:setPosition(position)
end

function Ball:setLinearVelocity(linearVelocity)
	self.go.rb:setLinearVelocity(linearVelocity)
    self.speedXY = vec2Length(linearVelocity)
end
