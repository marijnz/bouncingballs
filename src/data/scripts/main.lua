logMessage("using main.lua")

-- Options
local options = {
    freecamera = true,
    debugDrawing = true
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
        wall.pc:createRigidBody(cinfo)
        wallIndex = wallIndex + 1
    end

    createWall(5, 90, Vec3(4.9, 0.0, 2.5))
    createWall(5, 90, Vec3(-4.9, 0.0, 2.5))
    createWall(5, 0, Vec3(0.0, 4.9, 2.5))
    createWall(5, 0, Vec3(0.0, -4.9, 2.5))
end

include("ball.lua")
--balls(name, hp, startpos, startvel)
ballInitialize("ball1", 3, Vec3(0.0, 0.0, 5.0), Vec3(1.0, 2.0, 0.0))
ballInitialize("ball2", 3, Vec3(3.0, 0.0, 5.0), Vec3(2.0, 1.0, 0.0))


-- Default update function
function update(deltaTime)
	
		for k, v in pairs(balls) do
		
		if (v.hitFloor) then
		
		v.rb:applyLinearImpulse(Vec3(0, 0, 10))
		
		v.hitFloor = false
		
		end
		
	end

	return EventResult.Handled
end

-- Register the default update function
Events.Update:registerListener(update)
