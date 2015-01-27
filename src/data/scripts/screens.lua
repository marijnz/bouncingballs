logMessage("using screens.lua")

--gameOverScreen
gameOverScreen = GameObjectManager:createGameObject("gameOverScreen")
gameOverScreen.render = gameOverScreen:createRenderComponent()
gameOverScreen.render:setPath("data/models/gameOverText.thModel")
gameOverScreen:setPosition(Vec3(0,0,3))
gameOverScreen.render:setState(ComponentState.Inactive)
gameOverScreen.active=false

--pauseScreen
pauseScreen = GameObjectManager:createGameObject("pauseScreen")
pauseScreen.render = pauseScreen:createRenderComponent()
pauseScreen.render:setPath("data/models/pauseScreenText.thModel")
pauseScreen:setPosition(Vec3(-10,-10,3))
pauseScreen.render:setState(ComponentState.Inactive)
pauseScreen.active=false

--gameStartScreen
gameStartScreen = GameObjectManager:createGameObject("gameStartScreen")
gameStartScreen.render = gameStartScreen:createRenderComponent()
gameStartScreen.render:setPath("data/models/gameStartText.thModel")
gameStartScreen:setPosition(Vec3(-10,-10,3))
gameStartScreen.render:setState(ComponentState.Inactive)
gameStartScreen.active=false

function toggleScreen(screen)
	--logMessage("toggling", screen)
	if (screen.active) then
		--logMessage("deactivating",screen)
		screen.render:setState(ComponentState.Inactive)
		screen.active=false
	else
		--logMessage("activating",screen)
		screen.render:setState(ComponentState.Active)
		screen.active=true
	end
end