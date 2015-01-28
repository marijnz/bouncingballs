logMessage("using screens.lua")

--gameOverScreen
gameOverScreen = GameObjectManager:createGameObject("gameOverScreen")
gameOverScreen.render = gameOverScreen:createRenderComponent()
gameOverScreen.render:setPath("data/models/game-over-sign.thModel")
gameOverScreen:setPosition(Vec3(6, 6, 9))
gameOverScreen.render:setState(ComponentState.Inactive)
gameOverScreen.active = false

--pauseScreen
pauseScreen = GameObjectManager:createGameObject("pauseScreen")
pauseScreen.render = pauseScreen:createRenderComponent()
pauseScreen.render:setPath("data/models/game-paused-sign.thModel")
pauseScreen:setPosition(Vec3(6, 6, 9))
pauseScreen.render:setState(ComponentState.Inactive)
pauseScreen.active = false

--gameStartScreen
gameStartScreen = GameObjectManager:createGameObject("gameStartScreen")
gameStartScreen.render = gameStartScreen:createRenderComponent()
gameStartScreen.render:setPath("data/models/game-loading-sign.thModel")
gameStartScreen:setPosition(Vec3(6, 6, 9))
gameStartScreen.render:setState(ComponentState.Inactive)
gameStartScreen.active = false

function toggleScreen(screen)
	if (screen.active) then
		screen.render:setState(ComponentState.Inactive)
		screen.active = false
	else
		screen.render:setState(ComponentState.Active)
		screen.active = true
	end
end
