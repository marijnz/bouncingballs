
function createCharacter(guid, position)
	local character = GameObjectManager:createGameObject(guid)
	character.rc = character:createRenderComponent()
	character.rc:setPath("data/models/barbarian/barbarian.thModel")
	character.ac = character:createAnimationComponent()
	character.ac:setSkeletonFile("data/animations/Barbarian/Barbarian.hkt")
	character.ac:setSkinFile("data/animations/Barbarian/Barbarian.hkt")
	character.ac:addAnimationFile("Idle", "data/animations/Barbarian/Barbarian_Idle.hkt")
	character.ac:addAnimationFile("Walk", "data/animations/Barbarian/Barbarian_Walk.hkt")
	character.ac:addAnimationFile("Run", "data/animations/Barbarian/Barbarian_Run.hkt")
	character.ac:addAnimationFile("Attack", "data/animations/Barbarian/Barbarian_Attack.hkt")
	character.pc = character:createPhysicsComponent()
	local cinfo = RigidBodyCInfo()
	cinfo.collisionFilterInfo = 0x1
	local sphere = PhysicsFactory:createSphere(50)
	cinfo.shape = PhysicsFactory:createConvexTranslateShape(sphere, Vec3(0, 0, 50))
--	local capsule = PhysicsFactory:createCapsule(Vec3(0, 0, 0), Vec3(0, 0, 80), 40)
--	cinfo.shape = PhysicsFactory:createConvexTranslateShape(capsule, Vec3(0, 0, 120))
	cinfo.motionType = MotionType.Dynamic
	cinfo.position = position
	cinfo.mass = 100
	cinfo.restitution = 0
	cinfo.friction = 0
	cinfo.linearDamping = 2.5
	cinfo.angularDamping = 1
	cinfo.gravityFactor = 25
	cinfo.maxLinearVelocity = 300
	cinfo.maxAngularVelocity = 200
	character.pc.rb = character.pc:createRigidBody(cinfo)
	character.pc.rb:setUserData(character)
	character:setBaseViewDirection(Vec3(0, -1, 0):normalized())
	character.attacking = false
	character.walkSpeed = 0
	character.maxWalkSpeed = 10000
	character.relativeSpeed = 0
	character.rotationSpeed = 0
	character.maxRotationSpeed = 350
	character.walkAnimWeight = 0
	character.runAnimWeight = 0
	
	-- initialize function
	character.initialize = function(self)
	
		-- set the initial animation states
		self.ac:setReferencePoseWeightThreshold(0.1)
		self.ac:easeIn("Idle", 0.0)
		self.ac:setMasterWeight("Idle", 0.1)
		self.ac:easeIn("Walk", 0.0)
		self.ac:setMasterWeight("Walk", 0.0)
		self.ac:easeIn("Run", 0.0)
		self.ac:setMasterWeight("Run", 0.0)
		self.ac:easeOut("Attack", 0.0)
		self.ac:setMasterWeight("Attack", 1.0)
		self.ac:setPlaybackSpeed("Attack", 1.5)
	end
	
	-- update function
	character.update = function(self, walkSpeed, rotationSpeed, attack)
		
		-- relative walking speed
		self.relativeSpeed = self.walkSpeed / self.maxWalkSpeed
		if (self.relativeSpeed < 0) then
			self.relativeSpeed = -self.relativeSpeed
		end
		
		-- start attack
		if (not self.attacking) then
			if (attack) then
				self.attacking = true
				self.walkSpeed = 0
				self.rotationSpeed = 0
				self.ac:easeOut("Walk", 0.2)
				self.ac:easeOut("Run", 0.2)
				self.ac:easeIn("Attack", 0.2)
				self.ac:setLocalTimeNormalized("Attack", 0.0)
			end
		end
		
		-- while attacking
		if (self.attacking) then
			local localTimeNormalized = self.ac:getLocalTimeNormalized("Attack")

			-- hitting stuff
			if (localTimeNormalized >= 0.35 and localTimeNormalized <= 0.55) then
				axe.pc.rb:setCollisionFilterInfo(0x2)
			else
				axe.pc.rb:setCollisionFilterInfo(0x0)
			end

			-- attack ends
			if (localTimeNormalized >= 0.90) then
				self.attacking = false
				self.ac:easeIn("Walk", 0.2)
				self.ac:easeIn("Run", 0.2)
				self.ac:easeOut("Attack", 0.2)
			end

		-- idle, walking, and running
		else

			-- walk speed
			self.walkSpeed = walkSpeed
			
			-- forward or backward walking
			if (self.walkSpeed < 0) then
				self.walkSpeed = self.walkSpeed * 0.75
				self.ac:setPlaybackSpeed("Walk", -1.0)
				self.ac:setPlaybackSpeed("Run", -1.0)
			else
				self.ac:setPlaybackSpeed("Walk", 1.75)
				self.ac:setPlaybackSpeed("Run", 1.75)
			end

			-- rotation speed
			self.rotationSpeed = rotationSpeed
			
			-- walk and run animation weights
			local maxWeight = 1.0
			local threshold = 0.65
			self.walkAnimWeight = 0.0
			self.runAnimWeight = 0.0
			if (self.relativeSpeed <= threshold) then
				self.walkAnimWeight = maxWeight * (self.relativeSpeed / threshold)
			else
				self.walkAnimWeight = maxWeight * (1.0 - ((self.relativeSpeed - threshold) / (1.0 - threshold)))
				self.runAnimWeight = maxWeight - self.walkAnimWeight
			end
			self.ac:setMasterWeight("Walk", self.walkAnimWeight)
			self.ac:setMasterWeight("Run", self.runAnimWeight)
		end

		-- set character forces
		local viewDirection = self:getViewDirection()
		self.pc.rb:applyLinearImpulse(viewDirection:mulScalar(self.walkSpeed))
		self.pc.rb:setAngularVelocity(Vec3(0, 0, self.rotationSpeed))
	end
	
	-- steering function
	character.calcSteering = function(self, moveVector)
	
		-- calculate the steering direction and amount
		local rightVec = self:getRightDirection()
		local steer = rightVec:dot(moveVector)
		local crossRightMove = rightVec:cross(moveVector)
		if (crossRightMove.z < 0) then
			if (steer < 0) then
				steer = -1
			else
				steer = 1
			end
		end
		return steer
	end	
	
	return character
end