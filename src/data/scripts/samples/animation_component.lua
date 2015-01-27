
print("Initializing animation component test...")

include("utils/freeCamera.lua")
include("utils/physicsWorld.lua")
include("utils/stateMachine.lua")

-- PhysicsDebugView
PhysicsSystem:setDebugDrawingEnabled(true)

walkweight = 1.0
staggerweight = 0.0
refPoseThreshold = 0.1
physicsDebugView = true


-- Barbarian
barbarian = GameObjectManager:createGameObject("barbarian")
do
	-- render component
	barbarian.render = barbarian:createRenderComponent()
	barbarian.render:setPath("data/models/barbarian/barbarian.thModel")
	-- animation component
	barbarian.anim = barbarian:createAnimationComponent()
	barbarian.anim:setSkeletonFile("data/animations/Barbarian/Barbarian.hkt")
	barbarian.anim:setSkinFile("data/animations/Barbarian/Barbarian.hkt")
	barbarian.anim:addAnimationFile("walk","data/animations/Barbarian/Barbarian_Walk.hkt")
	barbarian.anim:addAnimationFile("stagger","data/animations/Barbarian/Barbarian_Stagger.HKT")

	barbarian.anim:setBoneDebugDrawingEnabled(physicsDebugView)


end

-- Havok Girl
havokGirl = GameObjectManager:createGameObject("havokGirl")

do
	havokGirl.anim = havokGirl:createAnimationComponent()
	havokGirl.anim:setSkeletonFile("data/animations/HavokGirl/hkRig_L4101.hkx")
	havokGirl.anim:setSkinFile("data/animations/HavokGirl/hkLowResSkinPSP8Bones_L4101.hkx")
	havokGirl.anim:addAnimationFile("walk","data/animations/HavokGirl/hkWalkLoop_L4101.hkx")
	havokGirl.anim:addAnimationFile("stagger","data/animations/HavokGirl/hkRunTurnLLoop_L4101.hkx")
end

havokGirl.anim:setBoneDebugDrawingEnabled(true)
havokGirl.anim:setDebugDrawingScale(100.0)
havokGirl:setPosition(Vec3(-250, 0, 0))

Events.Update:registerListener(function(elapsedTime)
	local bone = barbarian.anim:getBoneByName("left_attachment_jnt"):getWorldPosition()
	print(bone.x .. "  " .. bone.y .. "	" .. bone.z)
	if (InputHandler:wasTriggered(Key.Up)) then
		walkweight = walkweight + 0.1		
	end
	
	if (InputHandler:wasTriggered(Key.Down)) then
		walkweight = walkweight - 0.1
	end

	if (InputHandler:wasTriggered(Key.Left)) then
		staggerweight = staggerweight - 0.1
		
	end
	
	if (InputHandler:wasTriggered(Key.Right)) then
		staggerweight = staggerweight + 0.1
	end

	if (InputHandler:wasTriggered(Key.Z)) then
		refPoseThreshold = refPoseThreshold - 0.1
	end

	if (InputHandler:wasTriggered(Key.X)) then
		refPoseThreshold = refPoseThreshold + 0.1
	end


	if (InputHandler:wasTriggered(Key.B)) then
		physicsDebugView = not physicsDebugView
		barbarian.anim:setBoneDebugDrawingEnabled(physicsDebugView)
	end

	walkweight = math.clamp(walkweight, 0, 1)
	staggerweight = math.clamp(staggerweight, 0, 1)
	refPoseThreshold = math.clamp(refPoseThreshold, 0, 1)

	DebugRenderer:printText(Vec2(-0.9, 0.8),"Press Up and Down to modify walkweight, Left and Right for staggerweight." )
	DebugRenderer:printText(Vec2(-0.9, 0.75), "Z and X set ReferencePoseWeightThreshold")
	DebugRenderer:printText(Vec2(-0.9, 0.7), "B toggles bone debug drawing.")
	DebugRenderer:printText(Vec2(-0.9, 0.65), "Walk Weight: " .. walkweight)
	DebugRenderer:printText(Vec2(-0.9, 0.6), "Stagger Weight: " .. staggerweight)
	DebugRenderer:printText(Vec2(-0.9, 0.55), "Reference PoseWeight Threshold: " .. refPoseThreshold)

	barbarian.anim:setMasterWeight("walk", walkweight)
	barbarian.anim:setMasterWeight("stagger", staggerweight)
	barbarian.anim:setReferencePoseWeightThreshold(refPoseThreshold)

	havokGirl.anim:setMasterWeight("walk", walkweight)
	havokGirl.anim:setMasterWeight("stagger", staggerweight)
	havokGirl.anim:setReferencePoseWeightThreshold(refPoseThreshold)

	if InputHandler:wasTriggered(Key.E)then
		print(barbarian.anim:easeOut("walk", 0.9))		
	end
	if InputHandler:wasTriggered(Key.R)then
		print(barbarian.anim:easeIn("walk", 0.9))		
	end
	return EventResult.Handled
end)



print("Finished animation component test initialization.")
