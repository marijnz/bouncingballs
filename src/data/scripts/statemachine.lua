logMessage("using defaults/stateMachine.lua")
include("stateMachineHelper.lua")
include("screens.lua")

State{
	name = "default",
	parent = "/game",
}

State{
	name = "loading",
	parent = "/game",
	eventListeners = {
		enter = {
			function()
			toggleScreen(gameStartScreen)
			end
		},
		leave = {
			function()
				toggleScreen(gameStartScreen)
			end
		}
	}
}

State{
	name = "restartGame",
	parent = "/game",
	eventListeners = {
		enter = {
		function()
			disposeEverything()
			levelManager:setCurrentLevelId(0)
			levelManager:goNextLevel()
			player:reset()
		end
		}
	}
}

State{
	name = "restartLevel",
	parent = "/game",
	eventListeners = {
		enter = {
		function()
			disposeEverything()
			levelManager:setCurrentLevelId(levelManager:getCurrentLevelId() - 1)
			levelManager:goNextLevel()
			player:reset()
		end
		}
	}
}


State{
	name = "gameOver",
	parent = "/game",
	
	eventListeners = {
		enter = {
			function()
				toggleScreen(gameOverScreen)
				freezeEverything()
			end
		},
		leave = {
			function()
				toggleScreen(gameOverScreen)
				disposeEverything()
			end
		},
		update = {
			--gameOverUpdate
		}
    }
}

State{
	name = "pause",
	parent = "/game",
	
	eventListeners = {
		enter = {
			function()
				freezeEverything()
				toggleScreen(pauseScreen)
			end
		},
		leave = {
			function()
				unfreezeEverything()
				toggleScreen(pauseScreen)
			end
		},
    }
}

StateTransitions{
	parent = "/game",
	{ from = "__enter", to = "loading" },
	{ from = "loading", to = "default", condition = function() return gameLoadedBool end },
	{ from = "default", to = "__leave", condition = function() return InputHandler:wasTriggered(Key.Escape) end },
	{ from = "default", to = "pause", condition = function() return InputHandler:wasTriggered(Key.P) end },
	{ from = "pause", to = "default", condition = function() return InputHandler:wasTriggered(Key.P) end },
	{ from = "gameOver", to = "__leave", condition = function() return InputHandler:wasTriggered(Key.Escape) end },
	{ from = "default", to = "gameOver", condition = function() return gameOverBool end },
	{ from = "gameOver", to = "restartGame", condition = function() return InputHandler:wasTriggered(Key.R) end },
	{ from = "default", to = "restartGame", condition = function() return InputHandler:wasTriggered(Key.R) end },
	{ from = "gameOver", to = "restartLevel", condition = function() return InputHandler:wasTriggered(Key.L) end },
	{ from = "default", to = "restartLevel", condition = function() return InputHandler:wasTriggered(Key.L) end },
	{ from = "restartLevel", to = "default" },
	{ from = "restartGame", to = "default" }	
}
