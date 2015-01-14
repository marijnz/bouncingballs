logMessage("using player.lua")

ROBOT_CAMERA_OFFSET = Vec3(0.5, 0, 0.55);
BULLET_RECOIL_TIME = 0.5

-- Player/robot model
player = GameObjectManager:createGameObject("player")
player.render = player:createRenderComponent()
player.render:setPath("data/models/robot.thModel")

-- Player rigid body for collision and physics
player.pc = player:createPhysicsComponent()
cinfo = RigidBodyCInfo()
cinfo.shape = PhysicsFactory:createBox(Vec3(0.5, 0.5, 0.25))
cinfo.motionType = MotionType.Dynamic
cinfo.mass = 1.0
cinfo.friction = 1.0
cinfo.angularDamping = 0.0
cinfo.restitution = 1.0
cinfo.position = Vec3(0.0, 0.0, FLOOR_Z + 0.25)
cinfo.maxAngularVelocity = 0.0
player.pc:createRigidBody(cinfo)
player.speed = 5

-- Robot camera model
player.robotCamera = GameObjectManager:createGameObject("robotCamera")
player.robotCamera.render = player.robotCamera:createRenderComponent()
player.robotCamera.render:setPath("data/models/robot-camera.thModel")
player.robotCameraRotation = 0
player.robotCameraRotationDirection = 1
player.robotCamera:setPosition(player:getPosition() + ROBOT_CAMERA_OFFSET)

bulletCooldown = 0.2

-- Rotates the camera of the robot slowly and takes the player rotation in account to
-- calculate the right rotation
-- <playerRotation> is a Quaternion
updateRobotCameraRotation = function (playerRotation) 
    player.robotCameraRotation = player.robotCameraRotation + (player.robotCameraRotationDirection * 1)
    if (math.abs(player.robotCameraRotation) > 35) then
        player.robotCameraRotationDirection = player.robotCameraRotationDirection * -1
    end
    local rotation = Quaternion(Vec3(0, 0, 1), player.robotCameraRotation)
    player.robotCamera:setRotation(rotation * playerRotation)
end

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

    -- If a direction is set, walk & rotate
    if (direction.x ~= 0 or direction.y ~= 0) then
        -- Normalize the direction for when two buttons are pressed at the same time
        local normalized = direction:normalized()

        -- Multiply speed by the direction to walk
        player.pc:getRigidBody():setLinearVelocity(Vec3(normalized.x * player.speed, normalized.y * player.speed, 0.0))

        -- Set the rotation of the player/robot based on the direction it is moving
        local angle = toDegrees(vec2Angle(normalized))
        player:setRotation(Quaternion(Vec3(0, 0, 1), angle))

        -- Put the camera of the robot on the right place after the player moved, based on the ROBOT_CAMERA_OFFSET
        local offset = vec2Rotate(ROBOT_CAMERA_OFFSET, angle)
        player.robotCamera:setPosition(player:getPosition() + offset)
    else 
        player.pc:getRigidBody():setLinearVelocity(Vec3(0.0, 0.0, 0.0))
    end

    -- Make the camera of the robot rotation slowly
    updateRobotCameraRotation(player:getRotation())
	
	-- space press
	if(bulletCooldown ~= 0.2) then
		bulletCooldown = bulletCooldown - deltaTime
		if(bulletCooldown < 0) then
			bulletCooldown = 0.2
		end
	elseif (InputHandler:isPressed(32)) then
		bullet = objectManager:grab(Bullet)
		bullet:setInitialPosition(player:getPosition() + Vec3(0,0,1.5))
		bulletCooldown = bulletCooldown - deltaTime
    end

end
player.sc = player:createScriptComponent()
player.sc:setUpdateFunction(player.update)