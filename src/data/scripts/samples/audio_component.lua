logMessage("Initializing camera_prototype/camera_prototype.lua ...")

include("utils/freeCamera.lua")
include("utils/physicsWorld.lua")
include("utils/stateMachine.lua")

freeCam.movementFactor = 10
freeCam.cc:setPosition(Vec3(1, -5, 0))

SoundSystem:loadLibrary(".\\data\\sound\\Master Bank.bank")
SoundSystem:loadLibrary(".\\data\\sound\\Master Bank.bank.strings")
SoundSystem:loadLibrary(".\\data\\sound\\Vehicles.bank")
SoundSystem:loadLibrary(".\\data\\sound\\Surround_Ambience.bank")

soundSource = GameObjectManager:createGameObject("soundSource")
soundSource.rc = soundSource:createRenderComponent()
soundSource.rc:setPath("data/models/ball.thModel")
soundSource.sc = soundSource:createScriptComponent()
soundSource.sc:setUpdateFunction(function(guid, deltaTime)
	deltaTime = math.clamp(deltaTime, 1/60, 1/30)
	
	local rpm = engine:getParameter("RPM")
	if (InputHandler:isPressed(Key.Add)) then
		rpm = rpm + 2500 * deltaTime
	end
	if (InputHandler:isPressed(Key.Subtract)) then
		rpm = rpm - 4000 * deltaTime
	end
	rpm = rpm - 1000 * deltaTime
	rpm = math.clamp(rpm, 2000, 7000)
	engine:setParameter("RPM", rpm)
end)

soundSource.audio = soundSource:createAudioComponent()
engine = soundSource.audio:createSoundInstance("soundSourceEngine","/Vehicles/Car Engine")
engine:setParameter("RPM", 3000)
engine:setParameter("Load", 0.3)
logMessage(engine:getParameter("RPM"))
logMessage(engine:getParameter("Load"))
engine:play()

ambience = soundSource.audio:createSoundInstance("ambient", "/Ambience/Country")
ambience:setParameter("Time", 0.85)
ambience:play()

logMessage("... finished initializing camera_prototype/camera_prototype.lua.")
