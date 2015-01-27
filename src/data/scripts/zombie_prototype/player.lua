PLAYER_HEIGHT				= 120
ASPECT_RATIO				= 16 / 9
CURSOR_SIZE					= 0.1
CURSOR_COLOR_NORMAL			= Color(0, 1, 0, 1)
CURSOR_COLOR_BLOCKED		= Color(1, 0, 0, 1)
MOUSE_SENSITIVITY			= Vec2(0.50, 0.50 * ASPECT_RATIO)
STICK_SENSITIVITY			= Vec2(1.50, 1.50 * ASPECT_RATIO)
CAMERA_BORDER				= Vec2(0.15, 0.15 * ASPECT_RATIO)
CAMERA_WINDOW_MAX			= Vec2( 1,  1) - CAMERA_BORDER
CAMERA_WINDOW_MIN			= Vec2(-1, -1) + CAMERA_BORDER
CAMERA_SENSITIVITY			= Vec2(200, 100)

function __player__update(guid, elapsedTime)
	local self = player
	
	-- cursor
	local mouseDelta = InputHandler:getMouseDelta()
	self.cursor.x = self.cursor.x + mouseDelta.x * MOUSE_SENSITIVITY.x * elapsedTime
	self.cursor.y = self.cursor.y - mouseDelta.y * MOUSE_SENSITIVITY.y * elapsedTime
	local stick = InputHandler:gamepad(0):leftStick() + InputHandler:gamepad(0):rightStick()
	self.cursor.x = self.cursor.x + stick.x * STICK_SENSITIVITY.x * elapsedTime
	self.cursor.y = self.cursor.y + stick.y * STICK_SENSITIVITY.y * elapsedTime
	self.cursor.x = math.clamp(self.cursor.x, -1, 1)
	self.cursor.y = math.clamp(self.cursor.y, -1, 1)
	if (self.guns[self.activeGun]:canShootOrReload()) then
		drawCross(self.cursor, CURSOR_SIZE, CURSOR_COLOR_NORMAL)
	else
		drawCross(self.cursor, CURSOR_SIZE, CURSOR_COLOR_BLOCKED)
	end
	
	-- camera
	local look = Vec2(0, 0)
	if (self.cursor.x > CAMERA_WINDOW_MAX.x) then
		look.x =  (self.cursor.x - CAMERA_WINDOW_MAX.x) * (1 / CAMERA_BORDER.x) * CAMERA_SENSITIVITY.x * elapsedTime
	end
	if (self.cursor.x < CAMERA_WINDOW_MIN.x) then
		look.x = -(CAMERA_WINDOW_MIN.x - self.cursor.x) * (1 / CAMERA_BORDER.x) * CAMERA_SENSITIVITY.x * elapsedTime
	end
	if (self.cursor.y > CAMERA_WINDOW_MAX.y) then
		look.y = -(self.cursor.y - CAMERA_WINDOW_MAX.y) * (1 / CAMERA_BORDER.y) * CAMERA_SENSITIVITY.y * elapsedTime
	end
	if (self.cursor.y < CAMERA_WINDOW_MIN.y) then
		look.y =  (CAMERA_WINDOW_MIN.y - self.cursor.y) * (1 / CAMERA_BORDER.y) * CAMERA_SENSITIVITY.y * elapsedTime
	end
	if (look:length() > 0) then
		self.cursor = self.cursor:mulScalar(1 - 0.5 * elapsedTime)
	end
	self.cc:look(look)
	DebugRenderer:drawLine2D(Vec2(CAMERA_WINDOW_MIN.x, CAMERA_WINDOW_MAX.y), Vec2(CAMERA_WINDOW_MAX.x, CAMERA_WINDOW_MAX.y))
	DebugRenderer:drawLine2D(Vec2(CAMERA_WINDOW_MAX.x, CAMERA_WINDOW_MAX.y), Vec2(CAMERA_WINDOW_MAX.x, CAMERA_WINDOW_MIN.y))
	DebugRenderer:drawLine2D(Vec2(CAMERA_WINDOW_MAX.x, CAMERA_WINDOW_MIN.y), Vec2(CAMERA_WINDOW_MIN.x, CAMERA_WINDOW_MIN.y))
	DebugRenderer:drawLine2D(Vec2(CAMERA_WINDOW_MIN.x, CAMERA_WINDOW_MIN.y), Vec2(CAMERA_WINDOW_MIN.x, CAMERA_WINDOW_MAX.y))
	
	-- changing weapons
	if (self.guns[self.activeGun]:canShootOrReload()) then
		local leftShoulder = bit32.btest(InputHandler:gamepad(0):buttonsTriggered(), Button.LeftShoulder)
		local rightShoulder = bit32.btest(InputHandler:gamepad(0):buttonsTriggered(), Button.RightShoulder)
		local keyLeft = InputHandler:wasTriggered(Key.Left)
		local keyRight = InputHandler:wasTriggered(Key.Right)
		if (leftShoulder or keyLeft) then
			self.guns[self.activeGun]:setActive(false, false)
			if (self.activeGun == 1) then
				self.activeGun = #self.guns
			else
				self.activeGun = self.activeGun - 1
			end
			self.guns[self.activeGun]:setActive(true, false)
		end
		if (rightShoulder or keyRight) then
			self.guns[self.activeGun]:setActive(false, false)
			if (self.activeGun == #self.guns) then
				self.activeGun = 1
			else
				self.activeGun = self.activeGun + 1
			end
			self.guns[self.activeGun]:setActive(true, false)
		end
	end	
	
	-- shooting
	local screenRay = self.cc:getRayForNormalizedScreenPos(self.cursor)
	DebugRenderer:printText(Vec2(0.4, 0.65), "screenRay.from: " .. string.format("%5.2f", screenRay.from.x) .. ", " .. string.format("%5.2f", screenRay.from.y) .. ", " .. string.format("%5.2f", screenRay.from.z))
	DebugRenderer:printText(Vec2(0.4, 0.60), "screenRay.direction: " .. string.format("%5.2f", screenRay.direction.x) .. ", " .. string.format("%5.2f", screenRay.direction.y) .. ", " .. string.format("%5.2f", screenRay.direction.z))
	local rayIn = RayCastInput()
	rayIn.from = screenRay.from
	--rayIn.to = screenRay.from + screenRay.direction:mulScalar(ZOMBIE_SPAWN_DISTANCE[2])
	rayIn.to = screenRay.from + screenRay.direction:mulScalar(10000)
	DebugRenderer:printText(Vec2(0.4, 0.55), "rayIn.from: " .. string.format("%5.2f", rayIn.from.x) .. ", " .. string.format("%5.2f", rayIn.from.y) .. ", " .. string.format("%5.2f", rayIn.from.z))
	DebugRenderer:printText(Vec2(0.4, 0.50), "rayIn.to: " .. string.format("%5.2f", rayIn.to.x) .. ", " .. string.format("%5.2f", rayIn.to.y) .. ", " .. string.format("%5.2f", rayIn.to.z))
	DebugRenderer:drawLine3D(rayIn.from, rayIn.to, Color(1, 0, 1, 1))
	rayIn.filterInfo = 0xFF00
	local rayOut = world:castRay(rayIn)
	DebugRenderer:printText(Vec2(-0.8, 0.50), "rayOut:hasHit(): " .. tostring(rayOut:hasHit()))
	local leftMousePressed = InputHandler:isPressed(Key.LButton)
	local shoulderButtonPressed = (InputHandler:gamepad(0):leftTrigger() >= 0.95 or InputHandler:gamepad(0):rightTrigger() >= 0.95)
	local fire = leftMousePressed or shoulderButtonPressed
	if (fire and self.guns[self.activeGun]:shoot()) then
		if (rayOut:hasHit()) then
			local zombie = hitZombieBodyPart(rayOut.hitBody, self.guns[self.activeGun].damage)
			if (zombie.canSpawn) then
				self.score = self.score + 1
				player.killSound:play()
			end
		end
		local recoil = self.guns[self.activeGun]:getRecoil()
		self.cursor = self.cursor + recoil:mulScalar(0.01)
	end
	
	-- reloading
	local rightMousePressed = InputHandler:isPressed(Key.RButton)
	local padABXYPressed = bit32.btest(InputHandler:gamepad(0):buttonsPressed(), bit32.bor(Button.A, Button.B, Button.X, Button.Y))
	local reload = rightMousePressed or padABXYPressed
	if (reload) then
		self.guns[self.activeGun]:reload()
	end
	
	-- time goes by...
	player.ambienceTime = math.clamp(player.ambienceTime + 0.1 * elapsedTime, 0, 1)
	player.ambience:setParameter("Time", player.ambienceTime)
end

function __player__setActive(self, active)
	if (active) then
		self:setPosition(Vec3(0, 0, PLAYER_HEIGHT))
		self.cc:lookAt(Vec3(1, 0, PLAYER_HEIGHT))
		self.cursor = Vec2(0, 0)
		self.activeGun = 1
		self.score = 0
		for i, v in ipairs(self.guns) do
			v:setActive(i == self.activeGun, true)
		end
		self.sc:setState(ComponentState.Active)
	else
		self.sc:setState(ComponentState.Inactive)
	end
end

function createPlayer(guid)
	local player = GameObjectManager:createGameObject(guid)
	player.cc = player:createCameraComponent()
	player.sc = player:createScriptComponent()
	player:setPosition(Vec3(0, 0, PLAYER_HEIGHT))
	player.cc:setViewDirection(Vec3(1, 0, 0))
	player.sc:setUpdateFunction(__player__update)
	player.setActive = __player__setActive
	player.guns = {}
	player.guns[1] = createGun("Pistol", 20, 0.5,  3,  6, 2)
	player.guns[2] = createGun("Magnum", 55, 1.0,  6,  2, 2)
	player.guns[3] = createGun("MG",     10, 0.1,  1, 20, 3)
	for _, v in ipairs(player.guns) do
		v:setActive(false, true)
	end
	player.au = player:createAudioComponent()
	player.guns[1].shootSound = player.au:createSoundInstance("ShootSound", "/Weapons/Single-Shot")
	player.guns[2].shootSound = player.au:createSoundInstance("ExplosionSound", "/Explosions/Single Explosion")
	player.guns[2].shootSound:setParameter("Size", 0.8)
	player.guns[3].shootSound = player.guns[1].shootSound
	player.guns[1].reloadSound = player.au:createSoundInstance("ReloadSound", "/Character/Hand Foley/Doorknob")
	player.guns[2].reloadSound = player.guns[1].reloadSound
	player.guns[3].reloadSound = player.guns[1].reloadSound
	player.ambienceTime = 0
	player.ambience = player.au:createSoundInstance("AmbienceCountry", "/Ambience/Country")
	player.ambience:setParameter("Time", player.ambienceTime)
	player.ambience:play()
	player.killSound = player.au:createSoundInstance("UICancel", "/UI/Cancel")
	return player
end
