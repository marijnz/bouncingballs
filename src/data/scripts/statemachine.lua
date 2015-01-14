logMessage("using defaults/stateMachine.lua")

State{
	name = "default",
	parent = "/game",
}

State{
	name = "gameOver",
	parent = "/game",
	
	eventListeners = {
		enter = {
			function ()
				player.sc:setState(ComponentState.Inactive)
				for k, v in pairs(balls) do				
					v:freeze()
				end	
			end
		},
		leave = {
			function ()
				player.sc:setState(ComponentState.Active)
			end
		},
    }
}

StateTransitions{
	parent = "/game",
	{ from = "__enter", to = "default" },
	{ from = "default", to = "__leave", condition = function() return InputHandler:wasTriggered(Key.Escape) end },
	{ from = "gameOver", to = "__leave", condition = function() return InputHandler:wasTriggered(Key.Escape) end },
	{ from = "default", to = "gameOver", condition = function() return gameOverBool end },
	
}
