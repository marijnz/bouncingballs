logMessage([[Initializing samples/raycast.lua
  This sample demonstrates how to use the ray casting functionality of our physics API.
  Rotate the box in the center (the caster) using the left and right arrow keys.
  Once the (red) ray hits any of the targets (balls), it will become green.
  In addition to the ray, the collision normal is also drawn when the ray hits a target (ball).
  Main usage of the ray casting API can be found in the lines 83-97.
]])

include("utils/physicsWorld.lua")
include("utils/stateMachine.lua")
include("utils/freeCamera.lua")

PhysicsSystem:setDebugDrawingEnabled(true)

freeCam.cc:setPosition(Vec3(0, -10, 0))
freeCam.cc:setViewDirection(Vec3(0, 1, 0))

do
    -- The game object that casts a ray
    caster = GameObjectManager:createGameObject("caster")
    caster.render = caster:createRenderComponent()
    caster.render:setPath("data/models/box.thModel")
    caster.rotation = 0
    caster.angularSpeed = 45 -- degrees per second
end

do -- Relative data of the ray that will be cast by "caster"
    ray = {}
    ray.offset    = Vec3(0, 0, 1)
    ray.direction = Vec3(0, 0, 1)
    ray.length    = 4
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
    DebugRenderer:printText(Vec2(-0.15, 0.9), "Use the arrow keys to rotate the caster.")

    if InputHandler:isPressed(Key.Left) then
        caster.rotation = caster.rotation - caster.angularSpeed * dt
    elseif InputHandler:isPressed(Key.Right) then
        caster.rotation = caster.rotation + caster.angularSpeed * dt
    end

    -- Update the rotation of our caster
    local rotQuat = Quaternion(Vec3(0, 1, 0), caster.rotation)
    caster:setRotation(rotQuat)
    local rot = rotQuat:toMat3()

    -- do the ray casting
    local rayColor = Color(1, 0, 0, 1) -- red
    local message  = "Nothing was hit..."

    -- transform the ray into the object space of the caster
    local rayOffset    = rot:mulVec3(ray.offset)
    local rayDirection = rot:mulVec3(ray.direction)
    local rayStart     = caster:getPosition() + rayOffset
    local rayEnd       = rayStart + rayDirection:mulScalar(ray.length)

    -- create the necessary input data for the ray casting
    local rayInput = RayCastInput()
    rayInput.from  = rayStart
    rayInput.to    = rayEnd

    -- actually perform the ray cast now!
    local result = physicsWorld:castRay(rayInput)

    -- use the result for something
    if result:hasHit() then
        local target = result.hitBody:getUserData()
        rayColor = Color(0, 1, 0, 1) -- green
        message  = "Hit " .. target:getGuid()

        -- since we can determine the exact position where the ray hit the target, we shorten rayEnd to that point
        rayEnd = rayStart + rayDirection:mulScalar(ray.length * result.hitFraction)

        -- just for kicks we also draw the hit-normal
        DebugRenderer:drawArrow(rayEnd, rayEnd + result.normal, Color(1, 1, 0, 1))
    end

    DebugRenderer:drawArrow(rayStart, rayEnd, rayColor)
    DebugRenderer:printText(Vec2(-0.9, 0.6), message)
end)
