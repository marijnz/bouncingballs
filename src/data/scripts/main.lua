logMessage("using main.lua")

FLOOR_Z = 0.19
CEILING_Z = 4.6 

USERDATA_TYPE_FLOOR = 1
USERDATA_TYPE_WALL = 2 
USERDATA_TYPE_BALL = 3
USERDATA_TYPE_PLAYER = 4

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

-- Camera
local camera
if (options.freecamera) then
    include("freecamera.lua")
else
    include("camera.lua")
end

-- Some mathematical functions
include("util.lua")

-- Classes
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

objectManager:addPool(Hookshot, 5)

objectManager:addPool(LevelManager, 1)
levelManager = objectManager:grab(LevelManager)

objectManager:addPool(MediumBall, 2)
objectManager:addPool(SmallBall, 2 * 2)

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
