logMessage("using defaults/stateMachine.lua")
include("stateMachineHelper.lua")

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
			freezeEverything
		},
		leave = {
			disposeEverything
		},
		update = {
			gameOverUpdate
		}
    }
}

State{
	name = "pause",
	parent = "/game",
	
	eventListeners = {
		enter = {
			freezeEverything
		},
		leave = {
			unfreezeEverything
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
