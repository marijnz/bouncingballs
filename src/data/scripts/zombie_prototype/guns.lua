
function __gun__update(guid, elapsedTime)
	local self = __guns[guid]
	if (self.shootTimer > 0) then
		self.shootTimer = math.clamp(self.shootTimer - elapsedTime, 0, self.shootDelay)
	end
	if (self.reloadTimer > 0) then
		self.reloadTimer = math.clamp(self.reloadTimer - elapsedTime, 0, self.reloadDelay)
		if (self.reloadTimer == 0) then
			self.bullets = self.magazineSize
			self.shootTimer = 0
		end
	end
	DebugRenderer:printText(Vec2(-0.8, 0.65), "gun:     " .. self.name)
	DebugRenderer:printText(Vec2(-0.8, 0.60), "bullets: " .. self.bullets .. " / " .. self.magazineSize)
	if (self.reloadTimer > 0) then
		DebugRenderer:printText(Vec2(-0.8, 0.55), "reloading: " .. string.format("%5.2f", self.reloadTimer))
	end
end

function __gun__canShootOrReload(self)
	return (self.shootTimer == 0 and self.reloadTimer == 0)
end

function __gun__shoot(self)
	if (self:canShootOrReload()) then
		if (self.bullets > 0) then
			self.bullets = self.bullets - 1
			self.shootTimer = self.shootDelay
			self.shootSound:stop()
			self.shootSound:play()
			return true
		else
			self:reload()
			return false
		end
	end
	return false
end

function __gun__reload(self)
	if (self.bullets < self.magazineSize and self.reloadTimer == 0) then
		self.shootTimer = 0
		self.reloadTimer = self.reloadDelay
		self.reloadSound:play()
	end
end

function __gun__getRecoil(self)
	return Vec2(math.random(-self.recoil, self.recoil), math.random(-self.recoil, self.recoil))
end

function __gun__setActive(self, active, reset)
	self.shootTimer = 0
	self.reloadTimer = 0
	if (self.bullets == nil) then
		self.bullets = self.magazineSize
	end
	if (active) then
		if (reset) then
			self.bullets = self.magazineSize
		else
			self:reload()
		end
		self:setComponentStates(ComponentState.Active)
	else
		self:setComponentStates(ComponentState.Inactive)
	end
end

__guns = {}
function createGun(guid, damage, shootDelay, recoil, magazineSize, reloadDelay)
	local gun = GameObjectManager:createGameObject(guid)
	__guns[guid] = gun
	gun.sc = gun:createScriptComponent()
	gun.sc:setUpdateFunction(__gun__update)
	gun.name = guid
	gun.damage = damage
	gun.shootDelay = shootDelay
	gun.recoil = recoil
	gun.magazineSize = magazineSize
	gun.reloadDelay = reloadDelay
	gun.canShootOrReload = __gun__canShootOrReload
	gun.shoot = __gun__shoot
	gun.reload = __gun__reload
	gun.getRecoil = __gun__getRecoil
	gun.setActive = __gun__setActive
	return gun
end
