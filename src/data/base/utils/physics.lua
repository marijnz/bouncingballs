
function TriggerVolume(cinfo)
    if not cinfo.name then
        logError("Cannot create a trigger volume without a name!")
    end

    local go = GameObjectManager:createGameObject(cinfo.name)
    go.pc = go:createPhysicsComponent()

    local rbCInfo = RigidBodyCInfo()
    rbCInfo.isTriggerVolume = true
    rbCInfo.motionType = MotionType.Fixed

    if not cinfo.shape then
        logError("Cannot create trigger volume without a shape!")
    end

    rbCInfo.shape = cinfo.shape

    if cinfo.position then
        rbCInfo.position = cinfo.position
    end

    go.rb = go.pc:createRigidBody(rbCInfo)
    local triggerEvent = go.rb:getTriggerEvent()

    if cinfo.listeners then
        for _,listener in ipairs(cinfo.listeners) do
            triggerEvent:registerListener(listener)
        end
    end

    return go
end

local function checkConstraintType(t)
    assert(t, "Missing constraint cinfo table member 'type' (e.g. ConstraintType.BallAndSocket)")
    local result = t == ConstraintType.BallAndSocket
                or t == ConstraintType.Hinge
                or t == ConstraintType.PointToPlane
                or t == ConstraintType.Prismatic

    assert(result, "Invalid constraint type in constraint cinfo table member 'type'.")
end

-- This function checks for mistakes in what all constraints have in common, such as a valid type and two rigid body components.
function PhysicsFactory:createConstraint(cinfo)
    checkConstraintType(cinfo.type)
    return self:_createConstraint(cinfo)
end
