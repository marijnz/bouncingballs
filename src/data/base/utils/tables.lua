
function isEmpty(tab)
	assert(type(tab) == "table")
	return next(tab) == nil
end

-- checks if there is any table key that was not expected
function checkTableKeys(tab, validKeys)
	invalidKeys = {}
	for key, _ in pairs(tab) do
		local hasKey = false
		for _, validKey in ipairs(validKeys) do
			if key == validKey then hasKey = true end
		end
		if not hasKey then
			table.insert(invalidKeys, key)
		end
	end
	return invalidKeys
end

function isPureArray(array)
	for k,v in pairs(array) do
		if type(k) ~= "number" then return false end
	end
	return true
end
