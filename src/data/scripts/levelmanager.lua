logMessage("using levelmanager.lua")

LevelManager = {}
LevelManager.__index = LevelManager

setmetatable(LevelManager, {
	__index = PoolObject,
	__call = function (cls, ...)
    self = setmetatable({}, cls)
    return self
  end,
})

function LevelManager:create()
	go = GameObjectManager:createGameObject("levelManager")
	self.go = go
end

function LevelManager:initializeLevels(levels)
	logMessage(levels[0])
	self.levels = levels
	
	currentPos = Vec3(0,0,0)
	
	for key, level in ipairs(levels) do
		level:createLevel(currentPos, key)
		self.levels[key-1].center = Vec3(currentPos)
		currentPos.y = currentPos.y - 15
		currentPos.x = currentPos.x + 15
	end
	logMessage("end")
	logMessage(self.levels[0].center)
	self:loadLevel(0)
end

function LevelManager:loadLevel(levelId)
	level = self.levels[levelId]
	logMessage("OK")
	logMessage(level.center)
	logMessage("OK")
	-- Spawn balls
	level:startLevel(level.center, levelId)
	
	-- Move camera to level center
	
end

function LevelManager:initialize() 
end
function LevelManager:dispose() 
end

function LevelManager:update(deltaTime)
	
end