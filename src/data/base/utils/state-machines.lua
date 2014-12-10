-- Custom logging
local _logMessage = logMessage
local _logWarning = logWarning
local _logError = logError

disabledLogMessages = {}

function disableStateMachineLogMessage(id)
	table.append(disabledLogMessages, id)
end

local function isLoggingEnabledForId(id)
	for _, disabledId in ipairs(disabledLogMessages) do
		if id == disabledId then
			return false
		end
	end

	return true
end

local function logMessage(id, message)
	if isLoggingEnabledForId(id) then _logMessage("[id:" .. tostring(id) .. "] " .. tostring(message)) end
end

local function logWarning(id, message)
	if isLoggingEnabledForId(id) then _logWarning("[id:" .. tostring(id) .. "] " .. tostring(message)) end
end

local function logError(id, message)
	if isLoggingEnabledForId(id) then _logError("[id:" .. tostring(id) .. "] " .. tostring(message)) end
end

-- Gets a state machine from cinfo.parent by its fully qualified path.
-- If cinfo.parent is not a string, it is assumed
-- that it already is a state machine
local function getParentInstance(cinfo)
	if type(cinfo.parent) == "string" then
		return Game:getStateMachineFactory():get(cinfo.parent)
	else
		return cinfo.parent
	end
end

-- Assures the validity of cinfo.name
local function checkName(cinfo)
	assert(cinfo.name:find("/") == nil, "Your state name may not contain forward slashes '/'! name = " .. cinfo.name)
end

-- Prints the keys in the given table
local function printInvalidKeys(invalidKeys, name)
	local message = "Detected invalid keys in cinfo for '" .. name .. "':"
	for _, invalidKey in ipairs(invalidKeys) do
		message = message ..  "\n\t" .. invalidKey
	end
	logWarning(0, message)
end

-- Gets the actual events of 'instance' and registers the given event listeners.
-- Used by the State and StateMachine functors
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
			event = instance[eventName.getter](instance) -- Is equivalent to instance:getEnterEvent() if eventName.getter == "getEnterEvent"
			if #listeners == 0 then
				logWarning(6, "Found empty event listener list '" .. tostring(eventName.name) .. "' for " .. cinfo.qualifiedName)
			end
			for _, listener in ipairs(listeners) do
				assert(listener, "The listener must not be nil!")
				assert(type(listener) == "function", "A listener must be a function!")
				event:registerListener(listener)
			end
		end
	end
end

-- Creates states from the cinfo using 'instance', which is expected to be a state machine
-- Used only by the StateMachine functor
local function createAllStates(instance, cinfo)
	-- if there are no states, return
	if not cinfo.states then
		return
	end

	for _, stateCInfo in ipairs(cinfo.states) do
		-- Creates the state instance and adds it to the state machine
		if stateCInfo.parent then
			logWarning(1, "Inner states should not have a parent set! Ignoring parent.")
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
			logWarning(2, "Inner stateMachines should not have a parent set! Ignoring parent.")
		end
		stateMachineCInfo.parent = instance
		StateMachine(stateMachineCInfo)
	end
end

-- State creation helper
-- Check the documentation for details
State = {
	validKeys = { "name",           -- The (local) name of the state you want to define
				  "parent",         -- The fully qualified name of the parent state machine
				  "eventListeners", -- A table which contains event listeners for enter, update, and leave
				}
}
setmetatable(State, State)
function State:__call(cinfo)
	checkName(cinfo)
	local invalidKeys = checkTableKeys(cinfo, self.validKeys)

	assert(cinfo.parent, "A State MUST have a parent state machine!")
	local parent = getParentInstance(cinfo)
	cinfo.parent = nil

	-- Create the actual state instance
	local instance = parent:createState(cinfo.name)

	-- Set our type to 'state'
	instance.__type = "state"

	-- Save our qualified name in the cinfo for error messages and warnings
	cinfo.qualifiedName = instance:getQualifiedName()

	-- If there are invalid keys, print their names
	if not isEmpty(invalidKeys) then
		printInvalidKeys(invalidKeys, instance:getQualifiedName())
	end

	-- Register all specified event listeners
	setAllEventListeners(instance, cinfo)

	return instance
end

-- State machine creation helper
-- Check the documentation for details
StateMachine = {
	validKeys = { "name",           -- The (local) name of the state you want to define
				  "parent",         -- The fully qualified name of the parent state machine
				  "eventListeners", -- A table which contains event listeners for enter, update, and leave
				  "states",         -- A table which contains the state cinfos
				  "stateMachines",  -- Like 'states', but for state machine instances instead of plain states
				  "transitions",    -- A list of tables defined as { from = "...", to = "...", condition = funcThatReturnsBool }
				}
}
setmetatable(StateMachine, StateMachine)
function StateMachine:__call(cinfo)
	checkName(cinfo)
	local invalidKeys = checkTableKeys(cinfo, self.validKeys)

	local parent = getParentInstance(cinfo)
	cinfo.parent = nil

	local instance = nil
	if parent then
		instance = parent:createStateMachine(cinfo.name)
	else
		instance = Game:getStateMachineFactory():create(cinfo.name)
	end
	instance.__type = "stateMachine"

	-- Save our qualified name in the cinfo for error messages and warnings
	cinfo.qualifiedName = instance:getQualifiedName()

	if not isEmpty(invalidKeys) then
		printInvalidKeys(invalidKeys, instance:getQualifiedName())
	end

	setAllEventListeners(instance, cinfo)
	if cinfo.states or cinfo.stateMachines then
		createAllStates(instance, cinfo)
		createAllStateMachines(instance, cinfo)
	elseif not cinfo.states and not cinfo.stateMachines then
		logWarning(3, "No inner states or state machines in the state machine '" .. instance:getQualifiedName() .. "'")
	end

	-- create transitions if there are any
	if cinfo.transitions then
		local transitions = cinfo.transitions
		transitions.parent = instance
		StateTransitions(transitions)
	else
		logWarning(4, "No state transitions in state machine definition")
	end

	return instance
end

-- State transition creation helper
-- Check the documentation for details
StateTransitions = {}
setmetatable(StateTransitions, StateTransitions)
function StateTransitions:__call(cinfo)
	assert(cinfo.parent, "StateTransitions MUST have a parent")
	local parent = getParentInstance(cinfo)
	cinfo.parent = nil

	if not isPureArray(cinfo) then
		logWarning(5, "The transition cinfo is not a pure array! It should not contain any explicit keys (except for the parent).")
	end

	for _,transition in ipairs(cinfo) do
		-- If there is no condition, make one that always returns true
		local condition = transition.condition or function() return true end
		parent:addTransition(transition.from,
							   transition.to,
							   condition)
	end
end
