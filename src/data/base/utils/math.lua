
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
	return angle * ( math.pi / 180.0)
end

function toDegrees(angle)
	return angle * (180.0 / math.pi)
end

local function quatAxisAngle(axis, angle)
	local quat = _Quaternion()
		
	angle = toRadians(angle) /2
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
	
	if axis == nil then
		return _Quaternion()
	elseif type(angle) == "number" then
		return quatAxisAngle(axis, angle)
	elseif type(angle) == "table" then
		return quatAxisAxis(axis, angle)
	else
		assert(false, "Invalid Input for Quaternion Constructor!")
	end

end

function math.clamp(value, min, max)

	if value < min then
		return min
	end

	if value > max then
		return max
	end
	return value
end