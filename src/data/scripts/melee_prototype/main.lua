-- Dependencies
include("utils/timedStatusDisplay.lua")
include("melee_prototype/helper.lua")
include("melee_prototype/character.lua")

-- physics world
local cinfo = WorldCInfo()
cinfo.gravity = Vec3(0, 0, -9.81)
cinfo.worldSize = 2000
local world = PhysicsFactory:createWorld(cinfo)
world:setCollisionFilter(PhysicsFactory:createCollisionFilter_Simple())
PhysicsSystem:setWorld(world)
PhysicsSystem:setDebugDrawingEnabled(true)

-- random seed
math.randomseed(os.time())

-- ground
ground = createCollisionBox("ground", Vec3(1000, 1000, 20), Vec3(0, 0, -20))
-- walls
walls = {}
walls[1] = createCollisionBox("wall1", Vec3(20, 1000, 200), Vec3(-980, 0, 200))
walls[2] = createCollisionBox("wall2", Vec3(20, 1000, 200), Vec3(980, 0, 200))
walls[3] = createCollisionBox("wall3", Vec3(960, 20, 200), Vec3(0, -980, 200))
walls[4] = createCollisionBox("wall4", Vec3(960, 20, 200), Vec3(0, 980, 200))
-- spheres
spheres = {}
spheres[1] = createCollisionSphere("sphere1", 100, Vec3(300, 300, 200))
spheres[2] = createCollisionSphere("sphere2", 100, Vec3(-300, 300, 200))
spheres[3] = createCollisionSphere("sphere3", 100, Vec3(300, -300, 200))
spheres[4] = createCollisionSphere("sphere4", 100, Vec3(-300, -300, 200))
-- debug cam
debugCam = createDebugCam("debugCam")
-- 3rd person cam
cam = GameObjectManager:createGameObject("cam")
cam.cc = cam:createCameraComponent()
cam.cc:setBaseViewDirection(Vec3(0, -1, 0))
cam.rotation = 0
-- character
character = createCharacter("character", Vec3(0, 0, 5))
-- axe
axe = GameObjectManager:createGameObject("axe")
axe.rc = axe:createRenderComponent()
axe.rc:setPath("data/models/barbarian/barbarianAxe.thModel")
axe.pc = axe:createPhysicsComponent()
do
	local cinfo = RigidBodyCInfo()
	local box = PhysicsFactory:createBox(Vec3(5, 30, 50))
	cinfo.collisionFilterInfo = 0x0 --0x2
	cinfo.shape = PhysicsFactory:createConvexTranslateShape(box, Vec3(0, -50, -10))
	cinfo.motionType = MotionType.Keyframed
	cinfo.isTriggerVolume = true
	axe.pc.rb = axe.pc:createRigidBody(cinfo)
	axe.pc.rb:setUserData(axe)
	axe.pc.rb:getTriggerEvent():registerListener(function(args)
		local go = args:getRigidBody():getUserData()
		if args:getEventType() == TriggerEventType.Entered then
			timedStatusDisplay("Hit " .. go:getGuid())
--			local hitDir = axe:getViewDirection()
			local hitDir = axe:getUpDirection()
			go.pc.rb:applyLinearImpulse(hitDir:mulScalar(-50000.0))
		elseif args:getEventType() == TriggerEventType.Left then
			timedStatusDisplay("Not hitting " .. go:getGuid() .. " anymore.")
		end
		return EventResult.Handled
	end)
end
-- NPC
npc = createCharacter("npc", Vec3(500, -500, 5))

-- global state flags
debugCamEnabled = false
cameraRelativeControls = true

function defaultEnter(enterData)

--	character.rc:setState(ComponentState.Inactive)
--	character.rc:setScale(Vec3(0.5, 0.5, 0.5))
	character.ac:setBoneDebugDrawingEnabled(true)
	character:initialize()

--	npc.rc:setState(ComponentState.Inactive)
	npc.rc:setScale(Vec3(1.25, 1.25, 1.25))
	npc.ac:setBoneDebugDrawingEnabled(false)
	npc:initialize()

	-- parent the axe to the character's hand
	local bone = character.ac:getBoneByName("right_attachment_jnt")
	local rot = Quaternion(Vec3(1, 0, 0), -90)
	axe:setParent(bone)
	axe:setRotation(rot)

	return EventResult.Handled
end

function defaultUpdate(updateData)

	local elapsedTime = updateData:getElapsedTime()

	-- switch between normal and debug camera
	if (InputHandler:wasTriggered(Key.C) or bit32.btest(InputHandler:gamepad(0):buttonsTriggered(), Button.Start)) then
		debugCamEnabled = not debugCamEnabled
	end
	if (debugCamEnabled) then
		debugCam:setComponentStates(ComponentState.Active)
		debugCam:update(elapsedTime)
	else
		cam:setComponentStates(ComponentState.Active)
	end

	-- switch between camera-relative and absolute controls
	if (InputHandler:wasTriggered(Key.Y) or bit32.btest(InputHandler:gamepad(0):buttonsTriggered(), Button.Y)) then
		cameraRelativeControls = not cameraRelativeControls
	end

	-- virtual analog stick (WASD)
	local virtualStick = Vec2(0, 0)
	if (InputHandler:isPressed(Key.A)) then virtualStick.x = virtualStick.x - 1 end
	if (InputHandler:isPressed(Key.D)) then virtualStick.x = virtualStick.x + 1 end
	if (InputHandler:isPressed(Key.W)) then virtualStick.y = virtualStick.y + 1 end
	if (InputHandler:isPressed(Key.S)) then virtualStick.y = virtualStick.y - 1 end
	virtualStick = virtualStick:normalized()

	-- gamepad input
	local gamepad = InputHandler:gamepad(0)
	local leftStick = gamepad:leftStick()
	local rightStick = gamepad:rightStick()

	-- mose input
	local mouseDelta = InputHandler:getMouseDelta()

	-- combined move vector
	local moveVector = virtualStick + leftStick

	-- move vector relative to the camera
	local charViewDir = character:getViewDirection()
	local camViewDir
	if (debugCamEnabled) then
		camViewDir = debugCam.cc:getViewDirection()
	else
		camViewDir = cam.cc:getViewDirection()
	end
	camViewDir.z = 0
	local angleDeg = 0
	if (cameraRelativeControls) then
		angleDeg = calcAngleBetween(Vec2(1, 0), camViewDir)
	else
		angleDeg = calcAngleBetween(Vec2(1, 0), charViewDir)
	end

	local moveVector3 = Vec3(moveVector.x, moveVector.y, 0)
	DebugRenderer:drawArrow(Vec3(0.1, 0.1, 0.1), moveVector3:mulScalar(250), Color(1, 0, 0, 1))
	local moveVector3Rot = rotateVector(moveVector3, Vec3(0.0, 0.0, 1.0), angleDeg)
	if (moveVector3Rot:length() > 0) then -- FIXME Prevents crash when rendering the arrow
		DebugRenderer:drawArrow(character:getPosition(), character:getPosition() + moveVector3Rot:mulScalar(250), Color(0, 1, 1, 1))
	end

	-- update the character
	local walkSpeed = 0
	if (cameraRelativeControls) then
		walkSpeed = character.maxWalkSpeed * moveVector:length()	-- direction depends on the camera
	else
		walkSpeed = character.maxWalkSpeed * moveVector.y			-- up means forward
	end
	local rotationSpeed = 0
	if (cameraRelativeControls) then
		local steer = character:calcSteering(moveVector3Rot)
		rotationSpeed = character.maxRotationSpeed * -steer * elapsedTime			-- rotation relative to the camera
	else
		rotationSpeed = character.maxRotationSpeed * -moveVector.x * elapsedTime	-- left/right means rotate
	end
	local attack = (InputHandler:wasTriggered(Key.Space) or bit32.btest(InputHandler:gamepad(0):buttonsTriggered(), Button.X))
	character:update(walkSpeed, rotationSpeed, attack)

	-- third person camera
	cam.rotation = cam.rotation + (mouseDelta.x * 50 * elapsedTime) + (rightStick.x * -200 * elapsedTime)
	if (cam.rotation > 180) then
		cam.rotation = cam.rotation - 360
	end
	if (cam.rotation < -180) then
		cam.rotation = cam.rotation + 360
	end
	cam.rotation = cam.rotation * (1 - math.clamp(1.5 * character.relativeSpeed * elapsedTime, 0, 0.05))
	local invDir = character:getViewDirection():mulScalar(-350)
	local rotInvDir = rotateVector(invDir, Vec3(0.0, 0.0, 1.0), cam.rotation)
	local camPos = character:getPosition() + rotInvDir
	camPos.z = 250
	local camAim = character:getPosition()
	camAim.z = 50
	local realCamPos = cam.cc:getPosition()
	local newCamPos = realCamPos + (camPos - realCamPos):mulScalar(5 * elapsedTime)
	cam.cc:setPosition(newCamPos)
	cam.cc:lookAt(camAim)

	-- debug texts
	DebugRenderer:printText(Vec2(-0.9, 0.55), "cameraRelativeControls " .. tostring(cameraRelativeControls))
	DebugRenderer:printText(Vec2(-0.9, 0.50), "character.walkSpeed " .. string.format("%5.2f", character.walkSpeed))
	DebugRenderer:printText(Vec2(-0.9, 0.45), "character.rotationSpeed " .. string.format("%5.2f", character.rotationSpeed))
	DebugRenderer:printText(Vec2(-0.9, 0.40), "character.walkAnimWeight " .. string.format("%6.3f", character.walkAnimWeight))
	DebugRenderer:printText(Vec2(-0.9, 0.35), "character.runAnimWeight " .. string.format("%6.3f", character.runAnimWeight))
	DebugRenderer:printText(Vec2(-0.9, 0.30), "cam.rotation " .. string.format("%5.2f", cam.rotation))

	-- update the NPC
	local spherePos = spheres[3]:getPosition()
	DebugRenderer:drawArrow(spherePos, spherePos + Vec3(0, 0, 150), Color(1, 1, 0, 1))
	local npcPos = npc:getPosition()
	local npcMoveVec = spherePos - npcPos
	local npcMoveDist = npcMoveVec:length()
	DebugRenderer:drawArrow(npcPos, npcPos + npcMoveVec, Color(1, 0, 1, 1))
	local npcWalkSpeed = 0
	if (npcMoveDist > 350) then
		npcWalkSpeed = npc.walkSpeed + 5000 * elapsedTime
	else
		npcWalkSpeed = npc.walkSpeed - 7500 * elapsedTime
	end
	npcWalkSpeed = math.clamp(npcWalkSpeed, 0, npc.maxWalkSpeed)
	local npcRotationSpeed = 0
	if (npcMoveDist > 150) then
		local npcSteer = npc:calcSteering(npcMoveVec:normalized())
		npcRotationSpeed = npc.maxRotationSpeed * -npcSteer * elapsedTime
	end
	local npcAttack = false
	npc:update(npcWalkSpeed, npcRotationSpeed, npcAttack)

	return EventResult.Handled
end

-- global state machine
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
	{ from = "default", to = "__leave", condition = function() return InputHandler:wasTriggered(Key.Escape) or bit32.btest(InputHandler:gamepad(0):buttonsTriggered(), Button.Back) end }
}

timedStatusDisplayStart()
