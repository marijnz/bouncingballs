logMessage("using defaults/stateMachine.lua")

State{
	name = "default",
	parent = "/game/gameRunning",
}

State{
	name = "freeCam",
	parent = "/game/gameRunning"
}

StateTransitions{
	parent = "/game/gameRunning",
	{ from = "__enter", to = "default" },
}
