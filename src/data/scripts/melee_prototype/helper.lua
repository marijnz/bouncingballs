
function createCollisionBox(guid, halfExtends, position)
	local box = GameObjectManager:createGameObject(guid)
	box.pc = box:createPhysicsComponent()
	local cinfo = RigidBodyCInfo()
	cinfo.collisionFilterInfo = 0xFFFF
	cinfo.shape = PhysicsFactory:createBox(halfExtends)
	cinfo.motionType = MotionType.Fixed
	cinfo.position = position
	box.pc.rb = box.pc:createRigidBody(cinfo)
	box.pc.rb:setUserData(box)
	return box
end

function createCollisionSphere(guid, radius, position)
	local sphere = GameObjectManager:createGameObject(guid)
	sphere.pc = sphere:createPhysicsComponent()
	local cinfo = RigidBodyCInfo()
	cinfo.collisionFilterInfo = 0x2
	cinfo.shape = PhysicsFactory:createSphere(radius)
	cinfo.motionType = MotionType.Dynamic
	cinfo.position = position
	cinfo.mass = 50
	cinfo.linearDamping = 0
	cinfo.angularDamping = 0.5
	cinfo.restitution = 1.0
	cinfo.friction = 0.75
	cinfo.gravityFactor = 25
	cinfo.maxLinearVelocity = 10000
	cinfo.angularVelocity = Vec3(math.random(-10, 10), math.random(-10, 10), math.random(-10, 10))
	sphere.pc.rb = sphere.pc:createRigidBody(cinfo)
	sphere.pc.rb:setUserData(sphere)
	return sphere
end

function createDebugCam(guid)
	local cam = GameObjectManager:createGameObject(guid)
	cam.cc = cam:createCameraComponent()
	cam:setPosition(Vec3(0, 0, 0))
	cam.cc:setViewDirection(Vec3(0, 1, 0))
	cam.cc:move(Vec3(10, -75, 50))
	cam.cc:lookAt(Vec3(10, 0, 25))
	cam.update = function(self, elapsedTime)
		
		local mouseDelta = InputHandler:getMouseDelta()
		
		local gamepad = InputHandler:gamepad(0)
		local rightStick = gamepad:rightStick()
		rightStick.y = -rightStick.y
--		local leftStick = gamepad:leftStick()
		local leftTrigger = gamepad:leftTrigger()
		local rightTrigger = gamepad:rightTrigger()
		
		local lookSpeedMouse = 35 * elapsedTime
		local lookSpeedStick = 100 * elapsedTime
		local lookVec = rightStick:mulScalar(lookSpeedStick) + mouseDelta:mulScalar(lookSpeedMouse) 
		self.cc:look(lookVec)
		
		local moveSpeed = 300 * elapsedTime
		local controlPressed = 0
		if (InputHandler:isPressed(Key.Control)) then
			controlPressed = 1
		end
		local shiftPressed = 0
		if (InputHandler:isPressed(Key.Shift)) then
			shiftPressed = 1
		end
		moveSpeed = moveSpeed * (1 - leftTrigger*0.9) * (1 - controlPressed*0.9)
		moveSpeed = moveSpeed + moveSpeed * (4 * shiftPressed + 4 * rightTrigger)
		local moveVec = Vec2(0, 0)
		if (InputHandler:isPressed(Key.Up) or bit32.btest(InputHandler:gamepad(0):buttonsPressed(), Button.Up)) then
			moveVec.y = moveSpeed
		elseif (InputHandler:isPressed(Key.Down) or bit32.btest(InputHandler:gamepad(0):buttonsPressed(), Button.Down)) then
			moveVec.y = -moveSpeed
		end
		if (InputHandler:isPressed(Key.Left) or bit32.btest(InputHandler:gamepad(0):buttonsPressed(), Button.Left)) then
			moveVec.x = -moveSpeed
		elseif (InputHandler:isPressed(Key.Right) or bit32.btest(InputHandler:gamepad(0):buttonsPressed(), Button.Right)) then
			moveVec.x = moveSpeed
		end
--		moveVec = moveVec + leftStick:mulScalar(moveSpeed)
		self.cc:move(Vec3(moveVec.x, moveVec.y, 0))
		
		DebugRenderer:printText(Vec2(-0.9, 0.75), "updateDebugCam \"" .. self:getGuid() .. "\"")
		local pos = self.cc:getWorldPosition()
		DebugRenderer:printText(Vec2(-0.9, 0.70), "  pos: " .. string.format("%5.2f", pos.x) .. ", " .. string.format("%5.2f", pos.y) .. ", " .. string.format("%5.2f", pos.z))
		local dir = self.cc:getViewDirection()
		DebugRenderer:printText(Vec2(-0.9, 0.65), "  dir: " .. string.format("%5.2f", dir.x) .. ", " .. string.format("%5.2f", dir.y) .. ", " .. string.format("%5.2f", dir.z))
	end
	return cam
end

function calcAngleBetween(vector1, vector2)
	local angleRad = math.atan2(vector2.y, vector2.x) - math.atan2(vector1.x, vector1.y)
	local angleDeg = (angleRad / math.pi) * 180
	if (angleDeg > 180) then
		angleDeg = angleDeg - 360
	end
	if (angleDeg < -180) then
		angleDeg = angleDeg + 360
	end
	return angleDeg
end

function rotateVector(vector, axis, angle)
	local rotQuat = Quaternion(axis, angle)
	local rotMat = rotQuat:toMat3()
	local rotVector = rotMat:mulVec3(vector)
	return rotVector
end
