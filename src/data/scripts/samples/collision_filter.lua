logMessage([[Initializing samples/collision_filter.lua
  This sample demonstrates how to set up collision filters.
]])

do -- Create physics world
    local cinfo = WorldCInfo()
    cinfo.gravity = Vec3(0, 0, -9.81)
    physicsWorld = PhysicsFactory:createWorld(cinfo)
    physicsWorld:setCollisionFilter(PhysicsFactory:createCollisionFilter_Simple())
    PhysicsSystem:setWorld(physicsWorld)
end

include("utils/stateMachine.lua")
include("utils/freeCamera.lua")

PhysicsSystem:setDebugDrawingEnabled(true)

freeCam.cc:setPosition(Vec3(-7, 0, 25.1))
freeCam.cc:setUpDirection(Vec3(0, 1, 0))
freeCam.cc:setViewDirection(Vec3(0, 0, -1))

if false then -- Create the player
    player = GameObjectManager:createGameObject("player [0x55555555]")
    player.render = player:createRenderComponent()
    player.render:setPath("data/models/ball.thModel")

    player.physics = player:createPhysicsComponent()
    local cinfo = RigidBodyCInfo()
    cinfo.position = Vec3(0, 0, 1)
    cinfo.mass = 1
    cinfo.motionType = MotionType.Dynamic
    --cinfo.maxLinearVelocity = 100
    cinfo.shape = PhysicsFactory:createSphere(1)
    cinfo.collisionFilterInfo = 0x55555555 -- 0101 0101 0101 0101 0101 0101 0101 0101
    player.rigidBody = player.physics:createRigidBody(cinfo)
    player.rigidBody:setUserData(player) -- Always a good idea

    player.script = player:createScriptComponent()
    player.script:setUpdateFunction(function(guid, dt)
        player:update(dt)
    end)

    function player.update(self, dt)
        local imp = Vec3()
        local magnitude = 1.5
        if InputHandler:isPressed(Key.Up) then
            imp.y = imp.y + magnitude
        end
        if InputHandler:isPressed(Key.Down) then
            imp.y = imp.y - magnitude
        end
        if InputHandler:isPressed(Key.Left) then
            imp.x = imp.x - magnitude
        end
        if InputHandler:isPressed(Key.Right) then
            imp.x = imp.x + magnitude
        end

        if imp:squaredLength() > 0.05 then -- non-zero check
            self.rigidBody:applyLinearImpulse(imp:normalized())
        end
    end
end

function createBorder(guid, pos, halfExtends)
    local border = GameObjectManager:createGameObject(guid)
    border.physics = border:createPhysicsComponent()
    local cinfo = RigidBodyCInfo()
    cinfo.position = pos
    cinfo.motionType = MotionType.Fixed
    cinfo.shape = PhysicsFactory:createBox(halfExtends)
    cinfo.collisionFilterInfo = 0xFFFF -- 1111 1111 1111 1111 1111 1111 1111 1111
    border.rigidBody = border.physics:createRigidBody(cinfo)
    border.rigidBody:setUserData(border) -- Always a good idea
    return border
end

-- Used by the obstacle that is currently under player control
function handlePlayerInput(self, dt)
    local imp = Vec3()
    local magnitude = 1.5
    if InputHandler:isPressed(Key.Up) then
        imp.y = imp.y + magnitude
    end
    if InputHandler:isPressed(Key.Down) then
        imp.y = imp.y - magnitude
    end
    if InputHandler:isPressed(Key.Left) then
        imp.x = imp.x - magnitude
    end
    if InputHandler:isPressed(Key.Right) then
        imp.x = imp.x + magnitude
    end

    if imp:squaredLength() > 0.05 then -- non-zero check
        self.rigidBody:applyLinearImpulse(imp:normalized())
    end
end

obstacles = {}
function createObstacle(guid, pos, filter)
    local obstacle = GameObjectManager:createGameObject(guid)
    obstacle.physics = obstacle:createPhysicsComponent()
    local cinfo = RigidBodyCInfo()
    cinfo.position = pos
    cinfo.mass = 1
    cinfo.restitution = 1
    cinfo.motionType = MotionType.Dynamic
    cinfo.shape = PhysicsFactory:createSphere(1)
    cinfo.collisionFilterInfo = filter -- key part here
    obstacle.rigidBody = obstacle.physics:createRigidBody(cinfo)
    obstacle.rigidBody:setUserData(obstacle)

    obstacle.script = obstacle:createScriptComponent()
    obstacle.script:setUpdateFunction(function(guid, dt)
        obstacle:update(dt)
    end)

    function obstacle.update(self, dt)
        DebugRenderer:printText3D(self:getPosition() + Vec3(0, 1.5, 0), self:getGuid())
    end
    obstacle.handlePlayerInput = handlePlayerInput

    table.insert(obstacles, obstacle)
end

local a = 18 -- area
local w = 1.2 -- width reduction
createBorder("ground", Vec3(  0,  0,   -1), Vec3(a, a/w, 1))
createBorder("north",  Vec3(  0,  a/w,  a), Vec3(a, 1,   a))
createBorder("west",   Vec3( -a,  0,    a), Vec3(1, a/w, a))
createBorder("south",  Vec3(  0, -a/w,  a), Vec3(a, 1,   a))
createBorder("east",   Vec3(  a,  0,    a), Vec3(1, a/w, a))

-- Note: If you create an object with collision filter 0,
-- it will not collide with anything.
createObstacle("ball 1 [0001]",   Vec3(-8,  8, 1), 0x1)
createObstacle("ball 2 [0010]",   Vec3( 0,  8, 1), 0x2)
createObstacle("ball 3 [0011]",   Vec3( 8,  8, 1), 0x3)
createObstacle("ball 4 [0100]",   Vec3(-8,  0, 1), 0x4)
createObstacle("ball 5 [0101]",   Vec3( 0,  0, 1), 0x5)
createObstacle("ball 6 [0110]",   Vec3( 8,  0, 1), 0x6)
createObstacle("ball 7 [0111]",   Vec3(-8, -8, 1), 0x7)
createObstacle("ball 8 [1000]",   Vec3( 0, -8, 1), 0x8)
createObstacle("ball 9 [1001]",   Vec3( 8, -8, 1), 0x9)

controlledObstacle = obstacles[1]

Events.Update:registerListener(function(dt)

    -- Check whether the user wants to change the obstacle they are controlling
    if     InputHandler:wasTriggered(Key._1) then controlledObstacle = obstacles[1]
    elseif InputHandler:wasTriggered(Key._2) then controlledObstacle = obstacles[2]
    elseif InputHandler:wasTriggered(Key._3) then controlledObstacle = obstacles[3]
    elseif InputHandler:wasTriggered(Key._4) then controlledObstacle = obstacles[4]
    elseif InputHandler:wasTriggered(Key._5) then controlledObstacle = obstacles[5]
    elseif InputHandler:wasTriggered(Key._6) then controlledObstacle = obstacles[6]
    elseif InputHandler:wasTriggered(Key._7) then controlledObstacle = obstacles[7]
    elseif InputHandler:wasTriggered(Key._8) then controlledObstacle = obstacles[8]
    elseif InputHandler:wasTriggered(Key._9) then controlledObstacle = obstacles[9]
    end

    -- update the active obstacle according to user input
    controlledObstacle:handlePlayerInput(dt)

    DebugRenderer:printText(Vec2(-0.98, 0.2),
[[Use the arrow keys to move the currently selected ball.
Press keys 1-9 to choose which ball to control.
Only those entities that have the same bits set in
their [collision filter info] will collide with each other.
ball 7 [0111] for example will collide with every ball
except for ball 8 [1000].]])
end)
