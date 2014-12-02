logMessage("using freecamera.lua")

do -- cameraera
	camera = GameObjectManager:createGameObject("camera")
	camera.cc = camera:createCameraComponent()
	camera.cc:setPosition(Vec3(0.0, 0.0, 0.0))
	camera.cc:setViewDirection(Vec3(1.0, 0.0, 0.0))
	camera.baseViewDir = Vec3(1.0, 0.0, 0.0)
	camera.cc:setBaseViewDirection(camera.baseViewDir)
end

function cameraEnter(enterData)
	camera:setComponentStates(ComponentState.Active)
	return EventResult.Handled
end

function cameraUpdate(updateData)
	local deltaSeconds = updateData:getElapsedTime()
	local mouseDelta = InputHandler:getMouseDelta()
	local rotationSpeed = 20 * deltaSeconds
	local lookVec = mouseDelta:mulScalar(rotationSpeed)
	camera.cc:look(lookVec)

	local moveVec = Vec3(0.0, 0.0, 0.0)
	local moveSpeed = 50 * deltaSeconds
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
	camera.cc:move(moveVec)

	return EventResult.Handled
end

Events.Update:registerListener(function(dt)
	return cameraUpdate{ getElapsedTime = function() return dt end }
end)
