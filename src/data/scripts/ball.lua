
logMessage("using ball.lua")

balls = {}


function balls:__call(name, hp, startpos, startvel)

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
cinfo.linearVelocity = startvel
ball.pc:createRigidBody(cinfo)
local render = ball:createRenderComponent()
render:setPath("data/models/balls/mediumBall.thModel")

ball.hp = hp
	
rawset(balls, name, ball)

end

--balls.__call = balls:init
setmetatable(balls, balls)
--[[
balls:update = function (deltaTime)

	
	
end 
]]--