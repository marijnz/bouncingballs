include("utils/stateMachine.lua")
include("utils/freeCamera.lua")

math.randomseed(os.time())

do
	local cinfo = WorldCInfo()
	cinfo.gravity = Vec3(0, 0, -9.81)
	world = PhysicsFactory:createWorld(cinfo)
	PhysicsSystem:setWorld(world)
end

PhysicsSystem:setDebugDrawingEnabled(true)

freeCam.cc:setPosition(Vec3(20, 0, 0))
freeCam.cc:lookAt(Vec3(0, 0, 0))

-- Small helper to produce a pseudo-random vector
function RandomVec3(strength)
	strength = strength or 1
	return Vec3(((math.random() * 2) - 1) * strength,
			    ((math.random() * 2) - 1) * strength,
			    ((math.random() * 2) - 1) * strength)
end

-- Reset event that is triggered when R is pressed.
local ResetEvent = Events.create()
Events.Update:registerListener(function(dt)
end)

-- Captures the state of the rigid body when this function is called and restores that state when the ResetEvent is triggered.
local function registerResettableRigidBody(rb)
	local origMotion = rb:getMotionType()
	local origPos    = rb:getPosition()
	local origRot    = rb:getRotation()
	local origLinVel = rb:getLinearVelocity()
	local origAngVel = rb:getAngularVelocity()
	ResetEvent:registerListener(function()
		rb:setMotionType(origMotion)
		rb:setPosition(origPos)
		rb:setRotation(origRot)
		rb:setLinearVelocity(origLinVel)
		rb:setAngularVelocity(origAngVel)
	end)
end

-- General update
function update(dt)
	if InputHandler:wasTriggered(Key.R) then
		ResetEvent:trigger{}
	end
	DebugRenderer:printText(Vec2(-0.3, 0.95),
	      "Demonstrating all supported physics constraints.\n"
	    .."Press R to reset rigid bodies to their initial state")
	DebugRenderer:drawOrigin()
end

Events.Update:registerListener(update)

ballAndSocketInstanceCount = 0
hingeInstanceCount         = 0
pointToPlaneInstanceCount  = 0
prismaticInstanceCount     = 0

-- Sample code for the Ball-and-Socket constraint
function ballAndSocket(offset)
	function create(guid, pos, motionType)
		go = GameObjectManager:createGameObject(guid)
		go.pc = go:createPhysicsComponent()
		local cinfo = RigidBodyCInfo()
		cinfo.shape = PhysicsFactory:createBox(Vec3(1, 1, 1))
		cinfo.mass = 1
		cinfo.motionType = motionType
		cinfo.position = offset + pos
		go.rb = go.pc:createRigidBody(cinfo)
		return go
	end

	-- bas - ball and socket
	local top =    create("bas.top" .. ballAndSocketInstanceCount,    Vec3(0.0, 0.0,  2.0), MotionType.Fixed)
	local bottom = create("bas.bottom" .. ballAndSocketInstanceCount, Vec3(0.0, 0.0, -2.0), MotionType.Dynamic)
	ballAndSocketInstanceCount = ballAndSocketInstanceCount + 1

	local cinfo = {
		type = ConstraintType.BallAndSocket,
		A = top.rb,
		B = bottom.rb, -- Comment out this line to use the world as reference point
		constraintSpace = "world",
		pivot = offset + Vec3(0, 0, 0),
		solvingMethod = "stable",
	}

	local constraint = PhysicsFactory:createConstraint(cinfo)

	-- The constraint must be added in the post-initialization phase
	Events.PostInitialization:registerListener(function() world:addConstraint(constraint) end)

	-- Handle user input.
	Events.Update:registerListener(function(dt)
		if InputHandler:wasTriggered(Key._1) then
			bottom.rb:applyLinearImpulse(RandomVec3(15))
		end
	end)

	if ballAndSocketInstanceCount <= 1 then
		-- Print some instructions
		Events.Update:registerListener(function(dt)
			DebugRenderer:printText(Vec2(-0.95, 0.75), "Ball and Socket: Press 1 to get the lower cube moving")
			DebugRenderer:printText3D(top:getPosition() + Vec3(0, 0, 2), "Ball and Socket")
		end)
	end
	registerResettableRigidBody(bottom.rb)
end

-- Sample code for the Hinge constraint
function hinge(offset)
	function create(guid, pos, motionType)
		go = GameObjectManager:createGameObject(guid)
		go.pc = go:createPhysicsComponent()
		local cinfo = RigidBodyCInfo()
		cinfo.shape = PhysicsFactory:createBox(Vec3(1, 1, 1))
		cinfo.mass = 1
		cinfo.motionType = motionType
		cinfo.position = offset + pos
		cinfo.rotation = Quaternion(Vec3(0, 1, 0), 45)
		go.rb = go.pc:createRigidBody(cinfo)
		return go
	end

	local top =    create("hinge.top" .. hingeInstanceCount,    Vec3(0.0, 0.0,  2.0), MotionType.Fixed)
	local bottom = create("hinge.bottom" .. hingeInstanceCount, Vec3(0.0, 0.0, -2.0), MotionType.Dynamic)
	hingeInstanceCount = hingeInstanceCount + 1

	local cinfo = {
		type = ConstraintType.Hinge,
		A = top.rb,
		B = bottom.rb, -- Comment out this line to use the world as reference point
		constraintSpace = "world",
		pivot = Vec3(0, 1, 0),
		axis = Vec3(0, 1, 0),
	}

	local constraint = PhysicsFactory:createConstraint(cinfo)

	-- The constraint must be added in the post-initialization phase
	Events.PostInitialization:registerListener(function() world:addConstraint(constraint) end)

	-- Handle user input.
	Events.Update:registerListener(function(dt)
		if InputHandler:wasTriggered(Key._2) then
			bottom.rb:applyLinearImpulse(RandomVec3(15))
		end
	end)

	if hingeInstanceCount <= 1 then
		-- Print some instructions
		Events.Update:registerListener(function(dt)
			DebugRenderer:printText(Vec2(-0.95, 0.7), "Hinge: Press 2 to get the lower cube moving")
			DebugRenderer:printText3D(top:getPosition() + Vec3(0, 0, 2), "Hinge")
		end)
	end
	registerResettableRigidBody(bottom.rb)
end

-- Sample code for the Point-to-Plane constraint
function pointToPlane(offset)
	function create(guid, pos, motionType)
		go = GameObjectManager:createGameObject(guid)
		go.pc = go:createPhysicsComponent()
		local cinfo = RigidBodyCInfo()
		cinfo.shape = PhysicsFactory:createBox(Vec3(1, 1, 1))
		cinfo.mass = 1
		cinfo.motionType = motionType
		cinfo.position = offset + pos
		cinfo.rotation = Quaternion(Vec3(0, 1, 0), 45)
		go.rb = go.pc:createRigidBody(cinfo)
		return go
	end

	local top    = create("pointToPlane.top"    .. pointToPlaneInstanceCount, Vec3(0.0, 0.0,  2.0), MotionType.Fixed)
	local bottom = create("pointToPlane.bottom" .. pointToPlaneInstanceCount, Vec3(0.0, 0.0, -2.0), MotionType.Dynamic)
	pointToPlaneInstanceCount = pointToPlaneInstanceCount + 1

	local cinfo = {
		type = ConstraintType.PointToPlane,
		A = bottom.rb,
		B = top.rb, -- Comment out this line to use the world as reference point
		constraintSpace = "world",
		pivot = bottom:getPosition(),
		up = Vec3(0, 0, 1),
		solvingMethod = "stable",
	}

	local constraint = PhysicsFactory:createConstraint(cinfo)

	-- The constraint must be added in the post-initialization phase
	Events.PostInitialization:registerListener(function() world:addConstraint(constraint) end)

	-- Handle user input.
	Events.Update:registerListener(function(dt)
		if InputHandler:wasTriggered(Key._3) then
			logMessage("Hello plane world!")
			bottom.rb:applyLinearImpulse(RandomVec3(10))
		end
	end)

	if pointToPlaneInstanceCount <= 1 then
		-- Print some instructions
		Events.Update:registerListener(function(dt)
			DebugRenderer:printText(Vec2(-0.95, 0.65), "Point-to-Plane: Press 3 to get the bottom cube moving")
			DebugRenderer:printText3D(top:getPosition() + Vec3(0, 0, 2), "Point-to-Plane")
		end)
	end
	registerResettableRigidBody(bottom.rb)
end

-- Sample code for the Prismatic constraint which lets you constraint an object to a single axis
PrismaticMode = {
	Linear = 1,  -- Only linear movement along the axis allowed
	Angular = 2, -- Only angular movement around the axis allowed
	Both = 3     -- Both linear and angular movement along/around the axis allowed
}
function prismatic(offset, mode)
	function create(guid, pos, motionType)
		go = GameObjectManager:createGameObject(guid)
		go.pc = go:createPhysicsComponent()
		local cinfo = RigidBodyCInfo()
		cinfo.shape = PhysicsFactory:createBox(Vec3(1, 1, 1))
		cinfo.mass = 1
		cinfo.motionType = motionType
		cinfo.position = offset + pos
		cinfo.rotation = Quaternion(Vec3(0, 1, 0), 45)
		go.rb = go.pc:createRigidBody(cinfo)
		return go
	end

	local top    = create("prismatic.top"    .. prismaticInstanceCount, Vec3(0.0, 0.0,  2.0), MotionType.Fixed)
	local bottom = create("prismatic.bottom" .. prismaticInstanceCount, Vec3(0.0, 0.0, -2.0), MotionType.Dynamic)
	prismaticInstanceCount = prismaticInstanceCount + 1

	local cinfo = {
		type = ConstraintType.Prismatic,
		A = top.rb,
		B = bottom.rb, -- Comment out this line to use the world as reference point
		constraintSpace = "world",
		pivot = bottom:getPosition(),
		axis = bottom:getViewDirection(), -- Can only go/rotate around the "front" axis
		allowRotation = mode > PrismaticMode.Linear,
	}

	if mode == PrismaticMode.Angular then
		cinfo.maxLinearLimit = 0
		cinfo.minLinearLimit = 0
	end

	local constraint = PhysicsFactory:createConstraint(cinfo)

	-- The constraint must be added in the post-initialization phase
	Events.PostInitialization:registerListener(function() world:addConstraint(constraint) end)

	-- Handle user input.
	Events.Update:registerListener(function(dt)
		if InputHandler:wasTriggered(Key._4) then
			bottom.rb:applyLinearImpulse(RandomVec3(10))
		end
	end)

	if prismaticInstanceCount <= 1 then
		-- Print some instructions
		Events.Update:registerListener(function(dt)
			DebugRenderer:printText(Vec2(-0.95, 0.6), "Prismatic (linear): Press 4 to get the bottom cube moving")
			DebugRenderer:printText3D(top:getPosition() + Vec3(0, 0, 2), "Prismatic (linear)")
		end)
	end
	registerResettableRigidBody(bottom.rb)
end

-- Actually set up the scene content

ballAndSocket(Vec3(0, -10, 0))
hinge        (Vec3(0,  -5, 0))
pointToPlane (Vec3(0,   0, 0))
prismatic    (Vec3(0,   5, 0), PrismaticMode.Linear) -- Choose a different PrismaticMode value to see the difference
