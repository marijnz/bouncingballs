logMessage("using main.lua")

FLOOR_Z = 0.19
CEILING_Z = 4.6 

USERDATA_TYPE_FLOOR = 1
USERDATA_TYPE_WALL = 2 
USERDATA_TYPE_BALL = 3
USERDATA_TYPE_PLAYER = 4
USERDATA_TYPE_HOOKSHOT = 5

-- Options 
local options = {
    freecamera = false,
	debugDrawing = false 
}

PhysicsSystem:setDebugDrawingEnabled(options.debugDrawing)

--gameOver boolean for the state machine
gameOverBool = false
gameLoadedBool = false

-- Default state machine.
include("stateMachine.lua")

-- World
local world
include("world.lua")

-- Some mathematical functions
include("util.lua")

-- Classes
easing = require("easing")

function easingVec3(easeMethod, t, b, c, d)
	result = Vec3()
	result.x = easeMethod(t ,b.x ,c.x ,d)
	result.y = easeMethod(t ,b.y ,c.y ,d)
	result.z = easeMethod(t ,b.z ,c.z ,d)
	return result
end

include("tweener.lua")
include("objectManager.lua")
include("levelmanager.lua")
include("levelbuilderhelper.lua")
include("poolobject.lua")
include("hookshot.lua")
include("ball.lua")
include("bigBall.lua")
include("mediumBall.lua")
include("smallBall.lua")
include("player.lua");
include("levels/level1.lua")
include("levels/level2.lua")
include("levels/level3.lua")

-- Player
player:setPosition(0,0, 20)
--player:freeze()

-- Camera
local camera
if (options.freecamera) then
    include("freecamera.lua")
else
    include("camera.lua")
	objectManager:addPool(Camera, 1)
	mainCamera = objectManager:grab(Camera)
end

objectManager:addPool(Hookshot, 5)
objectManager:addPool(Tweener, 20)

objectManager:addPool(LevelManager, 1)
levelManager = objectManager:grab(LevelManager)

objectManager:addPool(BigBall, 1)
objectManager:addPool(MediumBall, 4)
objectManager:addPool(SmallBall, 4 * 2)

objectManager:addPool(Level1, 1)
objectManager:addPool(Level2, 1)
objectManager:addPool(Level3, 1)
levels = {}
levels[0] = nil --Yea, this should be solved
levels[1] = objectManager:grab(Level1)
levels[2] = objectManager:grab(Level2)
levels[3] = objectManager:grab(Level3)
levelManager:initializeLevels(levels)

-- Default update function
function update(deltaTime)
	return EventResult.Handled
end

-- Register the default update function
Events.Update:registerListener(update)
