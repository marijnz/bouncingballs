logMessage([[Initializing samples/screenray.lua
  This sample demonstrates how to cast a ray from screen coordinates.
  Simply click on the objects in the scene to see it in action.
  Main usage of the ray casting API can be found in the lines 51-63.
]])

include("utils/physicsWorld.lua")
include("utils/stateMachine.lua")
include("utils/timedStatusDisplay.lua")

PhysicsSystem:setDebugDrawingEnabled(true)
timedStatusDisplayStart()

do -- some static camera
    cam = GameObjectManager:createGameObject("cam")
    cam.cc = cam:createCameraComponent()
    cam.cc:setPosition(Vec3(0, -10, 0))
    cam.cc:setViewDirection(Vec3(0, 1, 0))
end

-- Creates a target that can be hit by a ray
function createTarget(guid, position, radius)
    local target = GameObjectManager:createGameObject(guid)
    target.physics = target:createPhysicsComponent()

    do -- create the rigid body
        local cinfo = RigidBodyCInfo()
        cinfo.motionType = MotionType.Fixed
        cinfo.position   = position
        cinfo.shape      = PhysicsFactory:createSphere(radius or 1)
        target.rigidBody = target.physics:createRigidBody(cinfo)

        -- Set the user data so we can retrieve it when the ray hits this target
        target.rigidBody:setUserData(target)
    end

    return target
end

createTarget("upper right", Vec3( 3, 0,  3))
createTarget("lower right", Vec3( 3, 0, -3))
createTarget("upper left",  Vec3(-3, 0,  3))
createTarget("lower left",  Vec3(-3, 0, -3))

Events.Update:registerListener(function(dt)
    DebugRenderer:printText(Vec2(-0.15, 0.9), "Click an object to see the ray in action.")

    local mousePos = InputHandler:getMouseNormalizedScreenPosition()

    if InputHandler:wasTriggered(Key.LButton) then
        local screenray = cam.cc:getRayForNormalizedScreenPos(mousePos)
        local ray = RayCastInput()
        ray.from = screenray.from
        ray.to   = ray.from + screenray.direction:mulScalar(1000)

        local result = physicsWorld:castRay(ray)

        if result:hasHit() then
            local target = result.hitBody:getUserData()
            timedStatusDisplay("hit " .. target:getGuid(), 2, Vec2())
        else
            timedStatusDisplay("nothing was hit...", 2, Vec2())
        end
    end

    -- Draw an accurate crosshair in yellow
    local yellow    = Color(1, 1, 0, 1)
    local magnitude = 0.025
    local hStart    = Vec2(mousePos.x - magnitude, mousePos.y)
    local hEnd      = Vec2(mousePos.x + magnitude, mousePos.y)
    local vStart    = Vec2(mousePos.x,             mousePos.y - magnitude)
    local vEnd      = Vec2(mousePos.x,             mousePos.y + magnitude)
    DebugRenderer:drawLine2D(hStart, hEnd, yellow)
    DebugRenderer:drawLine2D(vStart, vEnd, yellow)
end)
