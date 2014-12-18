
logMessage("using ball.lua")

balls = {}

function ballInitialize(name, hp, startpos, startvel)

local ball = GameObjectManager:createGameObject(name)
ball.pc = ball:createPhysicsComponent()
cinfo = RigidBodyCInfo()
cinfo.shape = PhysicsFactory:createSphere(0.5)
cinfo.motionType = MotionType.Dynamic
cinfo.mass = 1.0
cinfo.friction = 0.0
cinfo.angularDamping = 0.0
cinfo.linearDamping = 0.0
cinfo.restitution = 0.0
cinfo.position = startpos
cinfo.linearVelocity = startvel

-- collision event
ball.pc:getContactPointEvent():registerListener(ballCollision)

ball.rb = ball.pc:createRigidBody(cinfo)

ball.sc = ball:createScriptComponent()

local render = ball:createRenderComponent()
render:setPath("data/models/balls/mediumBall.thModel")

ball.hp = hp
	
rawset(balls, name, ball)

end

function ballCollision(event)

local ball = event:getBody(CollisionArgsCallbackSource.A)
local other = event:getBody(CollisionArgsCallbackSource.B)

--[[

	ballCollision handles each Collision of the ball with whatever it hit.
	The event returns ball as first callback source and the other object as the second callback source.
	
	This information is used to apply an Linear Impulse to the ball, that moves it according to the direction
	
	(Unfortunately there is no way to directly apply the Linear Impulse on the Rigid Body gotten from event, so an Iterator is used to check,
	which ball actually hit the other object.)
	applyLinearImpulse always seems to throw an error for some reason:
		Error: [1173527246] Constraint\Contact\hkpSimpleContactConstraintData.cpp(176):
		You modified the linear velocity of a body during a callback without calling 
		accessVelocities/updateVelocities.
	

]]--

	if (other:equals(floor.rb)) then
	
	for k, v in pairs(balls) do
		
		if (ball:equals(v.rb)) then
		
		logMessage(k.."hit floor")
		
		v.hitFloor = true
		
		end
		
	end
	
	end
	
	return EventResult.Handled

end

setmetatable(balls, balls)