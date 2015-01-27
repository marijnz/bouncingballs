
print("Initializing render component test...")

include("utils/freeCamera.lua")
include("utils/physicsWorld.lua")
include("utils/stateMachine.lua")

PhysicsSystem:setDebugDrawingEnabled(true)

freeCam.cc:setPosition(Vec3(200.0, 30.0, 50.0))
freeCam.cc:lookAt(Vec3(0.0, 0.0, 0.0))

distanceFromCamera = 300.0

-- PhysicsDebugView
PhysicsSystem:setDebugDrawingEnabled(true)

do
	theObject = GameObjectManager:createGameObject("theObject")

	-- render component
	theObject.rc = theObject:createRenderComponent()
	theObject.rc:setPath("data/models/barbarian/barbarianAxe.thModel")

	-- physics component
	theObject.pc = theObject:createPhysicsComponent()
	local cinfo = RigidBodyCInfo()
	local box = PhysicsFactory:createBox(Vec3(5, 30, 50))
	cinfo.shape = PhysicsFactory:createConvexTranslateShape(box, Vec3(0, -50, -10))
	cinfo.motionType = MotionType.Fixed
	theObject.pc.rb = theObject.pc:createRigidBody(cinfo)

end

local rotation = 0
function update(elapsedTime)

	local rotationSpeed = 50
	rotation = rotation + rotationSpeed * elapsedTime
	if (rotation > 180) then
		rotation = rotation - 360
	end
	if (rotation < -180) then
		rotation = rotation + 360
	end
	theObject:setRotation(Quaternion(Vec3(1, 0, 0), rotation))

	return EventResult.Handled
end

Events.Update:registerListener(update)

print("Finished render component test initialization.")
