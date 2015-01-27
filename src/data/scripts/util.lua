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

vec3Multiply = function (vec, n)
    return Vec3(vec.x * n, vec.y * n, vec.z * n)
end

vec3Divide = function (vec, n)
    return Vec3(vec.x / n, vec.y / n, vec.z / n)
end

vec2Length = function (vec)
    return math.sqrt((vec.x*vec.x) + (vec.y*vec.y))
end

vec2Normalize = function (vec)
    local length = vec2Length(vec)
    return vec3Divide(vec, length)
end
