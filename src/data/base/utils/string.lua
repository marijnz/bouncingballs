
function tostring(anything, typeOverride)
	if typeOverride == "vec3" then
		return "{"
			.. tostring(anything.x) .. ", "
			.. tostring(anything.y) .. ", "
			.. tostring(anything.z)
			.. "}"
	end
	return __builtins.tostring(anything)
end
