logMessage("using defaults/stateMachine.lua")
include("stateMachineHelper.lua")
include("screens.lua")

State{
	name = "default",
	parent = "/game",
}

State{
	name = "restart",
	parent = "/game",
	eventListeners = {
		enter = {
		function()
			levelManager:loadLevel(0)
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
	{ from = "__enter", to = "default" },
	{ from = "default", to = "__leave", condition = function() return InputHandler:wasTriggered(Key.Escape) end },
	{ from = "default", to = "pause", condition = function() return InputHandler:wasTriggered(Key.P) end },
	{ from = "pause", to = "default", condition = function() return InputHandler:wasTriggered(Key.P) end },
	{ from = "gameOver", to = "__leave", condition = function() return InputHandler:wasTriggered(Key.Escape) end },
	{ from = "default", to = "gameOver", condition = function() return gameOverBool end },
	{ from = "gameOver", to = "restart", condition = function() return InputHandler:wasTriggered(Key.R) end },
	{ from = "restart", to = "default" },
	
}
