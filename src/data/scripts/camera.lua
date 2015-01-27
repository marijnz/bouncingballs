logMessage("using camera.lua")

Camera = {}
Camera.__index = Camera

setmetatable(Camera, {
  __index = PoolObject, -- this is what makes the inheritance work
  __call = function (cls, ...)
    self = setmetatable({}, cls)
    return self
  end,
})

function Camera:create()
	camera = GameObjectManager:createGameObject("camera")
	camera.cc = camera:createCameraComponent()
    camera.cc:setOrthographic(true)
    local lookAt = Vec3(-3.0, -3.0, 0.0)
	camera.cc:setPosition(lookAt + Vec3(15.0, 15.0, 15.0))
    camera.cc:lookAt(lookAt)
    camera.cc:setFar(150)
    camera.cc:setNear(10)
	camera:setComponentStates(ComponentState.Active)	
	self.camera = camera
	
	self.isMoving = false
end

function Camera:moveTo(toPosition)
	if(self.isMoving) then
		return
	end
	-- linear lerp for now, boring! ease in ease out better probably
	
	-- from lookout!
	self.fromPosition = self.camera.cc:getPosition() - Vec3(15.0, 15.0, 15.0)
	self.toPosition = toPosition
	self.timeSinceMoving = 0
	self.isMoving = true
end

function Camera:update(deltaTime)
	if(self.isMoving) then
		logMessage("OK")
		self.timeSinceMoving = self.timeSinceMoving + deltaTime
		lookAt = easingVec3( easing.inOutSine, self.timeSinceMoving,self.fromPosition,self.toPosition - self.fromPosition, 1)
		logMessage(lookAt.x)
		self.camera.cc:setPosition(lookAt + Vec3(15.0, 15.0, 15.0))
		self.camera.cc:lookAt(lookAt)
		if(self.timeSinceMoving > 1) then
			self.isMoving = false
		end
	end
end

function Camera:initialize() end
function Camera:dispose() end

