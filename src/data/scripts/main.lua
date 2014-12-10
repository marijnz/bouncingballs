logMessage("using main.lua")

-- Options
local options = {
    freecamera = false 
}	

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

-- Default update function
function update(deltaTime)

	return EventResult.Handled
end

-- Register the default update function
Events.Update:registerListener(update)
