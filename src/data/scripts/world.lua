logMessage("using world.lua")

do -- Physics world
	local cinfo = WorldCInfo()
	cinfo.gravity = Vec3(0, 0, -9.8)
	cinfo.worldSize = 2000.0
	world = PhysicsFactory:createWorld(cinfo)
	PhysicsSystem:setWorld(world)
end
