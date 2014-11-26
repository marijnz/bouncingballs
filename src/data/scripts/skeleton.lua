-- Use the default physics world.
include("defaults/physicsWorld.lua")

-- Use the default state machine.
include("defaults/stateMachine.lua")

-- Enable wireframe drawing of physics geometry
-- PhysicsSystem:setDebugDrawingEnabled(true)




bar = GameObjectManager:createGameObject("bar")
function bar.update ()
end

bar.physics = bar:createPhysicsComponent();
local cinfo = RigidBodyCInfo()
cinfo.shape = PhysicsFactory:createBox(0.2, 0.2, 1.05) -- 40cm width and depth, 2.1m height
cinfo.motionType = MotionType.Keyframed
cinfo.restitution = 1.0
bar.rigidbody = bar.physics:createRigidBody(cinfo)

bar.script = bar:createScriptComponent()
bar:getScriptComponent():setUpdateFunction(bar.update)

bar.render = bar:createRenderComponent()
bar.render:setPath("data/models/barbarian/barbarianstatic.thModel")

bar:setPosition(Vec3(0, 0, 0))
bar.score = 0


-- Create and position the basic camera.
local cam = GameObjectManager:createGameObject("camera")
cameraComponent = cam:createCameraComponent()
local camLookAt = Vec3(0.0, 0.0, 1.0)
cameraComponent:setPosition(camLookAt + Vec3(15, 0, 0))
cameraComponent:lookAt(camLookAt)
cameraComponent:setState(ComponentState.Active)

-- Default update function
function update(deltaTime)
	
	return EventResult.Handled
end

-- Register the default update function
Events.Update:registerListener(update)
