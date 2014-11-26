logMessage("using defaults/freeCamera.lua")

do -- freeCam
	freeCam = GameObjectManager:createGameObject("freeCam")
	freeCam.cc = freeCam:createCameraComponent()
	freeCam.cc:setPosition(Vec3(0.0, 0.0, 0.0))
	freeCam.cc:setViewDirection(Vec3(1.0, 0.0, 0.0))
	freeCam.baseViewDir = Vec3(1.0, 0.0, 0.0)
	freeCam.cc:setBaseViewDirection(freeCam.baseViewDir)
end

function freeCamEnter(enterData)
	freeCam:setComponentStates(ComponentState.Active)
	return EventResult.Handled
end

function freeCamUpdate(updateData)
	local deltaSeconds = updateData:getElapsedTime()
	local mouseDelta = InputHandler:getMouseDelta()
	local rotationSpeed = 200 * deltaSeconds
	local lookVec = mouseDelta:mulScalar(rotationSpeed)
	freeCam.cc:look(lookVec)

	local moveVec = Vec3(0.0, 0.0, 0.0)
	local moveSpeed = 500 * deltaSeconds
	if (InputHandler:isPressed(Key.Shift)) then
		moveSpeed = moveSpeed * 5
	end
	if (InputHandler:isPressed(Key.W)) then
		moveVec.y = moveSpeed
	elseif (InputHandler:isPressed(Key.S)) then
		moveVec.y = -moveSpeed
	end
	if (InputHandler:isPressed(Key.A)) then
		moveVec.x = -moveSpeed
	elseif (InputHandler:isPressed(Key.D)) then
		moveVec.x = moveSpeed
	end
	freeCam.cc:move(moveVec)

	return EventResult.Handled
end

Events.Update:registerListener(function(dt)
	return freeCamUpdate{ getElapsedTime = function() return dt end }
end)
