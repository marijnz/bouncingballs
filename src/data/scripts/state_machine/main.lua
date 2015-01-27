-- Physics World
include("utils/physicsWorld.lua")

-- Enable drawing of physics geometry
PhysicsSystem:setDebugDrawingEnabled(true)

-- Camera
cam = {}
do
	cam.go = GameObjectManager:createGameObject("cam")
	cam.cc = cam.go:createCameraComponent()
	cam.cc:setPosition(Vec3(-100.0, 0.0, 0.0))
	cam.cc:lookAt(Vec3(0, 0, 0))
	cam.cc:setFar(1000)
	cam.cc:setOrthographic(true)
end
	
-- Enemy
enemy = {}
do
	enemy.go = GameObjectManager:createGameObject("enemy")
	-- physics component
	enemy.ph = enemy.go:createPhysicsComponent()
	local cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createBox(Vec3(5, 8, 5))
	cinfo.motionType = MotionType.Keyframed
	enemy.rb = enemy.ph:createRigidBody(cinfo)
	enemy.go:setComponentStates(ComponentState.Inactive)
	enemy.stateTimer = 0
	enemy.moveLeft = false
end
include("state_machine/enemyFSM.lua")

-- menu events
function menuUpdate(updateEventData)
	DebugRenderer:printText(Vec2(-0.15, 0.8), "###### main menu ######")
	DebugRenderer:printText(Vec2(-0.10, 0.7), "press Return to start")
	DebugRenderer:printText(Vec2(-0.10, 0.6), "press ESC to quit")
	return EventResult.Handled
end

-- menu state
State{
	name = "menu",
	parent = "/game",
	eventListeners = {
		update = { menuUpdate },
		leave = { nil },
	}
}

-- pause events
function pauseEnter(enterEventData)
	enemy.go:setComponentStates(ComponentState.Inactive)
	return EventResult.Handled
end
function pauseUpdate(updateEventData)
	DebugRenderer:printText(Vec2(-0.15, 0.8), "###### pause ######")
	DebugRenderer:printText(Vec2(-0.10, 0.7), "press P to continue")
	DebugRenderer:printText(Vec2(-0.10, 0.6), "press ESC for main menu")
	return EventResult.Handled
end

-- pause state
State{
	name = "pause",
	parent = "/game",
	eventListeners = {
		enter = { pauseEnter },
		update = { pauseUpdate },
	}
}

-- gameplay events
function gameplayEnter(enterEventData)
	if (enterEventData:getLeftState():getName() == "menu") then
		enemy.go:setPosition(Vec3(0, 0, 0))
		enemy.go:setRotation(Quaternion(Vec3(1, 0, 0), 0))
		enemy.moveLeft = false
	end
	return EventResult.Handled
end

-- action events
function actionEnter(enterEventData)
	enemy.go:setComponentStates(ComponentState.Active)
	return EventResult.Handled
end
function actionUpdate(updateEventData)
	DebugRenderer:printText(Vec2(-0.15, 0.8), "###### action ######")
	DebugRenderer:printText(Vec2(-0.10, 0.7), "press P to pause")
	DebugRenderer:printText(Vec2(-0.10, 0.6), "press D for dialog")
	return EventResult.Handled
end
function actionLeave(leaveEventData)
	enemy.go:setComponentStates(ComponentState.Inactive)
	return EventResult.Handled
end

-- dialog events
specialCutscene = false
function dialogEnter(enterEventData)
	if (enterEventData:getLeftState():getName() ~= "dialog") then
		specialCutscene = false
	end
	return EventResult.Handled
end
function dialogUpdate(updateEventData)
	DebugRenderer:printText(Vec2(-0.15, 0.8), "###### dialog ######")
	DebugRenderer:printText(Vec2(-0.10, 0.7), "press P to pause")
	DebugRenderer:printText(Vec2(-0.10, 0.6), "press Return to skip")
	DebugRenderer:printText(Vec2(-0.10, 0.5), "press C unlock the special cutscene -> " .. tostring(specialCutscene))
	if (InputHandler:wasTriggered(Key.C)) then
		specialCutscene = not specialCutscene
	end
	return EventResult.Handled
end
function dialogLeave(leaveEventData)
	if (specialCutscene == true) then
		leaveEventData:setNextStateByName("cutscene")
	end
	return EventResult.Handled
end

-- cutscene events
cutsceneTimer = 0
function cutsceneEnter(enterEventData)
	if (enterEventData:getLeftState():getName() ~= "cutscene") then
		cutsceneTimer = 5
	end
	return EventResult.Handled
end
function cutsceneUpdate(updateEventData)
	DebugRenderer:printText(Vec2(-0.15, 0.8), "###### cutscene ######")
	DebugRenderer:printText(Vec2(-0.10, 0.7), "press P to pause")
	DebugRenderer:printText(Vec2(-0.10, 0.6), "press Return to skip")
	DebugRenderer:printText(Vec2(-0.10, 0.5), "cutsceneTimer -> " .. string.format("%.2f", cutsceneTimer))
	cutsceneTimer = cutsceneTimer - updateEventData:getElapsedTime()
	return EventResult.Handled
end

-- gameplay state machine {action, dialog, cutscene}
StateMachine
{
	name = "gameplay",
	parent = "/game",
	eventListeners = {
		enter = { gameplayEnter },
	},
	states = {
		{
			name = "action",
			eventListeners = {
				enter = { actionEnter },
				update = { actionUpdate },
				leave = { actionLeave },
			}
		},
		{
			name = "dialog",
			eventListeners = {
				enter = { dialogEnter },
				update = { dialogUpdate },
				leave = { dialogLeave },
			}
		},
		{
			name = "cutscene",
			eventListeners = {
				enter = { cutsceneEnter },
				update = { cutsceneUpdate },
			}
		},
	},
	transitions = {
		{ from = "__enter", to = "action" },
		{ from = "action", to = "dialog", condition = function() return InputHandler:wasTriggered(Key.D) end },
		{ from = "dialog", to = "action", condition = function() return InputHandler:wasTriggered(Key.Return) end },
		{ from = "dialog", to = "cutscene", condition = function() return false end },
		{ from = "cutscene", to = "action", condition = function() return (InputHandler:wasTriggered(Key.Return) or cutsceneTimer <= 0) end },
	}
}

-- game state transitions
StateTransitions{
	parent = "/game",
	{ from = "__enter", to = "menu" },
	{ from = "menu", to = "gameplay", condition = function() return InputHandler:wasTriggered(Key.Return) end },
	{ from = "gameplay", to = "pause", condition = function() return InputHandler:wasTriggered(Key.P) end },
	{ from = "pause", to = "gameplay", condition = function() return InputHandler:wasTriggered(Key.P) end },
	{ from = "pause", to = "menu", condition = function() return InputHandler:wasTriggered(Key.Escape) end },
	{ from = "menu", to = "__leave", condition = function() return InputHandler:wasTriggered(Key.Escape) end },
}
