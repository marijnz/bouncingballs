logMessage([[Initializing samples/game_object_abstraction.lua
  This sample demonstrates a possible implementation of an abstraction of the game object creation process.
  It is very useful to abstract this in order to have a more uniform and convenient interface for creating game objects.
  For example, to some people it is unclear where to set the initial position of a game object. Using this abstraction mechanism, _you_ have this under control.
  Another advantage is the fact that you can support higher level constructs in the same interface, such as the creation of a 'turret', if this exists in your game. Just check the code sample below.
  Note: Don't be intimidated. Yes, this is not complete and it's already at > 250 lines of Lua code, but you can always split this up into several files to make your life easier.
]])

include("utils/stateMachine.lua")
include("utils/freeCamera.lua")

do -- Physics world
    -- We want some gravity, so we create the physics world ourselves
    local cinfo = WorldCInfo()
    cinfo.gravity = Vec3(0, 0, -9.81)
    cinfo.worldSize = 2000.0
    physicsWorld = PhysicsFactory:createWorld(cinfo)
    PhysicsSystem:setWorld(physicsWorld)
end

PhysicsSystem:setDebugDrawingEnabled(true)

freeCam.cc:setPosition(Vec3(0, -10, 5))
freeCam.cc:lookAt(Vec3(0, 0, 0))

-- This table can be called to create a new game object.
-- It also acts as the primary container for all created game objects.
-- I.e. you can access your game objects using GameObject[<guid>]
GameObject = {}
GameObject = setmetatable(GameObject, GameObject)

-- This table containins all functions that will be called by the GameObject functor.
GameObjectHelpers = {}

-- This function will be used to create a complete game object.
-- It expects a table as input and looks for inner tables in it. For each of these inner tables, a function is looked up in the global table 'GameObjectHelpers' that has the same name.
-- I.e. if cinfo.hello = {} exists, GameObjectHelpers.hello(...) is called.
function GameObject.__call(self, cinfo)
    assert(cinfo.guid, "Missing guid for game object")
    assert(GameObject[cinfo.guid] == nil, "Game object with guid = \"" .. cinfo.guid .. "\" already exists")
    local go = GameObjectManager:createGameObject(cinfo.guid)
    GameObject[cinfo.guid] = go

    for key, value in pairs(cinfo) do
        -- If the entry is a table and we didn't process it yet...
        if type(value) == "table" and not value._processed then
            -- ... get a helper that has the same name as the table entry ...
            local helper = GameObjectHelpers[key]
            -- ... and call it, if it exists.
            if helper then
                helper(go, cinfo)
            end
            value._processed = true
        end
    end

    -- Every game object gets a script function
    go.script = go:createScriptComponent()
    go.script:setUpdateFunction(function(guid, dt)
        if go.update then
            go:update(dt)
        end
    end)

    -- Set some initial state, if the caller wants us to
    if cinfo.state then
        go:setComponentStates(cinfo.state)
    end

    go:setPosition(cinfo.position or Vec3(0, 0, 0)) -- the given position or the zero vector
    go:setRotation(cinfo.rotation or Quaternion())  -- the given rotation or identity

    return go
end

--------------------------------------------------------------------------------
--- Fill the GameObjectHelpers table with functors that create components.   ---
--------------------------------------------------------------------------------

-- Creates a physics component and stores it in go.physics.
-- The rigid body is stored in go.physics.rigidBody.
function GameObjectHelpers.physics(go, cinfo)
    -- Guard against multiple calls
    if cinfo.physics._processed then return end

    go.physics = go:createPhysicsComponent()
    local rbInfo = RigidBodyCInfo()
    rbInfo.position = cinfo.position or cinfo.physics.position or rbInfo.position
    for k, v in pairs(rbInfo) do
        rbInfo[k] = cinfo.physics[k] or v
    end
    go.physics.rigidBody = go.physics:createRigidBody(rbInfo)
    go.physics.rigidBody:setUserData(go) -- Always a good idea

    if cinfo.physics.state then
        go.physics:setState(cinfo.physics.state)
    end

    -- Mark the physics entry as processed
    cinfo.physics._processed = true
end

-- Creates a render component and stores it in go.render.
function GameObjectHelpers.render(go, cinfo)
    -- Guard against multiple calls
    if cinfo.render._processed then return end

    go.render = go:createRenderComponent()
    if cinfo.render.path then
        go.render:setPath(cinfo.render.path)
    end

    if cinfo.render.state then
        go.render:setState(cinfo.render.state)
    end

    -- Mark the render entry as processed
    cinfo.render._processed = true
end

--------------------------------------------------------------------------------
--- Actually create some working game objects!                               ---
--------------------------------------------------------------------------------

-- Create the ground object by directly passing a table
local ground = GameObject{
    guid = "ground",
    position = Vec3(0, 0, 0),
    physics = {
        motionType = MotionType.Fixed,
        shape      = PhysicsFactory:createBox(Vec3(5, 5, 0.25))
    },
}

-- Some ball that will be bounding off the ground
-- We create this game object by first specifying a construction table,
-- defining some functions on it, and then passing it to the GameObject functor
local ball = GameObject{
    guid = "ball",
    position = Vec3(0, 0, 5),
    physics = {
        mass          = 1.337,
        motionType    = MotionType.Dynamic,
        shape         = PhysicsFactory:createSphere(1),
        linearDamping = 0
    },
    render = { path = "data/models/ball.thModel" },
}
function ball.update(self, dt)
    -- Jump is space was pressed
    if InputHandler:wasTriggered(Key.Space) then
        self.physics.rigidBody:applyLinearImpulse(Vec3(0, 0, 10))
    end
end

--------------------------------------------------------------------------------
--- Extend the game object abstraction to support a rotating 'laser'         ---
--------------------------------------------------------------------------------

-- Shoots a laser in some direction (view-dir by default)
function GameObjectHelpers.laser(go, cinfo)
    -- If there is no render component specified by the user, we create a default one
    if not cinfo.render then
        cinfo.render = {
            path = "data/models/ball.thModel"
        }
    end
    -- Note: It is safe to call this method multiple times.
    GameObjectHelpers.render(go, cinfo)

    go.laser = {}
    go.laser.angularSpeed = cinfo.laser.angularSpeed or Color(1, 0, 0, 1) -- degrees per second
    go.laser.angle        = cinfo.laser.angle        or 0 -- initial rotation
    go.laser.up           = cinfo.laser.up           or Vec3(0, 0, 1) -- up vector
    go.laser.color        = cinfo.laser.color        or Color(1, 0, 0, 1) -- red as default color
    go.laser.offset       = cinfo.laser.offset       or Vec3(0, 0, 0)
    go.laser.direction    = cinfo.laser.direction    or go:getViewDirection()
    go.laser.length       = cinfo.laser.length       or 10

    function go.laser.update(self, dt)
        self.laser.angle = self.laser.angle + self.laser.angularSpeed * dt
        local rotation = Quaternion(self.laser.up, self.laser.angle)
        self:setRotation(rotation)
        rotation = rotation:toMat3()

        -- Apply the rotation to our relative data
        local offset    = rotation:mulVec3(self.laser.offset)
        local direction = rotation:mulVec3(self.laser.direction)

        local laserStart = self:getPosition() + offset
        local laserEnd   = laserStart + direction:mulScalar(self.laser.length)

        -- At this point we could do some ray-casts, etc... whatever a turret does.

        DebugRenderer:drawLine3D(laserStart, laserEnd, self.laser.color)
    end
end

-- Now lets create some spherical, self-rotating turret with a laser!
GameObject{
    guid = "turret",
    position = Vec3(3, -3, 1.25),
    laser = {
        angularSpeed = 60,
        offset       = Vec3(0, -1, 0), -- negative unit-y
        direction    = Vec3(0, -1, 0), -- negative unit-y
    }
}
-- Set the laser update function as our main update function.
-- If the laser is invisible, it is because this function wasn't called.
GameObject.turret.update = GameObject.turret.laser.update

--------------------------------------------------------------------------------
--- Extend the game object abstraction with something silly and proove that  ---
--- it works as advertised.                                                  ---
--------------------------------------------------------------------------------

function GameObjectHelpers.foobar(go, cinfo)
    logMessage("trolling " .. go:getGuid() .. "...")
    go.foobar = "trolololo"
end

-- This game object is invisible!
GameObject{
    guid = "baz",
    foobar = {}
}

Events.Update:registerListener(function(dt)
    DebugRenderer:printText(Vec2(-0.9, -0.3), "foobar? " .. GameObject.baz.foobar)
end)

--------------------------------------------------------------------------------
--- And just for fun, we list all game objects                               ---
--------------------------------------------------------------------------------

Events.Update:registerListener(function(dt)
    local message = "All game objects:"
    for key, value in pairs(GameObject) do
        -- ignore entries that start with two underscores (__)
        if not string.startsWith(key, "__") then
            message = message .. "\n  " .. key .. " at " .. tostring(value:getPosition(), "vec3")
        end
    end
    DebugRenderer:printText(Vec2(-0.9, 0.5), message)
end)

-- Draw the origin for orientation
Events.Update:registerListener(function(dt)
    DebugRenderer:drawOrigin()
end)
