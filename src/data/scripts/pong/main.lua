-- Use the default physics world.
include("utils/physicsWorld.lua")

-- Use the default state machine.
include("utils/stateMachine.lua")


-- Enable wireframe drawing of physics geometry
PhysicsSystem:setDebugDrawingEnabled(true)

-- Initialize the pseudo random number generator
math.randomseed(os.time())
math.random(); math.random(); math.random() -- Why are we doing that? http://lua-users.org/wiki/MathLibraryTutorial

local playerSpeed = 12.0
local playerRotSpeed = 2.5
local maxLinearVelocity = 15

p1 = GameObjectManager:createGameObject("player1")
p2 = GameObjectManager:createGameObject("player2")
ball = GameObjectManager:createGameObject("ball")
cam = GameObjectManager:createGameObject("camera")

function initializePlayer(go)
	local physicsComponent = go:createPhysicsComponent()
	local cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createBox(0.2, 0.2, 1.05) -- 40cm width and depth, 2.1m height

	cinfo.motionType = MotionType.Keyframed
	cinfo.restitution = 1.0
	physicsComponent:createRigidBody(cinfo)
	local scriptComponent = go:createScriptComponent()
end

function initializeBall(go)
	local physicsComponent = go:createPhysicsComponent()
	local cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createSphere(0.35) -- 70cm diameter
	cinfo.motionType = MotionType.Dynamic
	cinfo.mass = 1.0
	cinfo.friction = 0.5
	cinfo.restitution = 1.0
	cinfo.linearVelocity = Vec3(0, -(maxLinearVelocity - 5), 2.5)
	--cinfo.angularVelocity = Vec3(0, 15, 0)
	cinfo.maxLinearVelocity = maxLinearVelocity
	physicsComponent:createRigidBody(cinfo)
	local scriptComponent = go:createScriptComponent()
	-- The bounds in which the ball will not disappear (y axis)
	go.bounds = {
		left = -15,
		right = 15,
	}
end

function createBorder()
	local boxDimensions = Vec3(0.2, 30.0, 0.2) -- 40cm height and depth, 60m width
	local distanceFromCenter = 7.0 -- => Space between borders is 14m

	-- Top
	local go = GameObjectManager:createGameObject("borderTop")
	local physicsComponent = go:createPhysicsComponent()
	local cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createBox(boxDimensions)
	cinfo.motionType = MotionType.Fixed
	cinfo.friction = 0.0
	cinfo.angularDamping = 0.0
	cinfo.restitution = 1.0
	physicsComponent:createRigidBody(cinfo)
	go:setPosition(Vec3(0.0, 0.0, distanceFromCenter))

	-- Bottom
	local go = GameObjectManager:createGameObject("borderBottom")
	local physicsComponent = go:createPhysicsComponent()
	local cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createBox(boxDimensions)
	cinfo.motionType = MotionType.Fixed
	cinfo.friction = 0.0
	cinfo.angularDamping = 0.0
	cinfo.restitution = 1.0
	physicsComponent:createRigidBody(cinfo)
	go:setPosition(Vec3(0.0, 0.0, -distanceFromCenter))
end

function p1.update(gameObjectName, elapsedMilliseconds)
	local pos = p1:getPosition()
	local vel = Vec3(0.0, 0.0, 0.0)
	local angVel = Vec3(0.0, 0.0, 0.0)

	--Move Up
	if (InputHandler:isPressed(Key.W) and pos.z < 5.5) then vel.z = playerSpeed end
	--Move Down
	if (InputHandler:isPressed(Key.S) and pos.z > -5.5) then vel.z = -playerSpeed end
	--Move Left
	if (InputHandler:isPressed(Key.A) and pos.y < 14) then vel.y = playerSpeed end
	--Move Right
	if (InputHandler:isPressed(Key.D) and pos.y > 5) then vel.y = -playerSpeed end
	--Rotate Left
	if InputHandler:isPressed(Key.E) then angVel.x = playerRotSpeed end
	--Rotate Right
	if InputHandler:isPressed(Key.Q) then angVel.x = -playerRotSpeed end

	p1:getPhysicsComponent():getRigidBody():setLinearVelocity(vel)
	p1:getPhysicsComponent():getRigidBody():setAngularVelocity(angVel)
end

function p2.update(gameObjectName, elapsedMilliseconds)
	local pos = p2:getPosition()
	local vel = Vec3(0.0, 0.0, 0.0)
	local angVel = Vec3(0.0, 0.0, 0.0)

	--Move Up
	if (InputHandler:isPressed(Key.Up) and pos.z < 5.5) then vel.z = playerSpeed end
	--Move Down
	if (InputHandler:isPressed(Key.Down) and pos.z > -5.5) then vel.z = -playerSpeed end
	--Move Left
	if (InputHandler:isPressed(Key.Left) and pos.y < -5) then vel.y = playerSpeed end
	--Move Right
	if (InputHandler:isPressed(Key.Right) and pos.y > -14) then vel.y = -playerSpeed end
	--Rotate Left
	if InputHandler:isPressed(Key.O) then angVel.x = playerRotSpeed end
	--Rotate Right
	if InputHandler:isPressed(Key.P) then angVel.x = -playerRotSpeed end

	p2:getPhysicsComponent():getRigidBody():setLinearVelocity(vel)
	p2:getPhysicsComponent():getRigidBody():setAngularVelocity(angVel)
end

function ball.update(gameObjectName, elapsedMilliseconds)
	--prevent the ball from leaving the 2 dimensional world
	local vel = ball:getPhysicsComponent():getRigidBody():getLinearVelocity()
	ball:getPhysicsComponent():getRigidBody():setLinearVelocity(Vec3(0, vel.y, vel.z))

	local pos = ball:getPosition()
	ball:setPosition(Vec3(0.0, pos.y, pos.z))

	--player two misses
	if pos.y < ball.bounds.left then
		p1.score = p1.score + 1
		ball:setPosition(Vec3(0, 0, 0))
		ball:getPhysicsComponent():getRigidBody():setLinearVelocity(Vec3(0, maxLinearVelocity, math.random(0, 50)))
	end

	--player one misses
	if pos.y > ball.bounds.right then
		p2.score = p2.score + 1
		ball:setPosition(Vec3(0, 0, 0))
		ball:getPhysicsComponent():getRigidBody():setLinearVelocity(Vec3(0, -maxLinearVelocity, math.random(0, 50)))
	end
	
	DebugRenderer:drawOrigin()
	DebugRenderer:printText(Vec2(-0.9, 0.8), "Player 1 Score: " .. p1.score)
	DebugRenderer:printText(Vec2( 0.7, 0.8), "Player 2 Score: " .. p2.score)
end

--Player 1
initializePlayer(p1)
p1:setPosition(Vec3(0, 12, 0))
p1:getScriptComponent():setUpdateFunction(p1.update)
p1.score = 0

--player 2
initializePlayer(p2)
p2:setPosition(Vec3(0, -12, 0))
p2:getScriptComponent():setUpdateFunction(p2.update)
p2.score = 0

--ball
initializeBall(ball)
ball:getScriptComponent():setUpdateFunction(ball.update)

--borders
createBorder()

-- Create and position the basic camera.
cameraComponent = cam:createCameraComponent()
local camLookAt = Vec3(0.0, 0.0, 1.0)
cameraComponent:setPosition(camLookAt + Vec3(-15, 0, 0))
cameraComponent:lookAt(camLookAt)
cameraComponent:setState(ComponentState.Active)

Events.Update:registerListener(function(dt)
	local mouseDelta = InputHandler:getMouseDelta()
	local camPos = cam:getPosition()
	camPos.x = camPos.x + mouseDelta.z -- mouse wheel
	cam:setPosition(camPos)
	DebugRenderer:printText(Vec2(0.0, 0.99), "New cam pos: " .. tostring(camPos, "vec3") ..
		                                    "\nPlayer1 pos: " .. tostring(p1:getPosition(), "vec3") ..
		                                    "\nPlayer2 pos: " .. tostring(p2:getPosition(), "vec3"))
	return EventResult.Handled
end)
