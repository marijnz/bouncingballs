-- Physics World
include("defaults/physicsWorld.lua")

-- Default state machine
include("defaults/stateMachine.lua")

-- Include our update functions
include("asteroids/update_stripped.lua")

-- Enable drawing of physics geometry
PhysicsSystem:setDebugDrawingEnabled(true)

-- -- Camera
-- cam = GameObjectManager:createGameObject("cam")
-- cam.cc = cam:createCameraComponent()
-- cam.cc:setPosition(Vec3(-300.0, 0.0, 0.0))
-- cam.cc:lookAt(Vec3(0, 0, 0))
-- cam.cc:setFar(4600)
-- cam.orthographic = true
-- cam.cc:setOrthographic(cam.orthographic)

function createShip(guid)
	local ship = {}
--	-- game object
--	ship.guid = guid
--	ship.go = GameObjectManager:createGameObject(guid)
--	-- physics component
--	ship.ph = ship.go:createPhysicsComponent()
--	local cinfo = RigidBodyCInfo()
--	cinfo.shape = PhysicsFactory:createBox(Vec3(5, 8, 5))
--	cinfo.mass = 50
--	cinfo.maxLinearVelocity = 100
--	cinfo.motionType = MotionType.Dynamic
--	ship.rb = ship.ph:createRigidBody(cinfo)
--	-- collision event
--	ship.ph:getContactPointEvent():registerListener(shipCollision)
--	-- script component
--	ship.sc = ship.go:createScriptComponent()
--	ship.sc:setUpdateFunction(updateShip)
	return ship
end
print("\tcreating ship")
ship = createShip("ship")
print("\tship created")

function createLaser(guid)
	local laser = {}
--	-- game object
--	laser.guid = guid
--	laser.go = GameObjectManager:createGameObject(guid)
--	-- physics component
--	laser.ph = laser.go:createPhysicsComponent()
--	local cinfo = RigidBodyCInfo()
--	cinfo.shape = PhysicsFactory:createBox(Vec3(1, 4, 1))
--	cinfo.mass = 5
--	cinfo.motionType = MotionType.Dynamic
--	laser.rb = laser.ph:createRigidBody(cinfo)
--	-- collision event
--	laser.ph:getContactPointEvent():registerListener(laserCollision)
--	-- script component
--	laser.sc = laser.go:createScriptComponent()
--	laser.sc:setUpdateFunction(updateLaser)
--	-- deactivate
--	laser.go:setComponentStates(ComponentState.Inactive)
--	-- other members
--	laser.lifetime = 0
	return laser
end
print("\tcreating laser")
laser = createLaser("laser")
print("\tlaser created")

function createAsteroid(guid)
	local asteroid = {}
--	-- game object
--	asteroid.guid = guid
--	asteroid.go = GameObjectManager:createGameObject(guid)
--	-- physics component
--	asteroid.ph = asteroid.go:createPhysicsComponent()
--	local cinfo = RigidBodyCInfo()
--	cinfo.shape = PhysicsFactory:createSphere(20)
--	cinfo.motionType = MotionType.Keyframed
--	asteroid.rb = asteroid.ph:createRigidBody(cinfo)
--	asteroid.rb:setUserData({score=10})
--	-- script component
--	asteroid.sc = asteroid.go:createScriptComponent()
--	asteroid.sc:setUpdateFunction(updateAsteroid)
--	-- deactivate
--	asteroid.go:setComponentStates(ComponentState.Inactive)
--	-- other members
--	asteroid.destructible = true
--	asteroid.destroyed = true
	return asteroid
end
print("\tcreating asteroid")
asteroid = createAsteroid("asteroid")
print("\tasteroid created")

-- register global update function
Events.Update:registerListener(update)

math.randomseed(os.clock())

print("Finished initializing game.")
