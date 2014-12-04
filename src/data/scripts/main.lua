logMessage("using main.lua")

-- Options
local options = {
    freecamera = false 
}

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

level.ph = ship.go:createPhysicsComponent()
local cinfo = RigidBodyCInfo()
cinfo.shape = PhysicsFactory:createBox(Vec3(5, 8, 5))
cinfo.mass = 50
cinfo.maxLinearVelocity = 100
cinfo.motionType = MotionType.Dynamic
ship.rb = ship.ph:createRigidBody(cinfo)

local player
include("player.lua");

-- Default update function
function update(deltaTime)

	return EventResult.Handled
end

-- Register the default update function
Events.Update:registerListener(update)
