logMessage([[Initializing samples/trigger_volumes.lua
  This sample demonstrates how to use trigger volumes in the physics world.
  Check the log to see when the player enters or leaves a detector.
]])

include("utils/stateMachine.lua")
include("utils/freeCamera.lua")

do -- Create physics world
    local cinfo = WorldCInfo()
    cinfo.gravity = Vec3(0, 0, -9.81)
    cinfo.worldSize = 4000.0
    physicsWorld = PhysicsFactory:createWorld(cinfo)
    PhysicsSystem:setWorld(physicsWorld)
end

PhysicsSystem:setDebugDrawingEnabled(true)

freeCam.cc:setPosition(0, -10, 2)

do -- Create the ground
    ground = GameObjectManager:createGameObject("ground")
    ground.physics = ground:createPhysicsComponent()
    local cinfo = RigidBodyCInfo()
    cinfo.motionType = MotionType.Fixed
    cinfo.shape = PhysicsFactory:createBox(Vec3(10, 10, 0.5))
    cinfo.position = Vec3(0, 0, 0)
    cinfo.friction = 0.8
    ground.rigidBody = ground.physics:createRigidBody(cinfo)
    ground.rigidBody:setUserData(ground) -- Always a good idea
end

do -- Create the player
    player = GameObjectManager:createGameObject("player")
    -- Some general data
    player.linearSpeed = 1 -- meters per second

    -- Player needs physics, otherwise it cannot be detected by trigger volumes
    player.physics = player:createPhysicsComponent()

    local cinfo = RigidBodyCInfo()
    cinfo.mass = 1
    cinfo.motionType = MotionType.Dynamic
    cinfo.position = Vec3(0, 0, 2)
    cinfo.shape = PhysicsFactory:createSphere(1)
    cinfo.maxLinearVelocity = player.linearSpeed * 5
    player.rigidBody = player.physics:createRigidBody(cinfo)
    player.rigidBody:setUserData(player) -- Always a good idea
end

do -- Create some trigger that detects the player
    detector1 = GameObjectManager:createGameObject("detector1")
    detector1.physics = detector1:createPhysicsComponent()
    local cinfo = RigidBodyCInfo()
    cinfo.motionType = MotionType.Fixed
    cinfo.position = Vec3(-5, 0, 2)
    cinfo.shape = PhysicsFactory:createBox(Vec3(1, 1, 1))
    cinfo.isTriggerVolume = true -- The key part to turn this rigid body into a trigger
    detector1.rigidBody = detector1.physics:createRigidBody(cinfo)
    detector1.rigidBody:setUserData(detector1) -- Always a good idea
    detector1.rigidBody:getTriggerEvent():registerListener(function(eventArgs)
        logMessage("trigger event!")
        if eventArgs:getEventType() == TriggerEventType.Entered then
            detector1.detected = eventArgs:getRigidBody():getUserData()
        elseif eventArgs:getEventType() == TriggerEventType.Left then
            detector1.detected = nil
        end
    end)

    detector1.script = detector1:createScriptComponent()
    detector1.script:setUpdateFunction(function(guid, dt)
        if not detector1.detected then return end
        DebugRenderer:printText3D(detector1:getPosition() + Vec3(0, 0, 1.5),
            "Hello " .. detector1.detected:getGuid())
    end)
end

Events.Update:registerListener(function(dt)
    DebugRenderer:printText(Vec2(-0.15, 0.9), "Use the arrow keys to move the player.")

    if InputHandler:isPressed(Key.Right) then
        player.rigidBody:applyLinearImpulse(Vec3(player.linearSpeed, 0, 0))
    end
    if InputHandler:isPressed(Key.Left) then
        player.rigidBody:applyLinearImpulse(Vec3(-player.linearSpeed, 0, 0))
    end
    if InputHandler:isPressed(Key.Up) then
        player.rigidBody:applyLinearImpulse(Vec3(0, 0, player.linearSpeed))
    end
    if InputHandler:isPressed(Key.Down) then
        player.rigidBody:applyLinearImpulse(Vec3(0, 0, -player.linearSpeed))
    end
end)
