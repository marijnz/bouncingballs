logMessage("using defaults/physicsWorld.lua")

do -- Physics world
	local cinfo = WorldCInfo()
	cinfo.gravity = Vec3(0, 0, 0)
	cinfo.worldSize = 2000.0
	physicsWorld = PhysicsFactory:createWorld(cinfo)
	PhysicsSystem:setWorld(physicsWorld)
end
