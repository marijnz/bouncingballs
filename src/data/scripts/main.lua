logMessage("using main.lua")

-- Options
local options = {
    freecamera = false 
}	

PhysicsSystem:setDebugDrawingEnabled(true)

PhysicsSystem:setDebugDrawingEnabled(true)

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
include("poolsystem.lua")
poolSystem = PoolSystem()
include("poolobject.lua")
include("poolexampleobject.lua")
include("bullet.lua")

--example = PoolExampleObject()
--example.exampleDoSomething()

local player
include("player.lua");

local floor = GameObjectManager:createGameObject("floor")
floor.pc = floor:createPhysicsComponent()
local cinfo = RigidBodyCInfo()
cinfo.shape = PhysicsFactory:createBox(Vec3(5, 5, 0.1))
cinfo.motionType = MotionType.Fixed
cinfo.friction = 0.0
cinfo.angularDamping = 0.0
cinfo.restitution = 1.0
floor.pc:createRigidBody(cinfo)

local wall = GameObjectManager:createGameObject("wall")
wall.pc = wall:createPhysicsComponent()
local cinfo = RigidBodyCInfo()
cinfo.shape = PhysicsFactory:createBox(Vec3(0.1, 5, 5))
cinfo.motionType = MotionType.Fixed
cinfo.friction = 0.0
cinfo.angularDamping = 0.0
cinfo.restitution = 1.0
cinfo.position = Vec3(5.0, 0.0, 0.0)
wall.pc:createRigidBody(cinfo)



include("ball.lua")
--balls(name, hp, startpos, startvel)
balls("ball1", 3, Vec3(0.0, 0.0, 5.0), Vec3(1.0, 2.0, 0.0))
balls("ball2", 3, Vec3(3.0, 0.0, 5.0), Vec3(2.0, 1.0, 0.0))


-- Default update function
function update(deltaTime)
	

	return EventResult.Handled
end

-- Register the default update function
Events.Update:registerListener(update)
