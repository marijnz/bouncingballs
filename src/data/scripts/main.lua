-- Options
local options = {
    freecamera = false 
}

-- Default state machine.
include("defaults/stateMachine.lua")

-- World
include("world.lua")

-- Camera
if (options.freecamera) then
    include("freecamera.lua")
else
    include("camera.lua")
end

level = GameObjectManager:createGameObject("level")
level.render = level:createRenderComponent()
level.render:setPath("data/models/cube-level.thModel")
level:setPosition(Vec3(0, 0, 0))

-- Default update function
function update(deltaTime)

	return EventResult.Handled
end

-- Register the default update function
Events.Update:registerListener(update)
