logMessage("using level1.lua")

Level2 = {}
Level2.__index = Level2

setmetatable(Level2, {
  __index = PoolObject, -- this is what makes the inheritance work
  __call = function (cls, ...)
    self = setmetatable({}, cls)
    return self
  end,
})

function Level2:createLevel(center, id)
	level = GameObjectManager:createGameObject("level"..id)
	level.render = level:createRenderComponent()
	level.render:setPath("data/models/cube-level.thModel")
	level:setPosition(center + Vec3(-100, 0, 0))

	floor = GameObjectManager:createGameObject("floor"..id)
	floor:setParent(level)
    floor.pc = floor:createPhysicsComponent()
    local cinfo = RigidBodyCInfo()
    cinfo.shape = PhysicsFactory:createBox(Vec3(5, 5, 0.1))
    cinfo.motionType = MotionType.Keyframed
    floor.rb = floor.pc:createRigidBody(cinfo)
    floor.rb:setUserData({type = USERDATA_TYPE_FLOOR})

    local wallIndex = 0
    local createWall = function (width, angle, position)
        local wall = GameObjectManager:createGameObject(id.."wall" .. wallIndex)
		wall:setParent(level)
        wall.pc = wall:createPhysicsComponent()
        local cinfo = RigidBodyCInfo()
        cinfo.shape = PhysicsFactory:createBox(Vec3(width, 0.1, 2.5))
        cinfo.motionType = MotionType.Keyframed
        cinfo.position = center + position
        cinfo.rotation = Quaternion(Vec3(0, 0, 1), angle)
        wall.rb = wall.pc:createRigidBody(cinfo)
        wall.rb:setUserData({type = USERDATA_TYPE_WALL})
        wallIndex = wallIndex + 1
		
		return wall
    end

    wall1=createWall(5, 90, Vec3(4.9, 0.0, 2.5))
    wall2=createWall(5, 90, Vec3(-4.9, 0.0, 2.5))
    wall3=createWall(5, 0, Vec3(0.0, 4.9, 2.5))
    wall4=createWall(5, 0, Vec3(0.0, -4.9, 2.5))
	
	return level
end

function Level2:startLevel(center, id)
	local ball1 = objectManager:grab(MediumBall)
	ball1:setInitialPositionAndMovement(center + Vec3(0.0, 0.0, 5.0), Vec3(2.0, 2.0, 0.0))
	local ball2 = objectManager:grab(MediumBall)
	ball2:setInitialPositionAndMovement(center + Vec3(3.0, 0.0, 5.0), Vec3(-2.0, -2.0, 0.0))
end
