logMessage("using main.lua")

FLOOR_Z = 0.19
CEILING_Z = 4.6 

-- Options 
local options = {
    freecamera = false,
	debugDrawing = false
}

PhysicsSystem:setDebugDrawingEnabled(options.debugDrawing)

--gameOver boolean for the state machine
gameOverBool = false

-- Default state machine.
include("stateMachine.lua")

-- World
local world
include("world.lua")



-- Player
local player

-- Some mathematical functions
include("util.lua")

-- Global bounciness
floorBounciness=9
wallBounciness=5

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
include("mediumBall.lua")
include("smallBall.lua")
include("player.lua");
include("levels/level1.lua")

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
objectManager:addPool(Tweener, 5)

objectManager:addPool(LevelManager, 1)
levelManager = objectManager:grab(LevelManager)



objectManager:addPool(MediumBall, 2)
objectManager:addPool(SmallBall, 2*2)



levels = {}
levels[0] = Level1()
levels[1] = Level1()
levels[2] = Level1()
logMessage(levels[0])
levelManager:initializeLevels(levels)




-- Default update function
function update(deltaTime)
	return EventResult.Handled
end

-- Update function for the game Over state
function gameOverupdate(deltaTime)
	return EventResult.Handled
end



-- Register the default update function
Events.Update:registerListener(update)
