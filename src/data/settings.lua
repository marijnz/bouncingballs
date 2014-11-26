
-- The x and y values correspond to the WinAPI #define CW_USEDEFAULT ((int)0x80000000)
-- For (much) more detail, see http://msdn.microsoft.com/en-us/library/windows/desktop/ms632679%28v=vs.85%29.aspx
-- tl;dr: This lets Windows itself decide where to position the window initially.
local defaultWindowPos = Vec2u(2147483648, 2147483648)

-- All settings for the game are described below.
Settings:load{

	-- General settings.
	general = {
		-- The title of the application. Will be used as the window title.
		applicationTitle = "Gameplay Programming",
	},

	scripts = {
		-- The first user-defined script that is executed by the engine.
		-- Unlike the usual scripts, this path is not relative to "data/scripts" but
		-- to the working directory of the application.
		-- Default: "data/base/initialize.lua"
		mainScript = "data/base/initialize.lua",

		-- This script basically just includes all of our "standard library" scripts
		-- Default: "data/base/setup.lua"
		setupScript = "data/base/setup.lua",

		-- The directory where the scripts live that should not be touched by the user.
		-- Note: The path needs to end with a slash/
		-- Default: "data/base/"
		importantScriptsRoot = "data/base/",

		-- The directory where the scripts live that the users will write.
		-- Note: The path needs to end with a slash/
		-- Default: "data/scripts/"
		userScriptsRoot = "data/scripts/",
	},

	-- Settings about video and the renderer
	video = {
		-- Sets the initial position of the render window.
		initialRenderWindowPosition = defaultWindowPos,

		-- Initial resolution of the render window.
		-- Default: 1280, 720
		screenResolution = Vec2u(1280, 720),

		-- Sync with refresh rate
		-- adaptiveVSyncEnabled will be ignored if vsyncEnabled is enabled.
		-- You may see the vsyncEnabled option as "force VSync at all times".
		vsyncEnabled = false,

		-- Will enable/disable VSync according to the threshold below.
		-- If below the threshold, VSync will be disabled, if above threshold, VSync will be turned on.
		adaptiveVSyncEnabled = true,

		-- Default: 60 FPS
		adaptiveVSyncThreshold = 1.0 / 60.0,

		-- Default: 1 FPS
		adaptiveVSyncTolerance = 1.0,

		-- The default color of the background.
		-- Format: rgba - Red, Green, Blue, Alpha -> 0.0 == none, 1.0 full.
		-- Default:       0.0, 0.125, 0.3,  1.0   -> A blueish color.
		clearColor = Color(0.0, 0.125, 0.3, 1.0),
	},

	-- Settings about the behavior of the scripting system.
	lua = {
		maxStackDumpLevel = 2,
		callstackTracebackEnabled = true,
		stackDumpEnabled = true,
	}
}
