logMessage("using defaults/stateMachine.lua")

State{
	name = "default",
	parent = "/game",
}

StateTransitions{
	parent = "/game",
	{ from = "__enter", to = "default" },
	{ from = "default", to = "__leave", condition = function() return InputHandler:wasTriggered(Key.Escape) end },
}
