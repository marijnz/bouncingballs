
function buttonOrKeyPressed(button, key)
	return
		bit32.btest(InputHandler:gamepad(0):buttonsTriggered(), button)
		or InputHandler:wasTriggered(key)
end

function drawLine(dir, pos, length, color)
	local from = dir:mulScalar(-0.5*length) + pos
	local to = dir:mulScalar(0.5*length) + pos
	DebugRenderer:drawLine2D(from, to, color)
end

function drawCross(pos, size, color)
	local downRight = Vec2(1, -1)
	drawLine(downRight, pos, size, color)
	local upRight = Vec2(1, 1)
	drawLine(upRight, pos, size, color)
end

function createCollisionBox(guid, halfExtends, position)
	local box = GameObjectManager:createGameObject(guid)
	box.pc = box:createPhysicsComponent()
	local cinfo = RigidBodyCInfo()
	cinfo.motionType = MotionType.Fixed
	cinfo.collisionFilterInfo = 0x00FF
	cinfo.shape = PhysicsFactory:createBox(halfExtends)
	cinfo.position = position
	box.pc.rb = box.pc:createRigidBody(cinfo)
	box.pc.rb:setUserData(box)
	return box
end

function createCollisionCapsule(guid, startPos, endPos, radius)
	local capsule = GameObjectManager:createGameObject(guid)
	capsule.pc = capsule:createPhysicsComponent()
	local cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createCapsule(startPos, endPos, radius)
	cinfo.motionType = MotionType.Fixed
	cinfo.collisionFilterInfo = 0xFFFF
	cinfo.isTriggerVolume = true
	capsule.pc.rb = capsule.pc:createRigidBody(cinfo)
	capsule.pc.rb:setUserData(capsule.pc)
	return capsule
end
