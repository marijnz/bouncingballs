local function extractParentStateMachine(cinfo)
	local parent = cinfo.parent
	cinfo.parent = nil
	if type(parent) == "string" then
		return Game:getStateMachineFactory():get(parent)
	end
	return parent
end

local function checkName(cinfo)
	assert(cinfo.name:find("/") == nil, "Your state name may not contain forward slashes '/'! name = " .. cinfo.name)
end

local function printInvalidKeys(invalidKeys, name)
	local message = "Detected invalid keys in cinfo for '" .. name .. "':"
	for _, invalidKey in ipairs(invalidKeys) do
		message = message ..  "\n\t" .. invalidKey
	end
	logWarning(message)
end

-- used by the state and state machine creator function
local function setAllEventListeners(instance, cinfo)
	-- if no listeners are specified, return
	if not cinfo.eventListeners then return end

	local event = nil
	local eventNames = {
		{ name = "enter", getter = "getEnterEvent" },
		{ name = "leave", getter = "getLeaveEvent" },
		{ name = "update", getter = "getUpdateEvent" },
	}

	for _, eventName in ipairs(eventNames) do
		local listeners = cinfo.eventListeners[eventName.name]
		if listeners then
			event = instance[eventName.getter](instance)
			for _, listener in ipairs(listeners) do
				assert(listener, "The listener must not be nil!")
				assert(type(listener) == "function", "A listener must be a function!")
				event:registerListener(listener)
			end
		end
	end
end

-- used by the state machine creator function
local function createAllStates(instance, cinfo)
	-- if there are no states, return
	if not cinfo.states then
		return
	end

	for _, stateCInfo in ipairs(cinfo.states) do
		-- Creates the state instance and adds it to the state machine
		if stateCInfo.parent ~= nil then
			logWarning("Inner states should not have a parent set! Ignoring parent.")
		end
		stateCInfo.parent = instance
		State(stateCInfo)
	end
end

-- used by the state machine creator function
local function createAllStateMachines(instance, cinfo)
	-- if there are no stateMachines, return
	if not cinfo.stateMachines then
		return
	end

	for _, stateMachineCInfo in ipairs(cinfo.stateMachines) do
		-- Creates the stateMachine instance and adds it to the state machine
		if stateMachineCInfo.parent ~= nil then
			logWarning("Inner stateMachines should not have a parent set! Ignoring parent.")
		end
		stateMachineCInfo.parent = instance
		StateMachine(stateMachineCInfo)
	end
end

State = {
	validKeys = { "name",
				  "parent",
				  "eventListeners",
				}
}
setmetatable(State, State)
function State:__call(cinfo)
	checkName(cinfo)
	local invalidKeys = checkTableKeys(cinfo, self.validKeys)

	assert(cinfo.parent, "A State MUST have a parent state machine!")
	local parent = extractParentStateMachine(cinfo)
	local instance = parent:createState(cinfo.name)
	instance.__type = "state"

	if not isEmpty(invalidKeys) then
		printInvalidKeys(invalidKeys, instance:getQualifiedName())
	end

	setAllEventListeners(instance, cinfo)

	return instance
end

-- state machine creation helper
StateMachine = {
	validKeys = { "name",
				  "parent",
				  "eventListeners",
				  "states",
				  "stateMachines",
				  "transitions",
				}
}
setmetatable(StateMachine, StateMachine)
function StateMachine:__call(cinfo)
	checkName(cinfo)
	local invalidKeys = checkTableKeys(cinfo, self.validKeys)

	local parent = extractParentStateMachine(cinfo)
	local instance = nil
	if parent then
		instance = parent:createStateMachine(cinfo.name)
	else
		instance = Game:getStateMachineFactory():create(cinfo.name)
	end
	instance.__type = "stateMachine"

	if not isEmpty(invalidKeys) then
		printInvalidKeys(invalidKeys, instance:getQualifiedName())
	end

	setAllEventListeners(instance, cinfo)
	if cinfo.states or cinfo.stateMachines then
		createAllStates(instance, cinfo)
		createAllStateMachines(instance, cinfo)
	elseif not cinfo.states and not cinfo.states then
		logWarning("No inner states or state machines in the state machine '" .. instance:getQualifiedName() .. "'")
	end

	-- create transitions
	local transitions = assert(cinfo.transitions, "A state machine needs to have transitions!")
	transitions.parent = instance
	StateTransitions(transitions)

	return instance
end

-- state transition creation helper
StateTransitions = {}
setmetatable(StateTransitions, StateTransitions)
function StateTransitions:__call(cinfo)
	assert(cinfo.parent, "StateTransitions MUST have a parent")
	local parent = extractParentStateMachine(cinfo)

	if not isPureArray(cinfo) then
		logWarning("The transition cinfo is not a pure array! It should not contain any explicit keys.")
	end

	for _,transition in ipairs(cinfo) do
		-- If there is no condition, make one that always returns true
		local condition = transition.condition or function() return true end
		parent:addTransition(transition.from,
							   transition.to,
							   condition)
	end
end
