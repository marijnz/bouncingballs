--
-- physics world
--
do
	local cinfo = WorldCInfo()
	cinfo.gravity = Vec3(0, 0, -9.81)
	cinfo.worldSize = 4000.0
	local world = PhysicsFactory:createWorld(cinfo)
	PhysicsSystem:setWorld(world)
	PhysicsSystem:setDebugDrawingEnabled(true)
end

--
-- dummy state machine
-- TODO remove
--
State{
	name = "dummy",
	parent = "/game"
}
StateTransitions{
	parent = "/game",
	{ from = "__enter", to = "dummy" }
}
