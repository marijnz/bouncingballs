logMessage("using camera.lua")

do -- camera 
	camera = GameObjectManager:createGameObject("camera")
	camera.cc = camera:createCameraComponent()
    camera.cc:setOrthographic(true)
    local lookAt = Vec3(-3.0, -3.0, 0.0)
	camera.cc:setPosition(lookAt + Vec3(15.0, 15.0, 15.0))
    camera.cc:lookAt(lookAt)
    camera.cc:setFar(150)
    camera.cc:setNear(10)
	camera:setComponentStates(ComponentState.Active)
end
