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
	self.levels = levels
	
	currentPos = Vec3(0,0,0)
	
	for key, level in ipairs(levels) do
	logMessage(key)
		self.levels[key].go = level:createLevel(currentPos, key)
		self.levels[key].center = Vec3(0,0,0)
		currentPos.y = currentPos.y - 15
		currentPos.x = currentPos.x + 15
	end
	
	self.levelProgress = 0
	self.loadLevelAfterTime = 0.1
end

function LevelManager:goNextLevel()
    if(self.isGoingNextLevel) then
        return
    end
	self.isGoingNextLevel = true
	disposeEverything()
	playerTweener = objectManager:grab(Tweener)
	playerTweener:startTween(easing.inSine, player, player:getPosition() + Vec3(0,0,20), 1, self.onPlayerDisplaced)
end

function LevelManager:onPlayerDisplaced()
	logMessage("onPlayerDisplaced")
	self = levelManager
	levelTweener = objectManager:grab(Tweener)
	levelTweener:startTween(easing.inBack, level.go, Vec3(15,-8,20), 1, self.onLevelUnloaded)
end

function LevelManager:onLevelUnloaded()
	logMessage("onLevelUnloaded")
	self = levelManager
	level = self.levels[self.currentLevelId]
	objectManager:put(Level1, level)
	self:loadLevel(self.currentLevelId + 1)
end

function LevelManager:loadLevel(levelId)
	self.isGoingNextLevel = true
	self = levelManager
	self.currentLevelId = levelId
	logMessage("id..: "..self.currentLevelId)
	level = self.levels[self.currentLevelId]
	-- Spawn balls
	
	level = self.levels[self.currentLevelId]
	
	level.go:setPosition(Vec3(-15,8,0))

	tweener = objectManager:grab(Tweener)
	
	tweener:startTween(easing.outBack, level.go, Vec3(0,0,0), 1, self.onLevelLoaded)
	
end

function LevelManager:onLevelLoaded()
	self = levelManager
	
	player:setPosition(0,0, 20)
	player:freeze()
	tweener2 = objectManager:grab(Tweener)
	tweener2:startTween(easing.outSine, player, Vec3(0,0,FLOOR_Z + 0.28), 1, self.onPlayerPlaced)
end

function LevelManager:onPlayerPlaced()
	self = levelManager
	
	level = self.levels[self.currentLevelId]
	level:startLevel(level.center, self.currentLevelId)
	player:unfreeze()
	logMessage("UNFREEZE PLAYER")
	self.isGoingNextLevel = false
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
		count = count + 1
	end
	if(count == 0) then
		self:goNextLevel()
	end
end

function LevelManager:update(deltaTime)
	if(self.loadLevelAfterTime ~= 0) then
		self.loadLevelAfterTime = self.loadLevelAfterTime + deltaTime
		if(self.loadLevelAfterTime > 5) then
			self.currentLevelId = 1
			self:loadLevel(self.currentLevelId)
			self.loadLevelAfterTime = 0
            gameLoadedBool = true
		end
	end
end

--needed to restart current level
function LevelManager:getCurrentLevelId()
	return self.currentLevelId
end

function LevelManager:setCurrentLevelId(id)
	self.currentLevelId=id
end
