
-- Table for all global events
Events = {}

-- Global update event
Events.Update = _EventManager:getUpdateEvent()

function Events.create()
	print("Creating generic event")
	return _EventManager:createGenericEvent()
end
