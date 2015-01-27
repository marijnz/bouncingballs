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
	self.balls = {}
end

function LevelManager:initializeLevels(levels)
	logMessage(levels[0])
	self.levels = levels
	
	currentPos = Vec3(0,0,0)
	
	for key, level in ipairs(levels) do
		self.levels[key-1].go = level:createLevel(currentPos, key)
		self.levels[key-1].center = Vec3(currentPos)
		currentPos.y = currentPos.y - 15
		currentPos.x = currentPos.x + 15
	end
	self.levelProgress = 0
	self.loadLevelAfterTime = 0.1
end

function LevelManager:goNextLevel()
	
	tweener = objectManager:grab(Tweener)
	
	tweener:startTween(easing.inOutSine, player, Vec3(0,0,20), 1, self.onPlayerDisplaced)
end

function LevelManager:onPlayerDisplaced()
	self = levelManager
	tweener:startTween(easing.inOutSine, level.go, Vec3(15,-8,20), 1, self.onLevelUnloaded)

end

function LevelManager:onLevelUnloaded()
	self = levelManager
	level = self.levels[self.currentLevelId]
	objectManager:put(level)
	loadLevel(self.currentLevelId + 1)
end

function LevelManager:loadLevel(levelId)
	self.currentLevelId = levelId
	
	level = self.levels[self.currentLevelId]
	logMessage(level.center)
	-- Spawn balls
	
	level.go:setPosition(Vec3(-15,8,0))

	tweener = objectManager:grab(Tweener)
	
	tweener:startTween(easing.inOutSine, level.go, Vec3(0,0,0), 1, self.onLevelLoaded)
	
end

function LevelManager:onLevelLoaded()
	self = levelManager
	
	player:freeze()
	tweener2 = objectManager:grab(Tweener)
	tweener2:startTween(easing.inOutSine, player, Vec3(0,0,FLOOR_Z + 0.25), 1, self.onPlayerPlaced)
end

function LevelManager:onPlayerPlaced()
	self = levelManager
	
	level = self.levels[self.currentLevelId]
	level:startLevel(level.center, self.currentLevelId)
	player:unfreeze()
	logMessage("UNFREEZE PLAYER")
end

function LevelManager:addBall(ball)
	rawset(self.balls, "ball" .. ball.uniqueIdentifier, ball)
end

function LevelManager:removeBall(ball)
	rawset(self.balls, "ball" .. ball.uniqueIdentifier, nil)
	self:checkAndRespondIfLevelIsDone()
end

function LevelManager:initialize() 
end
function LevelManager:dispose() 
end

function LevelManager:checkAndRespondIfLevelIsDone()
count = 0
	for k, v in pairs(self.balls) do
		count = count + 15
	end
	if(count == 0) then
		-- Move camera to next level
	end
end

function LevelManager:update(deltaTime)
	if(self.loadLevelAfterTime ~= 0) then
		self.loadLevelAfterTime = self.loadLevelAfterTime + deltaTime
		if(self.loadLevelAfterTime > 2) then		
			self:loadLevel(0)
			self.loadLevelAfterTime = 0
		end
	end
end