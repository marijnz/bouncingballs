
-- create enemyFSM
--enemy.fsm = Game:getStateMachineFactory():create("enemyFSM")
enemy.fsm = StateMachine{ name = "enemyFSM" }

-- decide events
function decideLeave(leaveEventData)
	local decision = math.random(0, 1)
	if (decision == 0) then
		leaveEventData:setNextStateByName("idle")
	elseif (decision == 1) then
		leaveEventData:setNextStateByName("patrol")
	end
	enemy.stateTimer = 3
	return EventResult.Handled
end

-- idle events
function idleUpdate(updateEventData)
	enemy.rb:setLinearVelocity(Vec3(0, 0, 0))
	enemy.rb:setAngularVelocity(Vec3(2, 0, 0))
	enemy.stateTimer = enemy.stateTimer - updateEventData:getElapsedTime()
	return EventResult.Handled
end

-- patrol events
function patrolUpdate(updateEventData)
	local pos = enemy.go:getPosition()
	if (pos.y < -50) then
		enemy.moveLeft = false
	elseif (pos.y > 50) then
		enemy.moveLeft = true
	end
	if (enemy.moveLeft) then
		enemy.rb:setLinearVelocity(Vec3(0, -10, 0))
	else
		enemy.rb:setLinearVelocity(Vec3(0, 10, 0))
	end
	enemy.rb:setAngularVelocity(Vec3(0, 0, 0))
	enemy.stateTimer = enemy.stateTimer - updateEventData:getElapsedTime()
	return EventResult.Handled
end

-- enemy active state machine {decide, idle, patrol}
StateMachine{
	name = "active",
	parent = "/enemyFSM",
	states = {
		{
			name = "decide",
			eventListeners = {
				leave = { decideLeave },
			}
		},
		{
			name = "idle",
			eventListeners = {
				update = { idleUpdate },
			}
		},
		{
			name = "patrol",
			eventListeners = {
				update = { patrolUpdate },
			}
		},
	},
	transitions = {
		{ from = "__enter", to = "decide" },
		{ from = "decide", to = "decide" },
		{ from = "decide", to = "idle", condition = function() return false end },
		{ from = "decide", to = "patrol", condition = function() return false end },
		{ from = "idle", to = "decide", condition = function() return (enemy.stateTimer <= 0) end },
		{ from = "patrol", to = "decide", condition = function() return (enemy.stateTimer <= 0) end },
	}
}

-- inactive state
State{
	name = "inactive",
	parent = "/enemyFSM",
}

StateTransitions{
	parent = "/enemyFSM",
	{ from = "__enter", to = "active" },
	{ from = "active", to = "inactive", condition = function() return enemy.ph:getState() == ComponentState.Inactive end },
	{ from = "inactive", to = "active", condition = function() return enemy.ph:getState() == ComponentState.Active end },
}

-- run enemyFSM
enemy.fsm:run()
