
include("utils/timedStatusDisplay.lua")

include("zombie_prototype/utils.lua")
include("zombie_prototype/player.lua")
include("zombie_prototype/guns.lua")
include("zombie_prototype/zombie.lua")

-- physics world
local cinfo = WorldCInfo()
cinfo.gravity = Vec3(0, 0, -9.81)
cinfo.worldSize = 5000
world = PhysicsFactory:createWorld(cinfo)
world:setCollisionFilter(PhysicsFactory:createCollisionFilter_Simple())
PhysicsSystem:setWorld(world)
PhysicsSystem:setDebugDrawingEnabled(true)

-- sound banks
SoundSystem:loadLibrary(".\\data\\sound\\Master Bank.bank")
SoundSystem:loadLibrary(".\\data\\sound\\Master Bank.bank.strings")
SoundSystem:loadLibrary(".\\data\\sound\\Weapons.bank")
SoundSystem:loadLibrary(".\\data\\sound\\UI_Menu.bank")
SoundSystem:loadLibrary(".\\data\\sound\\Surround_Ambience.bank")
SoundSystem:loadLibrary(".\\data\\sound\\Character.bank")

-- random seed
math.randomseed(os.time())

-- ground
ground = createCollisionBox("ground", Vec3(2500, 2500, 10), Vec3(0, 0, -10))

-- player
player = createPlayer("player")
player:setActive(false)

-- zombies
zombies = {}
for i = 1, ZOMBIE_COUNT do
	zombies[i] = createZombie("zombie_" .. i)
	zombies[i]:setActive(false)
end
zombies.closest = ZOMBIE_SPAWN_DISTANCE[2]

function zombieFSMEnter(enterData)
	for _, v in ipairs(zombies) do
		v:initialize()
	end
	return EventResult.Handled
end

function titleScreenUpdate(updateData)
	DebugRenderer:printText(Vec2(-0.2, 0.5), "Zombie Shooter")
	DebugRenderer:printText(Vec2(-0.2, 0.4), "Press Return or Start to play!")
	return EventResult.Handled
end

function gameplayEnter(enterData)
	player:setActive(true)
	zombies.spawnDelay = ZOMBIE_SPAWN_DELAY_MAX
	zombies.spawnTimer = 0
	zombies.closest = ZOMBIE_SPAWN_DISTANCE[2]
	return EventResult.Handled
end

function gameplayUpdate(updateData)
	-- spawn zombies
	zombies.spawnTimer = zombies.spawnTimer - updateData:getElapsedTime()
	if (zombies.spawnTimer <= 0) then
		zombies.spawnDelay = math.clamp(zombies.spawnDelay - ZOMBIE_SPAWN_DELAY_REDUCTION, ZOMBIE_SPAWN_DELAY_MIN, ZOMBIE_SPAWN_DELAY_MAX)
		zombies.spawnTimer = zombies.spawnDelay
		for _, v in ipairs(zombies) do
			if (v.canSpawn) then
				v:spawn()
				break
			end
		end
	end
	
	-- check loose condition
	for _, v in ipairs(zombies) do
		if (not v.canSpawn) then
			local distance = v:getPosition():length()
			if (distance < zombies.closest) then
				zombies.closest = distance
			end
		end
	end
	
	DebugRenderer:printText(Vec2(0.6, 0.85), "Score: " .. player.score)
	return EventResult.Handled
end

function gameplayLeave(leaveData)
	player:setActive(false)
	for _, v in ipairs(zombies) do
		v:setActive(false)
	end
	return EventResult.Handled
end

function gameOverUpdate(updateData)
	DebugRenderer:printText(Vec2(-0.2, 0.6), "GAME OVER")
	DebugRenderer:printText(Vec2(-0.2, 0.5), "Score:     " .. player.score)
	DebugRenderer:printText(Vec2(-0.2, 0.4), "Press Return or Start to continue!")
	return EventResult.Handled
end

-- global state machine
StateMachine{
	name = "zombieFSM",
	parent = "/game",
	states =
	{
		{
			name = "titleScreen",
			eventListeners = {
				update = { titleScreenUpdate }
			}
		},
		{
			name = "gameplay",
			eventListeners = {
				enter  = { gameplayEnter },
				update = { gameplayUpdate },
				leave  = { gameplayLeave }
			}
		},
		{
			name = "gameOver",
			eventListeners = {
				update = { gameOverUpdate }
			}
		}
	},
	transitions =
	{
		{ from = "__enter", to = "titleScreen" },
		{ from = "titleScreen", to = "gameplay", condition = function() return buttonOrKeyPressed(Button.Start, Key.Return) end },
		{ from = "gameplay", to = "gameOver", condition = function() return zombies.closest <= 50 end },
		{ from = "gameOver", to = "titleScreen", condition = function() return buttonOrKeyPressed(Button.Start, Key.Return) end }
	},
	eventListeners =
	{
		enter = { zombieFSMEnter }
	}
}

StateTransitions{
	parent = "/game",
	{ from = "__enter", to = "zombieFSM" },
	{ from = "zombieFSM", to = "__leave", condition = function() return InputHandler:wasTriggered(Key.Escape) or bit32.btest(InputHandler:gamepad(0):buttonsTriggered(), Button.Back) end }
}

timedStatusDisplayStart()
