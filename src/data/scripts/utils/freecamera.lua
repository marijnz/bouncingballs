logMessage([[Default free camera usage instructions:
  - Use WASD to move the camera, hold down Shift to move faster or Ctrl to move slower
  - Use the mouse to rotate the camera
  - Use the mouse wheel to increase or decrease movement speed
  - Hold Shift and use the mouse wheel to increase or decrease rotation speed
]])

do -- freeCam
	-- Note: This variable is global so it can be manipulated from somewhere else
	freeCam = GameObjectManager:createGameObject("freeCam")
	freeCam.cc = freeCam:createCameraComponent()
	freeCam.cc:setPosition(Vec3(0, 0, 0))
	freeCam.cc:setViewDirection(Vec3(0, 1, 0))
	freeCam.cc:setUpDirection(Vec3(0, 0, 1))
end

freeCam.drawPosition = true

-- This factor is multiplied to the relative mouse movement of the user which is then used to rotate the camera
freeCam.angularSpeed = 80
freeCam.minAngularSpeed = 1

-- The amount by which the freeCam.angularSpeed is increased or decreased according to (mouse wheel) user input (per second)
freeCam.angularSpeedStepping = 3

-- This is the determining factor to move (translate) the camera
freeCam.linearSpeed = 30
freeCam.minLinearSpeed = 1

-- The amount by which the freeCam.linearSpeed is increased or decreased according to user input (per second)
freeCam.linearSpeedStepping = 5

-- This is multiplied to the movement speed when Shift is pressed, and divided when Ctrl is pressed
freeCam.linearSpeedShiftModifier = 10

local function update(dt) -- dt => delta time in seconds
	if freeCam.drawPosition then
		DebugRenderer:printText(Vec2(-0.98, -0.95), "freeCam@" .. tostring(freeCam:getPosition(), "vec3"))
	end

	local mouseDelta = InputHandler:getMouseDelta()
	local shift      = InputHandler:isPressed(Key.Shift)
	local ctrl       = InputHandler:isPressed(Key.Control)

	-- Tweak the factors using the mouse wheel
	---------------------------------------------
	if mouseDelta.z ~= 0 then
		if shift
		then freeCam.angularSpeed = freeCam.angularSpeed + mouseDelta.z * freeCam.angularSpeedStepping
		else freeCam.linearSpeed  = freeCam.linearSpeed  + mouseDelta.z * freeCam.linearSpeedStepping
		end

		-- Make sure the factors are still in range
		if freeCam.angularSpeed < freeCam.minAngularSpeed then freeCam.angularSpeed = freeCam.minAngularSpeed end
		if freeCam.linearSpeed  < freeCam.minLinearSpeed  then freeCam.linearSpeed  = freeCam.minLinearSpeed  end
	end

	-- Rotate the camera according to user input
	-----------------------------------------------
	local lookVec = mouseDelta:mulScalar(freeCam.angularSpeed * dt)
	freeCam.cc:look(lookVec)

	-- Move the camera according to user input
	---------------------------------------------
	local moveVec = Vec3(0.0, 0.0, 0.0)

	-- Forwards and backwards
	if     InputHandler:isPressed(Key.W) then moveVec.y =  1
	elseif InputHandler:isPressed(Key.S) then moveVec.y = -1 end

	-- Left and right
	if     InputHandler:isPressed(Key.D) then moveVec.x =  1
	elseif InputHandler:isPressed(Key.A) then moveVec.x = -1 end

	--
	if moveVec:squaredLength() == 0 then return end

	-- Shift and control modifiers
	local moveSpeed = freeCam.linearSpeed
	if    shift then moveSpeed = moveSpeed * freeCam.linearSpeedShiftModifier
	elseif ctrl then moveSpeed = moveSpeed / freeCam.linearSpeedShiftModifier end
	moveVec = moveVec:normalized():mulScalar(moveSpeed * dt)
	freeCam.cc:move(moveVec)
end

Events.Update:registerListener(update)
