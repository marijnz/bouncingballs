logMessage("using levelbuilder.lua")

LevelBuilder = {}
LevelBuilder.__index = LevelBuilder

setmetatable(LevelBuilder, {
  __call = function (cls, ...)
    local self = setmetatable({}, cls)
    self:_initialize()
    return self
  end,
})

function LevelBuilder:_initialize()
	go = GameObjectManager:createGameObject("levelBuilder")
	
	self.buildGUID = 0
	self.go = go
end

function LevelBuilder:CreateFloor()

end


function LevelBuilder:CreateWall(width, angle, position)
	local wall = GameObjectManager:createGameObject("wall" .. buildGUID)
	buildGUID = buildGUID + 1
	wall.pc = wall:createPhysicsComponent()
	
	local cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createBox(Vec3(width, 0.1, 2.5))
	cinfo.motionType = MotionType.Fixed
	cinfo.position = position
	cinfo.rotation = Quaternion(Vec3(0, 0, 1), angle)
	wall.rb = wall.pc:createRigidBody(cinfo)
	
	return wall

end

function LevelBuilder:update(deltaTime)
	
end
