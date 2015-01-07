logMessage([[Default free camera usage instructions:
  - Use WASD to move the camera, hold down Shift to move faster or Ctrl to move slower
  - Use the mouse to rotate the camera
  - Use the mouse wheel to increase or decrease movement speed
  - Hold Shift and use the mouse wheel to increase or decrease rotation speed
]])

-- This factor is multiplied to the relative mouse movement of the user which is then used to rotate the camera
local rotationFactor = 200

-- The amount by which the rotationFactor is increased or decreased according to user input (mouse wheel)
local rotationFactorStepping = 10

-- This is the determining factor to move (translate) the camera
local movementFactor = 100

-- The amount by which the movementFactor is increased or decreased according to user input
local movementFactorStepping = 10

-- This is multiplied to the movement speed when Shift is pressed, and divided when Ctrl is pressed
local movementSpeedModifier = 5

do -- freeCam
	-- Note: This variable is global so it can be manipulated from somewhere else
	freeCam = GameObjectManager:createGameObject("freeCam")
	freeCam.cc = freeCam:createCameraComponent()
	freeCam.cc:setUpDirection(Vec3(0, 0, 1))
end

local function update(dt) -- dt => delta time in seconds
	local mouseDelta = InputHandler:getMouseDelta()
	local shift      = InputHandler:isPressed(Key.Shift)
	local ctrl       = InputHandler:isPressed(Key.Control)

	-- Tweak the factors using the mouse wheel
	---------------------------------------------
	if mouseDelta.z ~= 0 then
		if shift
		then rotationFactor = rotationFactor + mouseDelta.z * rotationFactorStepping
		else movementFactor = movementFactor + mouseDelta.z * movementFactorStepping
		end

		-- Make sure the factors are still in range
		if rotationFactor < rotationFactorStepping then rotationFactor = rotationFactorStepping end
		if movementFactor < movementFactorStepping then movementFactor = movementFactorStepping end
	end

	-- Rotate the camera according to user input
	-----------------------------------------------
	local lookVec = mouseDelta:mulScalar(rotationFactor * dt)
	freeCam.cc:look(lookVec)

	-- Move the camera according to user input
	---------------------------------------------
	local moveVec = Vec3(0.0, 0.0, 0.0)

	-- Forwards and backwards
	if     InputHandler:isPressed(Key.W) then moveVec.y = 1
	elseif InputHandler:isPressed(Key.S) then moveVec.y = -1 end

	-- Left and right
	if     InputHandler:isPressed(Key.D) then moveVec.x = 1
	elseif InputHandler:isPressed(Key.A) then moveVec.x = -1 end

	--
	if moveVec:squaredLength() == 0 then return end

	-- Shift and control modifiers
	local moveSpeed = movementFactor * dt
	if    shift then moveSpeed = moveSpeed * movementSpeedModifier
	elseif ctrl then moveSpeed = moveSpeed / movementSpeedModifier end
	moveVec = moveVec:normalized():mulScalar(moveSpeed)
	freeCam.cc:move(moveVec)
end

Events.Update:registerListener(update)
