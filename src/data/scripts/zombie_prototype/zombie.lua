
ZOMBIE_COUNT					= 25
ZOMBIE_SPAWN_DELAY_MIN			= 0.5
ZOMBIE_SPAWN_DELAY_MAX			= 5.0
ZOMBIE_SPAWN_DELAY_REDUCTION	= 0.1
ZOMBIE_SPAWN_DISTANCE			= {1000, 1250}
ZOMBIE_SPAWN_ROTATION			= {-55, 55}

ZOMBIE_HEALTH					= 100
ZOMBIE_HIT_RECOVERY_TIME		= 0.5
ZOMBIE_TORSOSHOT_MULTIPLIER		= 1.5
ZOMBIE_HEADSHOT_MULTIPLIER		= 2.5
ZOMBIE_ARMSHOT_MULTIPLIER		= 0.75
ZOMBIE_LEGSHOT_MULTIPLIER		= 0.5
ZOMBIE_SPEED					= 50 * 0.5
ZOMBIE_SPEED_LIMPING_MULTIPLIER	= {0.6, 0.3}

function __zombie__initialize(self)
	self:setBaseViewDirection(Vec3(0, -1, 0))
	self.ac:setReferencePoseWeightThreshold(1)
	self.ac:setMasterWeight("Idle", 0.8)
	self.ac:setPlaybackSpeed("Idle", 0.5)
	self.ac:setMasterWeight("Walk", 1.0)
	self.ac:setPlaybackSpeed("Walk", 0.35)
	self.ac:setMasterWeight("Stagger", 0.2)
	self.ac:setPlaybackSpeed("Stagger", 0.5)
	self.bodyParts["torso"]:setParent(self.ac:getBoneByName("spine_jnt"))
	self.bodyParts["head"]:setParent(self.ac:getBoneByName("head_jnt"))
	self.bodyParts["leftUpperArm"]:setParent(self.ac:getBoneByName("left_upperArm_jnt"))
	self.bodyParts["rightUpperArm"]:setParent(self.ac:getBoneByName("right_upperArm_jnt"))
	self.bodyParts["leftLowerArm"]:setParent(self.ac:getBoneByName("left_elbow_jnt"))
	self.bodyParts["rightLowerArm"]:setParent(self.ac:getBoneByName("right_elbow_jnt"))
	self.bodyParts["leftThigh"]:setParent(self.ac:getBoneByName("left_thigh_jnt"))
	self.bodyParts["rightThigh"]:setParent(self.ac:getBoneByName("right_thigh_jnt"))
	self.bodyParts["leftCalf"]:setParent(self.ac:getBoneByName("left_knee_jnt"))
	self.bodyParts["rightCalf"]:setParent(self.ac:getBoneByName("right_knee_jnt"))
end

function __zombie__update(guid, elapsedTime)
	local self = __zombies[guid]
	local pos = self:getPosition()
	local rightDir = self:getRightDirection()
	local relativeHealth = self.health / ZOMBIE_HEALTH
	local healthFrom = pos + rightDir:mulScalar(-40 * relativeHealth) + Vec3(0, 0, 175)
	local healthTo = pos + rightDir:mulScalar(40 * relativeHealth) + Vec3(0, 0, 175)
	DebugRenderer:drawLine3D(healthFrom, healthTo, Color(1 - relativeHealth, relativeHealth, 0, 1))
	if (self.hitRecoveryTime > 0) then
		self.hitRecoveryTime = math.clamp(self.hitRecoveryTime - elapsedTime, 0, ZOMBIE_HIT_RECOVERY_TIME)
		if (self.hitRecoveryTime == 0) then
			self.ac:easeOut("Stagger", 0.5)
		end
	else
		local viewDir = self:getViewDirection()
		self:setPosition(pos + viewDir:mulScalar(self.speed * elapsedTime))
	end
end

function hitZombieBodyPart(hitBody, damage)
	local bodyPartPC = hitBody:getUserData()
	local bodyPartGO = bodyPartPC:getParentGameObject()
	local zombie = __zombies[bodyPartGO:getGuid()]
	zombie:takeDamage(bodyPartGO, damage)
	return zombie
end

function __zombie__isBodyPart(self, bodyPart, names)
	local result = false
	for _, v in ipairs(names) do
		result = result or bodyPart:getGuid() == self.bodyParts[v]:getGuid()
		print("comparing " .. bodyPart:getGuid() .. " with " .. self.bodyParts[v]:getGuid())
	end
	print("returning " .. tostring(result))
	return result
end

function __zombie__takeDamage(self, bodyPart, damage)
	if (self:isBodyPart(bodyPart, {"torso"})) then
		damage = damage * ZOMBIE_TORSOSHOT_MULTIPLIER
	elseif (self:isBodyPart(bodyPart, {"head"})) then
		damage = damage * ZOMBIE_HEADSHOT_MULTIPLIER
	elseif (self:isBodyPart(bodyPart, {"leftUpperArm", "rightUpperArm", "leftLowerArm", "rightLowerArm"})) then
		damage = damage * ZOMBIE_ARMSHOT_MULTIPLIER
	elseif (self:isBodyPart(bodyPart, {"leftThigh", "rightThigh", "leftCalf", "rightCalf"})) then
		damage = damage * ZOMBIE_LEGSHOT_MULTIPLIER
		if (self.legDamage < #ZOMBIE_SPEED_LIMPING_MULTIPLIER) then
			self.legDamage = self.legDamage + 1
			self.speed = ZOMBIE_SPEED * ZOMBIE_SPEED_LIMPING_MULTIPLIER[self.legDamage]
		end
	end
	self.health = self.health - damage
	self.hitRecoveryTime = ZOMBIE_HIT_RECOVERY_TIME
	self.ac:easeIn("Stagger", 0.2)
	self.ac:setLocalTimeNormalized("Stagger", math.clamp(self.ac:getLocalTimeNormalized("Stagger") + 0.1, 0, 1))
	if (self.health <= 0) then
		self:setActive(false)
	end
end

function __zombie__setActive(self, active)
	if (active) then
		self.canSpawn = false
		self.health = ZOMBIE_HEALTH
		self.hitRecoveryTime = 0
		self.speed = ZOMBIE_SPEED
		self.legDamage = 0
		self.ac:easeOut("Stagger", 0)
		self:setComponentStates(ComponentState.Active)
		for _, v in pairs(self.bodyParts) do
			v:setComponentStates(ComponentState.Active)
		end
		self.rc:setState(ComponentState.Inactive)
	else
		self.canSpawn = true
		self:setPosition(Vec3(0, 0, -500))
		self:setComponentStates(ComponentState.Inactive)
		for _, v in pairs(self.bodyParts) do
			v:setComponentStates(ComponentState.Inactive)
		end
	end
end

function __zombie__spawn(self)
	local pos = Vec3(math.random(ZOMBIE_SPAWN_DISTANCE[1], ZOMBIE_SPAWN_DISTANCE[2]), 0, 0)
	local rorQuat = Quaternion(Vec3(0, 0, 1), math.random(ZOMBIE_SPAWN_ROTATION[1], ZOMBIE_SPAWN_ROTATION[2]))
	local posRot = rorQuat:toMat3():mulVec3(pos)
	self:setPosition(posRot)
	local initialRotQuat = Quaternion(Vec3(0, 0, 1), -90)
	self:setRotation(initialRotQuat * rorQuat)
	self:setActive(true)
end

__zombies = {}
function createZombie(guid)
	local zombie = GameObjectManager:createGameObject(guid)
	__zombies[guid] = zombie
	zombie.sc = zombie:createScriptComponent()
	zombie.sc:setUpdateFunction(__zombie__update)
	zombie.rc = zombie:createRenderComponent()
	zombie.rc:setPath("data/models/barbarian/barbarian.thModel")
	zombie.ac = zombie:createAnimationComponent()
	zombie.ac:setSkeletonFile("data/animations/Barbarian/Barbarian.hkt")
	zombie.ac:setSkinFile("data/animations/Barbarian/Barbarian.hkt")
	zombie.ac:addAnimationFile("Idle", "data/animations/Barbarian/Barbarian_Idle.hkt")
	zombie.ac:addAnimationFile("Walk", "data/animations/Barbarian/Barbarian_Walk.hkt")
	zombie.ac:addAnimationFile("Stagger", "data/animations/Barbarian/Barbarian_Stagger.hkt")
	zombie.ac:setBoneDebugDrawingEnabled(true)
	zombie.bodyParts = {}
	zombie.bodyParts["torso"] = createCollisionCapsule(guid .. "_torso", Vec3(22, 2, 0), Vec3(-6, 2, 0), 22)
	zombie.bodyParts["head"] = createCollisionCapsule(guid .. "_head", Vec3(10, 2, 0), Vec3(-5, 2, 0), 10)
	zombie.bodyParts["leftUpperArm"] = createCollisionCapsule(guid .. "_leftUpperArm", Vec3(30, 0, 0), Vec3(-6, 0, 0), 10)
	zombie.bodyParts["rightUpperArm"] = createCollisionCapsule(guid .. "_rightUpperArm", Vec3(6, 0, 0), Vec3(-30, 0, 0), 10)
	zombie.bodyParts["leftLowerArm"] = createCollisionCapsule(guid .. "_leftLowerArm", Vec3(42, 0, 0), Vec3(0, 0, 0), 8)
	zombie.bodyParts["rightLowerArm"] = createCollisionCapsule(guid .. "_rightLowerArm", Vec3(-42, 0, 0), Vec3(0, 0, 0), 8)
	zombie.bodyParts["leftThigh"] = createCollisionCapsule(guid .. "_leftThigh", Vec3(32, 0, 0), Vec3(0, 0, 0), 11)
	zombie.bodyParts["rightThigh"] = createCollisionCapsule(guid .. "_rightThigh", Vec3(-32, 0, 0), Vec3(0, 0, 0), 11)
	zombie.bodyParts["leftCalf"] = createCollisionCapsule(guid .. "_leftCalf", Vec3(38, 0, 0), Vec3(0, 0, 0), 9)
	zombie.bodyParts["rightCalf"] = createCollisionCapsule(guid .. "_rightCalf", Vec3(-38, 0, 0), Vec3(0, 0, 0), 9)
	for _, v in pairs(zombie.bodyParts) do
		__zombies[v:getGuid()] = zombie	
	end
	zombie.initialize = __zombie__initialize
	zombie.isBodyPart = __zombie__isBodyPart
	zombie.takeDamage = __zombie__takeDamage
	zombie.setActive = __zombie__setActive
	zombie.spawn = __zombie__spawn
	return zombie
end
