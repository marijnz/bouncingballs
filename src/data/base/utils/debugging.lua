
function debugBreak(message)
	Scripting:_debugBreak(message or "")
end

function DebugRenderer:drawArrow(startPoint, endPoint, color)
	DebugRenderer:_drawArrow(startPoint, endPoint, color or Color(1,1,1,1))
end

function DebugRenderer:drawLine3D(startPoint, endPoint, color)
	DebugRenderer:_drawLine3D(startPoint, endPoint, color or Color(1,1,1,1))
end

function DebugRenderer:drawLine2D(startPoint, endPoint, color)
	DebugRenderer:_drawLine2D(startPoint, endPoint, color or Color(1,1,1,1))
end

function DebugRenderer:printText(position, text, color)
	DebugRenderer:_printText2D(position, text, color or Color(1,1,1,1))
end

function DebugRenderer:printText3D(position, text, color)
	DebugRenderer:_printText3D(position, text, color or Color(1,1,1,1))
end

function DebugRenderer:drawBox(min, max, color)
	DebugRenderer:_drawBox(min, max, color or Color(1,1,1,1))
end

function DebugRenderer:drawLocalAxes(position, rotation, axesScale, colorX, colorY, colorZ)
	DebugRenderer:_drawLocalAxes({
		position = position,
		rotation = rotation,
		axesScale = axesScale,
		colorX = colorX,
		colorY = colorY,
		colorZ = colorZ,
	})
end

function DebugRenderer:drawOrigin()
	local r = Color(1, 0, 0, 1)
	local g = Color(0, 1, 0, 1)
	local b = Color(0, 0, 1, 1)
	DebugRenderer:printText3D(Vec3(0.0, 0.0, -0.2), "Origin")
	DebugRenderer:_drawLocalAxes({
		position = Vec3(0.0, 0.0, 0.0),
		colorX = r,
		colorY = g,
		colorZ = b,
	})

    DebugRenderer:_printText3D(Vec3(1.1, 0.0, 0.0), "X", r);
    DebugRenderer:_printText3D(Vec3(0.0, 1.1, 0.0), "Y", g);
    DebugRenderer:_printText3D(Vec3(0.0, 0.0, 1.1), "Z", b);
end
