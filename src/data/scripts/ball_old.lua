
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
ball.hitObject = 0
	
rawset(balls, name, ball)

end

function ballCollision(event)

local ball = event:getBody(CollisionArgsCallbackSource.A)
local other = event:getBody(CollisionArgsCallbackSource.B)

--check for collision with floor

	if (other:equals(floor.rb)) then
	
		for k, v in pairs(balls) do
		
			if (ball:equals(v.rb)) then
		
			v.hitFloor = true
		
			end
		
		end
		
	end
		
--hard coded wall detection

	if (other:equals(wall1.rb)) then
	
		for k, v in pairs(balls) do
		
			if (ball:equals(v.rb)) then
		
			v.hitWall1 = true
		
			end
		
		end
		
	end
	
	if (other:equals(wall2.rb)) then
	
		for k, v in pairs(balls) do
		
			if (ball:equals(v.rb)) then
		
			v.hitWall2 = true
		
			end
		
		end
		
	end
	
	if (other:equals(wall3.rb)) then
	
		for k, v in pairs(balls) do
		
			if (ball:equals(v.rb)) then
		
			v.hitWall3 = true
		
			end
		
		end
		
	end
	
	if (other:equals(wall4.rb)) then
	
		for k, v in pairs(balls) do
		
			if (ball:equals(v.rb)) then
		
			v.hitWall4 = true
		
			end
			
		end
		
	end
	

--check for collision with other objects (soft coded attempt)
--[[		
	else
	
		for k, v in pairs(balls) do
		
			if (ball:equals(v.rb)) then
		
			v.hitObject = 3
		
			end
			
			if (other:equals(v.rb)) then
		
			v.hitObject = 3
		
			end
		
		end
	
	end
--]]	
	return EventResult.Handled

end

setmetatable(balls, balls)