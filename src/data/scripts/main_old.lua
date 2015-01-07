logMessage("using main.lua")

-- Options 
local options = {
    freecamera = true,
    debugDrawing = false
}	

PhysicsSystem:setDebugDrawingEnabled(options.debugDrawing)

-- Default state machine.
include("defaults/stateMachine.lua")

-- World
local world
include("world.lua")

-- Camera
local camera
if (options.freecamera) then
    include("freecamera.lua")
else
    include("camera.lua")
end

level = GameObjectManager:createGameObject("level")
level.render = level:createRenderComponent()
level.render:setPath("data/models/cube-level.thModel")
level:setPosition(Vec3(0, 0, 0))

-- Classes
include("objectManager.lua")
include("poolobject.lua")
include("bullet.lua")
objectManager:addPool(Bullet, 10)

--[[ Example usage of the PoolExampleObject
include("poolexampleobject.lua")

objectManager:addPool(PoolExampleObject, 5)

poolExampleObjectAnother = objectManager:grab(PoolExampleObject)
poolExampleObjectAndAnother = objectManager:grab(PoolExampleObject)

objectManager:put(PoolExampleObject, poolExampleObjectAnother)

poolExampleObjectAndAgainAnother = objectManager:grab(PoolExampleObject)
--]]



--global variables for bounciness

floorBounciness = 9
wallBounciness = 5

local player
include("player.lua");

-- Create the floor and walls of the cube level
do
    floor = GameObjectManager:createGameObject("floor")
    floor.pc = floor:createPhysicsComponent()
    local cinfo = RigidBodyCInfo()
    cinfo.shape = PhysicsFactory:createBox(Vec3(5, 5, 0.1))
    cinfo.motionType = MotionType.Fixed
    floor.rb = floor.pc:createRigidBody(cinfo)

    local wallIndex = 0
    local createWall = function (width, angle, position)
        local wall = GameObjectManager:createGameObject("wall" .. wallIndex)
        wall.pc = wall:createPhysicsComponent()
        local cinfo = RigidBodyCInfo()
        cinfo.shape = PhysicsFactory:createBox(Vec3(width, 0.1, 2.5))
        cinfo.motionType = MotionType.Fixed
        cinfo.position = position
        cinfo.rotation = Quaternion(Vec3(0, 0, 1), angle)
        wall.rb = wall.pc:createRigidBody(cinfo)
        wallIndex = wallIndex + 1
		
		return wall
    end

    wall1=createWall(5, 90, Vec3(4.9, 0.0, 2.5))
    wall2=createWall(5, 90, Vec3(-4.9, 0.0, 2.5))
    wall3=createWall(5, 0, Vec3(0.0, 4.9, 2.5))
    wall4=createWall(5, 0, Vec3(0.0, -4.9, 2.5))
end

include("ball.lua")
--balls(name, hp, startpos, startvel)
ballInitialize("ball1", 3, Vec3(0.0, 0.0, 5.0), Vec3(1.0, 2.0, 0.0))
ballInitialize("ball2", 3, Vec3(3.0, 0.0, 5.0), Vec3(2.0, 1.0, 0.0))


-- Default update function
function update(deltaTime)
	
	for k, v in pairs(balls) do
		
	--if hitFloor was set true by the custom collision detection of the ball, a linear impulse will be applied, to let the ball move to its original height
		
		if (v.hitFloor) then
		
		v.rb:applyLinearImpulse(Vec3(0, 0, floorBounciness))
		
		v.hitFloor = false
		
		end
		
		if (v.hitWall1) then
		
		v.rb:applyLinearImpulse(Vec3(-wallBounciness, 0, 0))
		
		v.hitWall1 = false
		
		end
		
		if (v.hitWall2) then
		
		v.rb:applyLinearImpulse(Vec3(wallBounciness, 0, 0))
		
		v.hitWall2 = false
		
		end
		
		if (v.hitWall3) then
		
		v.rb:applyLinearImpulse(Vec3(0, -wallBounciness, 0))
		
		v.hitWall3 = false
		
		end
		
		if (v.hitWall4) then
		
		v.rb:applyLinearImpulse(Vec3(0, wallBounciness, 0))
		
		v.hitWall4 = false
		
		end
		
	--[[
	Each ball has to save its own last velocity at each update iteration. This allows us to check how the standard collision handling changed the velocity of the balls.
	This information will then be used, to influence the velocity of the ball when it hits different walls while keeping the right direction.
	]]--	
	

--[[	
		if (v.hitObject == 1) then
			
			currentVel = v.pc:getRigidBody():getLinearVelocity()
			comparisonVel = v.comparisonVel
			
			v.hitObject = 0
			
			if (currentVel.x<0 and comparisonVel.x>0) then
			
			v.rb:applyLinearImpulse(Vec3(-wallBounciness, 0, 0))
			
			end
						
			if(currentVel.x>0 and comparisonVel.x<0) then
			
			v.rb:applyLinearImpulse(Vec3(wallBounciness, 0, 0))
			
			end
			
			if (currentVel.y<0 and comparisonVel.y>0) then
			
			v.rb:applyLinearImpulse(Vec3(0, -wallBounciness, 0))
			
			end
			
			if(currentVel.y>0 and comparisonVel.y<0) then
			
			v.rb:applyLinearImpulse(Vec3(0, wallBounciness, 0))
			
			end
				
			--test
			
			if (comparisonVel.x>0 and currentVel.x==0) then
			
			v.rb:applyLinearImpulse(Vec3(-wallBounciness, 0, 0))
			
			end
			
			if (comparisonVel.y>0 and currentVel.y==0) then
			
			v.rb:applyLinearImpulse(Vec3(0, wallBounciness, 0))
			
			end
			
			if (comparisonVel.x>0 and currentVel.x==0) then
			
			v.rb:applyLinearImpulse(Vec3(wallBounciness, 0, 0))
			
			end
			
			if (comparisonVel.y>0 and currentVel.y==0) then
			
			v.rb:applyLinearImpulse(Vec3(0, -wallBounciness, 0))
			
			end
			
			--end of test
				
		end
		
		if (v.hitObject == 2) then
			
			v.hitObject = 1
		
		end
		
		if (v.hitObject == 3) then
		
			v.comparisonVel = v.lasterVel
			
			v.hitObject = 2 --decrease the hitObject counter by one, to start the next step of the object collision processing in the next update cycle
		
		end
		v.lasterVel =v.lastVel
		v.lastVel = v.pc:getRigidBody():getLinearVelocity()
]]--		
	end

	return EventResult.Handled
	
end

-- Register the default update function
Events.Update:registerListener(update)
