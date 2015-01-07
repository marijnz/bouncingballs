logMessage("using player.lua")

player = GameObjectManager:createGameObject("player")
player.render = player:createRenderComponent()
player.render:setPath("data/models/player.thModel")
player:setPosition(Vec3(0, 0, 0))

player.pc = player:createPhysicsComponent()
cinfo = RigidBodyCInfo()
cinfo.shape = PhysicsFactory:createCapsule(Vec3(0.0, 0.0, 0.35), Vec3(0.0, 0.0, 1.05), 0.4)
cinfo.motionType = MotionType.Dynamic
cinfo.mass = 1.0
cinfo.friction = 1.0
cinfo.angularDamping = 0.0
cinfo.restitution = 1.0
cinfo.position = Vec3(0.0, 0.0, 0.0)
cinfo.maxAngularVelocity = 0.0
player.pc:createRigidBody(cinfo)

player.speed = 5

bulletCooldown = 0.2

--local bullet = Bullet(player:getPosition() + Vec3(0,0,0))
player.update = function (guid, deltaTime) 	
    -- The direction the player is going to walk this frame
    local direction = Vec3(0.0, 0.0, 0.0)
    if (InputHandler:isPressed(Key.Up)) then
        direction = direction + Vec3(-1, -1, 0)
    end
    if (InputHandler:isPressed(Key.Down)) then
        direction = direction + Vec3(1, 1, 0)
    end
    if (InputHandler:isPressed(Key.Left)) then
        direction = direction + Vec3(1, -1, 0)
    end
    if (InputHandler:isPressed(Key.Right)) then
        direction = direction + Vec3(-1, 1, 0)
    end
	
	-- space press
	if(bulletCooldown ~= 0.2) then
		bulletCooldown = bulletCooldown - deltaTime
		if(bulletCooldown < 0) then
			bulletCooldown = 0.2
		end
	elseif (InputHandler:isPressed(32)) then
		bullet = objectManager:grab(Bullet)
		bullet:setPosition(player:getPosition() + Vec3(0,0,1.5))
		bulletCooldown = bulletCooldown - deltaTime
    end

    -- If a direction is set, walk 
    if (direction.x ~= 0 or direction.y ~= 0) then
        -- Normalize the direction for when two buttons are pressed at the same time
        local normalized = direction:normalized()
        -- Multiply speed by the direction to walk
        -- player:setPosition(player:getPosition() + Vec3(normalized.x * player.speed, normalized.y * player.speed, normalized.z * player.speed))

        player.pc:getRigidBody():setLinearVelocity(Vec3(normalized.x * player.speed, normalized.y * player.speed, 0.0))
    else 
        player.pc:getRigidBody():setLinearVelocity(Vec3(0.0, 0.0, 0.0))
    end
end
player.sc = player:createScriptComponent()
player.sc:setUpdateFunction(player.update)

