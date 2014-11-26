-- Contains the most important built-in functions and tables.
-- This way the global ones can be overriden and the built-in versions are still available
-- Note! The table 'debug' is used internally in the engine!

__builtins = {
	type = type,
	print = print,
	io = io,
	tostring = tostring,
}
