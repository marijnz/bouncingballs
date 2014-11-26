
-- For more convenience
function Scripting:loadScript(name, options)
	self:_loadScript(name, options or ScriptLoadOptions.Default)
end

function debugBreak(message)
	Scripting:_debugBreak(message or "[LUA DEBUG BREAK]")
end

function include(oneOrMoreScriptNames)
	function includeByFileName(fileName)
		Scripting:loadScript(fileName)
	end
	if type(oneOrMoreScriptNames) == "string" then
		includeByFileName(oneOrMoreScriptNames)
	else
		assert(type(oneOrMoreScriptNames) == "table")
		for _,scriptName in ipairs(oneOrMoreScriptNames) do
			includeByFileName(scriptName)
		end
	end
end
