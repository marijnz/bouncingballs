
function TriggerVolume(cinfo)
	if not cinfo.name then
		logError("Cannot create a trigger volume without a name!")
	end

	local go = GameObjectManager:createGameObject(cinfo.name)
	go.pc = go:createPhysicsComponent()

	local rbCInfo = RigidBodyCInfo()
	rbCInfo.isTriggerVolume = true
	rbCInfo.motionType = MotionType.Fixed

	if not cinfo.shape then
		logError("Cannot create trigger volume without a shape!")
	end

	rbCInfo.shape = cinfo.shape

	if cinfo.position then
		rbCInfo.position = cinfo.position
	end

	go.rb = go.pc:createRigidBody(rbCInfo)
	local triggerEvent = go.rb:getTriggerEvent()

	if cinfo.listeners then
		for _,listener in ipairs(cinfo.listeners) do
			triggerEvent:registerListener(listener)
		end
	end

	return go
end
