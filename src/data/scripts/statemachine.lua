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
	name = "nextLevel",
	parent = "/game",
	eventListeners = {
		enter = {
			disposeEverything
		}
	}
}


State{
	name = "gameOver",
	parent = "/game",
	
	eventListeners = {
		enter = {
			function()
				local gamepad = InputHandler:gamepad(0)
				gamepad:rumbleLeftFor(10,0.0005)
				gamepad:rumbleRightFor(10, 0.0005)
				toggleScreen(gameOverScreen)
				freezeEverything()
			end
		},
		leave = {
			function()
				toggleScreen(gameOverScreen)
				--disposeEverything()
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
	{ from = "default", to = "pause", condition = function() return InputHandler:wasTriggered(Key.P) or bit32.btest(InputHandler:gamepad(0):buttonsTriggered(), Button.Start) end },
	{ from = "pause", to = "default", condition = function() return InputHandler:wasTriggered(Key.P) or bit32.btest(InputHandler:gamepad(0):buttonsTriggered(), Button.Start) end },
	{ from = "gameOver", to = "__leave", condition = function() return InputHandler:wasTriggered(Key.Escape) end },
	{ from = "default", to = "gameOver", condition = function() return gameOverBool end },
	{ from = "gameOver", to = "restartGame", condition = function() return InputHandler:wasTriggered(Key.R) or bit32.btest(InputHandler:gamepad(0):buttonsTriggered(), Button.Back) end },
	{ from = "default", to = "restartGame", condition = function() return InputHandler:wasTriggered(Key.R) or bit32.btest(InputHandler:gamepad(0):buttonsTriggered(), Button.Back) end },
	{ from = "gameOver", to = "restartLevel", condition = function() return InputHandler:wasTriggered(Key.L) or bit32.btest(InputHandler:gamepad(0):buttonsTriggered(), Button.Y) end },
	{ from = "default", to = "restartLevel", condition = function() return InputHandler:wasTriggered(Key.L) or bit32.btest(InputHandler:gamepad(0):buttonsTriggered(), Button.Y) end },
	{ from = "default", to = "nextLevel", condition = function() return InputHandler:wasTriggered(Key.N) end },
	{ from = "nextLevel", to = "default" },
	{ from = "restartLevel", to = "default" },
	{ from = "restartGame", to = "default" }	
}
