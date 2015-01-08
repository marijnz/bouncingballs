
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
