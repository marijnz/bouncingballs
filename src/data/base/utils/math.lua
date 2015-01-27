
function math.sign(number)
	if number < 0 then
		return -1
	elseif number > 0 then
		return 1
	else
		return 0
	end
end

function toRadians(angle)
	return angle * (math.pi / 180.0)
end

function toDegrees(angle)
	return angle * (180.0 / math.pi)
end

-- Redefine Quaternion to support a more Lua-like interface
local _Quaternion = Quaternion
local function quatAxisAngle(axis, angle)
	local quat = _Quaternion()

	angle = toRadians(angle) / 2
	temp = math.sin(angle)
	quat.x = axis.x * temp
	quat.y = axis.y * temp
	quat.z = axis.z * temp
	quat.angle = math.cos(angle)
	quat = quat:normalized()
	return quat
end

local function quatAxisAxis(axis1, axis2)
	local costheta = axis1:dot(axis2)
	local theta = math.acos(costheta)
	local nAxis = axis1:cross(axis2)
	return quatAxisAngle(nAxis, toDegrees(theta))
end

function Quaternion(axis, angle)
	if axis == nil then             return _Quaternion()              end -- Identity quaternion
	if type(angle) == "number" then return quatAxisAngle(axis, angle) end -- Rotation about given axis by given angle
	if type(angle) == "table"  then return quatAxisAxis(axis, angle)  end -- Rotation from one axis to another

	assert(false, "Invalid Input for Quaternion Constructor!")
end

-- Redefine Vec3 to support a more Lua-like interface
local _Vec3 = Vec3
function Vec3(x, y, z)
	local result = nil
	if x == nil then return _Vec3(0, 0, 0) end -- Defualt to zero-vector
	if y == nil then return _Vec3(x, x, x) end -- Initialize all components with given scalar
	do return               _Vec3(x, y, z) end -- Use given x, y, and z values
end

function math.clamp(value, min, max)
	if value < min then return min end
	if value > max then return max end
	do                  return value end
end
