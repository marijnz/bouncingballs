
do -- Physics world
	local cinfo = WorldCInfo()
	cinfo.gravity = Vec3(0, 0, 0)
	cinfo.worldSize = 2000.0
	local world = PhysicsFactory:createWorld(cinfo)
	PhysicsSystem:setWorld(world)
	PhysicsSystem:setDebugDrawingEnabled(true)
end

do -- debugCam
	debugCam = GameObjectManager:createGameObject("debugCam")
	debugCam.cc = debugCam:createCameraComponent()
	debugCam.eye = Vec3(100.0, -100.0, 100.0)
	debugCam.aim = Vec3(0.0, 0.0, 75.0)
	debugCam.rotSpeed = 20.0
	debugCam.zoomSpeed = 50.0
	debugCam.maxZoom = 1000.0
	debugCam.minZoom = 10.0
	debugCam.zoom = (debugCam.aim - debugCam.eye):length()
	debugCam.cc:setPosition(debugCam.eye)
	debugCam.cc:lookAt(debugCam.aim)
end

do -- Character
	character = GameObjectManager:createGameObject("character")
	
	character.rc = character:createRenderComponent()
	character.rc:setPath("data/models/barbarian/barbarian.thModel")
	
	character.ac = character:createAnimationComponent()
	character.ac:setSkeletonFile("data/animations/Barbarian/Barbarian.hkt")
	character.ac:setSkinFile("data/animations/Barbarian/Barbarian.hkt")
	
	character.idles = { "Idle", "IdleFidget", "IdleFidget2", "IdleFidget3" }
	character.ac:addAnimationFile(character.idles[1], "data/animations/Barbarian/Barbarian_Idle.hkt")
	character.ac:addAnimationFile(character.idles[2], "data/animations/Barbarian/Barbarian_IdleFidget.hkt")
	character.ac:addAnimationFile(character.idles[3], "data/animations/Barbarian/Barbarian_IdleFidget2.hkt")
	character.ac:addAnimationFile(character.idles[4], "data/animations/Barbarian/Barbarian_IdleFidget3.hkt")
	character.activeIdle = 1
	
	character.ac:addAnimationFile("Walk", "data/animations/Barbarian/Barbarian_Walk.hkt")
	character.ac:addAnimationFile("Run", "data/animations/Barbarian/Barbarian_Run.hkt")
	character.acceleration = 20.0
	character.damping = 10.0
	character.velocity = 0.0
	character.maxVelocity = 10.0
	
	character.playbackSpeed = 1.0

	character.attacks = { "Attack", "Attack2", "Attack3", "Attack4", "SpecialAttack" }
	character.ac:addAnimationFile(character.attacks[1], "data/animations/Barbarian/Barbarian_Attack.hkt")
	character.ac:addAnimationFile(character.attacks[2], "data/animations/Barbarian/Barbarian_Attack2.hkt")
	character.ac:addAnimationFile(character.attacks[3], "data/animations/Barbarian/Barbarian_Attack3.hkt")
	character.ac:addAnimationFile(character.attacks[4], "data/animations/Barbarian/Barbarian_Attack4.hkt")
	character.ac:addAnimationFile(character.attacks[5], "data/animations/Barbarian/Barbarian_SpecialAttack.hkt")
	character.activeAttack = 0 -- no attack
end

do --axe
	axe = GameObjectManager:createGameObject("axe")
	axe.rc = axe:createRenderComponent()
	axe.rc:setPath("data/models/barbarian/barbarianAxe.thModel")
end

function idleEventListener(eventData)
	
	-- select a new idle animation
	character.ac:easeOut(character.idles[character.activeIdle], 1.0)
	local idleDuration = 0
	if (character.activeIdle == 1) then
		-- random idle
		character.activeIdle = math.random(2, #character.idles)
		idleDuration = character.ac:getAnimationDuration(character.idles[character.activeIdle]) - 0.1
	else
		-- default idle
		character.activeIdle = 1
		idleDuration = math.random(5, 10)
	end
	character.ac:setLocalTimeNormalized(character.idles[character.activeIdle], 0.0)
	character.ac:easeIn(character.idles[character.activeIdle], 1.0)
	
	-- re-create the idle timer event
	local idleEvent = Events.create()
	idleEvent:registerListener(idleEventListener)
	idleEvent:delayedTrigger(idleDuration, {})

	return EventResult.Handled
end

--function attackEventListener(eventData)
--
--	return EventResult.Handled
--end

function characterEnter()

	character.ac:setReferencePoseWeightThreshold(0.1)

	-- set idle animation weights
	for i = 1, #character.idles do
		character.ac:setMasterWeight(character.idles[i], 0.1)
		if (i == character.activeIdle) then
			character.ac:easeIn(character.idles[i], 0.0)	
		else
			character.ac:easeOut(character.idles[i], 0.0)	
		end
	end

	-- trigger the first idle animation
	local idleEvent = Events.create()
	idleEvent:registerListener(idleEventListener)
	idleEvent:delayedTrigger(5, {})

	-- set walk and run animation weights
	character.ac:setMasterWeight("Walk", 0.0)
	character.ac:setMasterWeight("Run", 0.0)
	
	-- set attack animation weights
	for i = 1, #character.attacks do
		character.ac:setMasterWeight(character.attacks[i], 1.0)
		character.ac:easeOut(character.attacks[i], 0.0)	
	end
	
	character.ac:setBoneDebugDrawingEnabled(true)

	local bone = character.ac:getBoneByName("right_attachment_jnt")
	local rot = Quaternion(Vec3(1,0,0), -90)
--	local rot = Quaternion(Vec3(1,0,0), -90) * Quaternion(Vec3(0,1,0), 180)
	axe:setParent(bone)
	axe:setRotation(rot)
end

function defaultEnter(enterData)

	characterEnter();

	return EventResult.Handled
end

function cameraUpdate(elapsedTime)

	local mouseDelta = InputHandler:getMouseDelta()
	local rightStick = InputHandler:gamepad(0):rightStick():mulScalar(-10.0)
	
	-- rotation
	local invDir = debugCam.cc:getViewDirection():mulScalar(-1.0)
	local rotQuat = Quaternion(Vec3(0.0, 0.0, 1.0), (mouseDelta.x + rightStick.x) * debugCam.rotSpeed * elapsedTime)
	local rotMat = rotQuat:toMat3()
	local rotInvDir = rotMat:mulVec3(invDir)

	-- zoom
	debugCam.zoom = debugCam.zoom + (mouseDelta.y + rightStick.y) * debugCam.zoomSpeed * elapsedTime
	if (debugCam.zoom > debugCam.maxZoom ) then debugCam.zoom = debugCam.maxZoom end
	if (debugCam.zoom < debugCam.minZoom ) then debugCam.zoom = debugCam.minZoom end
	
	-- set new values
	debugCam.eye = debugCam.aim + rotInvDir:mulScalar(debugCam.zoom)
	debugCam.cc:setPosition(debugCam.eye)
	debugCam.cc:lookAt(debugCam.aim)
end

function characterUpdate(elapsedTime)

	DebugRenderer:printText(Vec2(-0.9, 0.8), "character.activeIdle " .. character.idles[character.activeIdle])

	-- walking forward
	local leftStick = InputHandler:gamepad(0):leftStick():mulScalar(0.75)
	if (InputHandler:isPressed(Key.Up)) then
		character.velocity = character.velocity + character.acceleration * elapsedTime
	end
	character.velocity = character.velocity + character.acceleration * leftStick.y * elapsedTime
	character.velocity = character.velocity - character.damping * elapsedTime
	if (character.velocity > character.maxVelocity) then
		character.velocity = character.maxVelocity
	end
	if (character.velocity < 0.0) then
		character.velocity = 0.0
	end
	DebugRenderer:printText(Vec2(-0.9, 0.70), "character.velocity " .. string.format("%6.3f", character.velocity))
	
	-- walk and run animation weights
	local relativeVelocity = character.velocity / character.maxVelocity
	local maxWeight = 1.0
	local threshold = 0.65
	local walkWeight = 0.0
	local runWeight = 0.0
	if (relativeVelocity <= threshold) then
		walkWeight = maxWeight * (relativeVelocity / threshold)
	else
		walkWeight = maxWeight * (1.0 - ((relativeVelocity - threshold) / (1.0 - threshold)))
		runWeight = maxWeight - walkWeight
	end
	character.ac:setMasterWeight("Walk", walkWeight)
	DebugRenderer:printText(Vec2(-0.9, 0.65), "walkWeight " .. string.format("%6.3f", walkWeight))
	character.ac:setMasterWeight("Run", runWeight)
	DebugRenderer:printText(Vec2(-0.9, 0.60), "runWeight " .. string.format("%6.3f", runWeight))
	
	-- walk and run speed
	local leftTrigger = InputHandler:gamepad(0):leftTrigger()
	local rightTrigger = InputHandler:gamepad(0):rightTrigger()
	if (InputHandler:wasTriggered(Key.Right)) then
		character.playbackSpeed = character.playbackSpeed + 0.1
	elseif (InputHandler:wasTriggered(Key.Left)) then
		character.playbackSpeed = character.playbackSpeed - 0.1
	end
	character.playbackSpeed = character.playbackSpeed - 0.5 * leftTrigger * elapsedTime
	character.playbackSpeed = character.playbackSpeed + 0.5 * rightTrigger * elapsedTime
	character.playbackSpeed = math.clamp(character.playbackSpeed, 0.1, 2.5)
	character.ac:setPlaybackSpeed("Walk", character.playbackSpeed)
	character.ac:setPlaybackSpeed("Run", character.playbackSpeed)
	DebugRenderer:printText(Vec2(-0.9, 0.55), "character.playbackSpeed " .. string.format("%6.3f", character.playbackSpeed))
	
	-- attack
	local buttonsTriggered = InputHandler:gamepad(0):buttonsTriggered()
	if (character.activeAttack == 0) then
		DebugRenderer:printText(Vec2(-0.9, 0.45), "character.activeAttack none")
		if (InputHandler:wasTriggered(Key.Space) or bit32.btest(buttonsTriggered, Button.A)) then
			character.activeAttack = math.random(1, #character.attacks)
			character.ac:setLocalTimeNormalized(character.attacks[character.activeAttack], 0.0)
			character.ac:easeIn(character.attacks[character.activeAttack], 0.25)
			character.ac:easeOut("Walk", 0.25)
			character.ac:easeOut("Run", 0.25)

	-- dummy delayed event
--	local attackEvent = Events.create()
--	attackEvent:registerListener(attackEventListener)
--	attackEvent:delayedTrigger(1.0, {})

		end
	else
		DebugRenderer:printText(Vec2(-0.9, 0.45), "character.activeAttack " .. character.attacks[character.activeAttack])
		local localTimeNormalized = character.ac:getLocalTimeNormalized(character.attacks[character.activeAttack])
		DebugRenderer:printText(Vec2(-0.9, 0.40), "getLocalTimeNormalized " .. string.format("%6.3f", localTimeNormalized))
		if (localTimeNormalized > 0.3 and localTimeNormalized < 0.5) then
			InputHandler:gamepad(0):rumbleLeft(0.75)
			InputHandler:gamepad(0):rumbleRight(0.75)
		else
			InputHandler:gamepad(0):rumbleLeft(0.0)
			InputHandler:gamepad(0):rumbleRight(0.0)
		end
		if (localTimeNormalized > 0.75) then
			character.ac:easeOut(character.attacks[character.activeAttack], 0.25)
			character.ac:easeIn("Walk", 0.25)
			character.ac:easeIn("Run", 0.25)
			character.activeAttack = 0
		end
	end
end

function defaultUpdate(updateData)

	local elapsedTime = updateData:getElapsedTime()
	
	cameraUpdate(elapsedTime)
	characterUpdate(elapsedTime)

	return EventResult.Handled
end

State{
	name = "default",
	parent = "/game",
	eventListeners = {
		enter = { defaultEnter },
		update = { defaultUpdate }
	}
}

StateTransitions{
	parent = "/game",
	{ from = "__enter", to = "default" }
}

StateTransitions{
	parent = "/game",
	{ from = "default", to = "__leave", condition = function()
		return
			InputHandler:isPressed(Key.Escape) or
			bit32.btest(InputHandler:gamepad(0):buttonsPressed(), bit32.bor(Button.Start, Button.Back))
	end }
}
