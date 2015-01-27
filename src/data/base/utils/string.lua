
function tostring(anything, typeOverride)
	if typeOverride == nil then
		return __builtins.tostring(anything)
	end

	if typeOverride == "vec3" then
		return "{"
			.. tostring(anything.x) .. ", "
			.. tostring(anything.y) .. ", "
			.. tostring(anything.z)
			.. "}"
	end
end

function string.startsWith(str, start)
	return string.sub(str, 1, string.len(start)) == start
end

function string.endsWith(str, theEnd)
	return theEnd == "" or string.sub(str, -string.len(theEnd)) == theEnd
end
