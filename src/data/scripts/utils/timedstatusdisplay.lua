
local status = {
	message = nil,
	time = 0.0,
	screenPosition = Vec2(-0.8, -0.8)
}

-- Create a timed status display
local fsm = StateMachine{
	name = "timedStatusDisplay",
	states ={
		{
			name = "idle",
			eventListeners = {
				enter = {
					function()
						status.message = nil
						return EventResult.Handled
					end
				}
			}
		},
		{
			name = "display",
			eventListeners = {
				enter = { function() if status.time <= 0.0 then status.time = 3.0 end return EventResult.Handled end},
				update = {
					function(args)
						local elapsedTime = args:getElapsedTime()
						DebugRenderer:printText(status.screenPosition, status.message)
						status.time = status.time - elapsedTime
						return EventResult.Handled
					end
				}
			}
		},
	},
	transitions = {
		{ from = "__enter", to = "idle" },
		{ from = "idle", to = "display", condition = function() return status.message ~= nil end },
		{ from = "display", to = "idle", condition = function() return status.time <= 0.0 end },
	}
}

function timedStatusDisplay(message, time, pos)
	status.message = message
	status.time = time or 3.0
	status.screenPosition = pos or status.screenPosition
end

function timedStatusDisplayStart()
	fsm:run()
end

function timedStatusDisplayStop()
	logWarning("Stopping the timed status display is not implemented yet!")
	-- fsm:stop()?
end
