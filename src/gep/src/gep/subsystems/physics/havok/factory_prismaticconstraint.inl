#include <Physics/Constraint/Data/Prismatic/hkpPrismaticConstraintData.h>

static hkpConstraintInstance* createPrismatic(ScriptTableWrapper& scriptTable)
{
    auto L = scriptTable.getState();

    auto pA = getRigidBody(scriptTable, "A", false);
    auto pB = getRigidBody(scriptTable, "B", true);

    auto pData = new hkpPrismaticConstraintData();
    extractCommonConstraintData(scriptTable, pData);
    std::string temp;

    // Constraint space
    //////////////////////////////////////////////////////////////////////////
    if(!scriptTable.tryGet("constraintSpace", temp))
    {
        luaL_error(L, "Missing table member 'constraintSpace' (must be either \"world\" or \"body\")");
    }

    if (temp == "world")
    {
        hkVector4 pivot = getVec4(scriptTable, "pivot");
        hkVector4 axis = getVec4(scriptTable, "axis");
        pData->setInWorldSpace(pA->getTransform(), pB->getTransform(), pivot, axis);
    }
    else if(temp == "body")
    {
        hkVector4 pivotA = getVec4(scriptTable, "pivotA");
        hkVector4 pivotB = getVec4(scriptTable, "pivotB");
        hkVector4 axisA = getVec4(scriptTable, "axisA");
        hkVector4 axisB = getVec4(scriptTable, "axisB");
        hkVector4 axisAPerp = getVec4(scriptTable, "axisAPerp");
        hkVector4 axisBPerp = getVec4(scriptTable, "axisBPerp");
        pData->setInBodySpace(pivotA,    pivotB,
                              axisA,     axisB,
                              axisAPerp, axisBPerp);
    }
    else
    {
        luaL_error(L, "Value of constraint cinfo member 'constraintSpace' must be either \"world\" or \"body\" "
                   "(got \"%s\")", temp.c_str());
    }

    bool allowRotation;
    if(scriptTable.tryGet("allowRotation", allowRotation))
    {
        pData->allowRotationAroundAxis(allowRotation);
    }

    hkReal maxLinearLimit;
    if (scriptTable.tryGet("maxLinearLimit", maxLinearLimit))
    {
        pData->setMaxLinearLimit(maxLinearLimit);
    }

    hkReal minLinearLimit;
    if (scriptTable.tryGet("minLinearLimit", minLinearLimit))
    {
        pData->setMinLinearLimit(minLinearLimit);
    }

    hkReal maxFrictionForce;
    if (scriptTable.tryGet("maxFrictionForce", maxFrictionForce))
    {
        pData->setMaxFrictionForce(maxFrictionForce);
    }

    auto pConstraint = new hkpConstraintInstance(pA, pB, pData);
    pData->removeReference();

    return pConstraint;
}
