-- allow custom type flags
-- uses __builtins (which is not a lua standard, its defined in data/base/utils/builtins.lua)

function type(anything)
	local builtinType = __builtins.type(anything)
	if anything and builtinType == "table" and anything.__type then
		return anything.__type
	end
	return builtinType
end
