
logMessage("using ball.lua")

balls = {}





function balls:__call(name, hp, startpos)

local ball = GameObjectManager:createGameObject(name)
ball.pc = ball:createPhysicsComponent()
cinfo = RigidBodyCInfo()
cinfo.shape = PhysicsFactory:createSphere(0.5)
cinfo.motionType = MotionType.Dynamic
cinfo.mass = 1.0
cinfo.friction = 0.0
cinfo.angularDamping = 0.0
cinfo.restitution = 1.0
cinfo.position = startpos
cinfo.linearVelocity = Vec3(2.0, 1.0, 0.0)
ball.pc:createRigidBody(cinfo)

ball.hp = hp
	
return ball

end

--balls.__call = balls:init
setmetatable(balls, balls)
--[[
balls:update = function (deltaTime)

	
	
end 
]]--