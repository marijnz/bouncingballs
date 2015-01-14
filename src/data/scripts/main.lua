logMessage("using main.lua")

FLOOR_Z = 0.19
CEILING_Z = 4.6 

-- Options 
local options = {
    freecamera = false,
	debugDrawing = false
}

function BIT(x)
	return bit32.lshift(1, x)
end

COL_NOTHING = 0 -- <Collide with nothing
COL_PLAYER = BIT(0) -- <Collide with player
COL_BALL = BIT(1) -- <Collide with balls
COL_LEVEL = BIT(2) -- <Collide with level
COL_BULLET = BIT(3) -- <Collide with bullets

PhysicsSystem:setDebugDrawingEnabled(options.debugDrawing)

--gameOver boolean for the state machine
gameOverBool = false

-- Default state machine.
include("stateMachine.lua")

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

-- Some mathematical functions
include("util.lua")

-- Classes
include("objectManager.lua")
include("poolobject.lua")
include("bullet.lua")
objectManager:addPool(Bullet, 5)

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
        wall.rb = wall.pc:createRigidBody(cinfo)
        wallIndex = wallIndex + 1
		
		return wall
    end

    wall1=createWall(5, 90, Vec3(4.9, 0.0, 2.5))
    wall2=createWall(5, 90, Vec3(-4.9, 0.0, 2.5))
    wall3=createWall(5, 0, Vec3(0.0, 4.9, 2.5))
    wall4=createWall(5, 0, Vec3(0.0, -4.9, 2.5))
end

-- Global bounciness
floorBounciness=9
wallBounciness=5

include("ball.lua")
include("mediumBall.lua")
include("smallBall.lua")

-- Ball initialization
do
	objectManager:addPool(MediumBall, 2)
	local ball1 = objectManager:grab(MediumBall)
	ball1:setInitialPositionAndMovement(Vec3(0.0, 0.0, 5.0), Vec3(2.0, 2.0, 0.0))
	local ball2 = objectManager:grab(MediumBall)
	ball2:setInitialPositionAndMovement(Vec3(3.0, 0.0, 5.0), Vec3(-2.0, -2.0, 0.0))

	objectManager:addPool(SmallBall, 4)
	
end

-- Default update function
function update(deltaTime)
	return EventResult.Handled
end

-- Update function for the game Over state
function gameOverupdate(deltaTime)
	return EventResult.Handled
end

-- Register the default update function
Events.Update:registerListener(update)
