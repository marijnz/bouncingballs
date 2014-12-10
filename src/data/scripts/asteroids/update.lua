
score = 0
gameOver = false
function update(elapsedTime)
	DebugRenderer:printText(Vec2(-0.9, 0.75), "score: " .. score)

	-- game over state
	if (gameOver) then
		DebugRenderer:printText(Vec2(-0.1, 0.5), "GAME OVER")
		DebugRenderer:printText(Vec2(-0.1, 0.45), "press return to restart")
		if (InputHandler:wasTriggered(Key.Return)) then
			ship.go:setPosition(Vec3(0, 0, 0))
			ship.go:setRotation(Quaternion())
			ship.rb:setLinearVelocity(Vec3(0, 0, 0))
			ship.rb:setAngularVelocity(Vec3(0, 0, 0))
			ship.go:setComponentStates(ComponentState.Active)
			score = 0
			gameOver = false
		end
		return EventResult.Handled
	end

	-- camera selection
	if (InputHandler:wasTriggered(Key.C)) then
		cam.orthographic = not cam.orthographic
		cam.cc:setOrthographic(cam.orthographic)
	end
	DebugRenderer:printText(Vec2(-0.9, 0.70), "orthographic cam: " .. tostring(cam.orthographic))
	
	-- respawn asteroid
	if (asteroid.destroyed) then
		local shipPosition = ship.go:getPosition()
		local offsetY = 150
		if (math.random() > 0.5) then offsetY = -offsetY end
		local offsetZ = 150
		if (math.random() > 0.5) then offsetZ = -offsetZ end
		local asteroidPosition = Vec3(0, shipPosition.y + offsetY + ((math.random() - 0.5) * 75), shipPosition.z + offsetZ + ((math.random() - 0.5) * 75))
		local asteroidVelocity = Vec3(0, math.random(-75, 75), math.random(-75, 75))
		local asteroidRotation = Vec3(math.random(-2, 2), 0, 0)
		asteroid.go:setPosition(asteroidPosition)
		asteroid.rb:setLinearVelocity(asteroidVelocity)
		asteroid.go:setRotation(Quaternion())
		asteroid.rb:setAngularVelocity(asteroidRotation)
		asteroid.destroyed = false
		asteroid.go:setComponentStates(ComponentState.Active)
	end

	return EventResult.Handled
end

border = { y=330, z=190, tolerance=5 }
function handleScreenBorder(go)
	local position = go:getPosition()
	if (position.y < -(border.y + border.tolerance)) then
		position.y = border.y
	end
	if (position.y > (border.y + border.tolerance)) then
		position.y = -border.y
	end
	if (position.z < -(border.z + border.tolerance)) then
		position.z = border.z
	end
	if (position.z > (border.z + border.tolerance)) then
		position.z = -border.z
	end
	go:setPosition(position)
end

angularVelocitySwapped = false
function updateShip(guid, elapsedTime)
	
	-- ship rotation
	local angularVelocity = ship.rb:getAngularVelocity()
	-- angularVelocity.x = 0
	local rotationSpeed = 5
	if (InputHandler:isPressed(Key.Left) and InputHandler:isPressed(Key.Right)) then
		if (not angularVelocitySwapped) then
			-- swap current angular velocity
			angularVelocity.x = -angularVelocity.x
			angularVelocitySwapped = true
		end
	elseif (InputHandler:isPressed(Key.Left)) then
		angularVelocity.x = -rotationSpeed
		angularVelocitySwapped = false
	elseif (InputHandler:isPressed(Key.Right)) then
		angularVelocity.x = rotationSpeed
		angularVelocitySwapped = false
	else -- neither left nor right is pressed
		angularVelocity.x = 0
		angularVelocitySwapped = false
	end
	ship.rb:setAngularVelocity(angularVelocity)

	local shipPosition = ship.go:getPosition()
	local shipRotation = ship.go:getRotation()
	local shipViewDirection = ship.go:getViewDirection()

	-- ship acceleration
	local acceleration = 60
	if (InputHandler:isPressed(Key.Up)) then
		local linearImpulse = shipViewDirection:mulScalar(acceleration)
		ship.rb:applyLinearImpulse(linearImpulse)
	end

	-- shooting laser
	local laserOffset = 15
	local laserSpeed = 250
	if (InputHandler:wasTriggered(Key.Space) and laser.lifetime <= 0) then
		local laserPosition = shipPosition + shipViewDirection:mulScalar(laserOffset)
		laserPosition.x = 0
		local laserVelocity = shipViewDirection:mulScalar(laserSpeed)
		laserVelocity.x = 0
		laser.go:setPosition(laserPosition)
		laser.go:setRotation(shipRotation)
		laser.rb:setLinearVelocity(laserVelocity)
		laser.rb:setAngularVelocity(Vec3(0, 0, 0))
		laser.lifetime = 1.5
		laser.go:setComponentStates(ComponentState.Active)
	end
	handleScreenBorder(ship.go)
	DebugRenderer:printText(Vec2(-0.9, 0.65), "ship: x " .. string.format("%.2f", shipPosition.x) .. ", y " .. string.format("%.2f", shipPosition.y) .. ", z " .. string.format("%.2f", shipPosition.z))

	-- draw direction arrow
	local arrowLength = 15
	local arrowStart = shipPosition
	local arrowEnd = shipPosition + shipViewDirection:mulScalar(arrowLength)
	DebugRenderer:drawArrow(arrowStart, arrowEnd, Color(1, 0, 0, 1))
end

function shipCollision(event)
	local rigidBody = event:getBody(CollisionArgsCallbackSource.A)
	if (rigidBody:equals(asteroid.rb)) then
		gameOver = true
		ship.go:setComponentStates(ComponentState.Inactive)
		laser.go:setComponentStates(ComponentState.Inactive)
		laser.lifetime = 0
		asteroid.go:setComponentStates(ComponentState.Inactive)
		asteroid.destroyed = true
	end

	return EventResult.Handled
end

function updateLaser(guid, elapsedTime)
	laser.lifetime = laser.lifetime - (elapsedTime)
	if (laser.lifetime <= 0) then
		laser.go:setComponentStates(ComponentState.Inactive)
	else
		handleScreenBorder(laser.go)
		local laserPosition = laser.go:getPosition()
		DebugRenderer:printText(Vec2(-0.9, 0.60), "laser: x " .. string.format("%.2f", laserPosition.x) .. ", y " .. string.format("%.2f", laserPosition.y) .. ", z " .. string.format("%.2f", laserPosition.z))
	end
end

function laserCollision(event)
	local rigidBody = event:getBody(CollisionArgsCallbackSource.A)
	if (rigidBody:equals(asteroid.rb)) then
		score = score + asteroid.rb:getUserData().score
		laser.go:setComponentStates(ComponentState.Inactive)
		laser.lifetime = 0
		if (asteroid.destructible) then
			asteroid.go:setComponentStates(ComponentState.Inactive)
			asteroid.destroyed = true
		else
			local colDir = (asteroid.go:getPosition() - laser.go:getPosition()):normalized()
			event:accessVelocities(CollisionArgsCallbackSource.A)
			asteroid.rb:setLinearVelocity(asteroid.rb:getLinearVelocity() + colDir:mulScalar(25))
			event:updateVelocities(CollisionArgsCallbackSource.A)
		end
	end

	return EventResult.Handled
end

function updateAsteroid(guid, elapsedTime)
	handleScreenBorder(asteroid.go)
	local asteroidPosition = asteroid.go:getPosition()
	DebugRenderer:printText(Vec2(-0.9, 0.55), "asteroidPosition: x " .. string.format("%.2f", asteroidPosition.x) .. ", y " .. string.format("%.2f", asteroidPosition.y) .. ", z " .. string.format("%.2f", asteroidPosition.z))
	if (InputHandler:wasTriggered(Key.D)) then
		asteroid.destructible = not asteroid.destructible
	end
	DebugRenderer:printText(Vec2(-0.9, 0.50), "asteroidDestructible: " .. tostring(asteroid.destructible))
end
