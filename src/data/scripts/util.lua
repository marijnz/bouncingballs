logMessage("using util.lua")

vec2Angle = function (vec)
    return math.atan2(vec.y, vec.x)
end

vec2Rotate = function (vec, angle)
    theta = toRadians(angle)

    cs = math.cos(theta);
    sn = math.sin(theta);

    return Vec3(vec.x * cs - vec.y * sn, vec.x * sn + vec.y * cs, vec.z)
end
