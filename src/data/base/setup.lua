local utilsDir = Scripting:getImportantScriptsRoot() .. "utils/"

-- executes important scripts that live in the data/base/utils/ dir
-- Note: this function is local to this script!
local function includeUtilLibrary(utilsFile)
	local scriptFile = utilsDir .. utilsFile
	dofile(scriptFile)
end

includeUtilLibrary("builtins.lua")
includeUtilLibrary("type.lua")
--includeUtilLibrary("debugging.lua")
includeUtilLibrary("tables.lua")
includeUtilLibrary("logging.lua")
includeUtilLibrary("math.lua")
includeUtilLibrary("string.lua")
includeUtilLibrary("scripting.lua")
includeUtilLibrary("events.lua")
includeUtilLibrary("state-machines.lua")
includeUtilLibrary("physics.lua")
