-- Use the default physics world.
include("utils/physicsWorld.lua")

-- Use the default state machine.
include("utils/stateMachine.lua")


-- Enable wireframe drawing of physics geometry
PhysicsSystem:setDebugDrawingEnabled(true)


-- Default update function
function update(deltaTime)
	
	return EventResult.Handled
end

-- Register the default update function
Events.Update:registerListener(update)
