
include("utils/physicsWorld.lua")
include("utils/stateMachine.lua")
include("utils/freeCamera.lua")

-- PhysicsDebugView
PhysicsSystem:setDebugDrawingEnabled(true)

do
	renderedObject = GameObjectManager:createGameObject("renderedObject")

	-- render component
	renderedObject.render = renderedObject:createRenderComponent()
	renderedObject.render:setPath("data/models/barbarian/barbarian.thModel")
	renderedObject.render:setScale(Vec3(0.5, 0.5, 0.5))

	renderedObject.animation = renderedObject:createAnimationComponent()
	renderedObject.animation:setSkeletonFile("data/animations/Barbarian/Barbarian.hkt")
	renderedObject.animation:setSkinFile("data/animations/Barbarian/Barbarian.hkt")
	renderedObject.animation:addAnimationFile("Idle", "data/animations/Barbarian/Barbarian_Idle.hkt")
	renderedObject.animation:addAnimationFile("Walk", "data/animations/Barbarian/Barbarian_Walk.hkt")
	renderedObject.animation:addAnimationFile("Run", "data/animations/Barbarian/Barbarian_Run.hkt")
	renderedObject.animation:addAnimationFile("Attack", "data/animations/Barbarian/Barbarian_Attack.hkt")

	GameObjectManager:createGameObject("sponza"):createRenderComponent():setPath("data/sponza/sponza.thModel")
end
