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
include("bullet.lua")

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

--[[
local ball = GameObjectManager:createGameObject("ball")
ball.pc = ball:createPhysicsComponent()
cinfo = RigidBodyCInfo()
cinfo.shape = PhysicsFactory:createSphere(0.5)
cinfo.motionType = MotionType.Dynamic
cinfo.mass = 1.0
cinfo.friction = 0.0
cinfo.angularDamping = 0.0
cinfo.restitution = 1.0
cinfo.position = Vec3(0.0, 0.0, 5.0)
cinfo.linearVelocity = Vec3(2.0, 1.0, 0.0)
ball.pc:createRigidBody(cinfo)
]]--


include("ball.lua")

ball1 = balls("ball1", 3, Vec3(0.0, 0.0, 5.0))
ball2 = balls("ball2", 3, Vec3(3.0, 0.0, 5.0))


-- Default update function
function update(deltaTime)

	return EventResult.Handled
end

-- Register the default update function
Events.Update:registerListener(update)
