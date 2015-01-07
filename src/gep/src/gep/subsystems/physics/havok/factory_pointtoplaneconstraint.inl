#include <Physics/Constraint/Data/PointToPlane/hkpPointToPlaneConstraintData.h>

static hkpConstraintInstance* createPointToPlane(ScriptTableWrapper& scriptTable)
{
    auto L = scriptTable.getState();

    auto pA = getRigidBody(scriptTable, "A", false);
    auto pB = getRigidBody(scriptTable, "B", true);

    auto pData = new hkpPointToPlaneConstraintData();
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
        hkVector4 up = getVec4(scriptTable, "up");
        pData->setInWorldSpace(pA->getTransform(), pB->getTransform(), pivot, up);
    }
    else if(temp == "body")
    {
        hkVector4 pivotA = getVec4(scriptTable, "pivotA");
        hkVector4 pivotB = getVec4(scriptTable, "pivotB");
        hkVector4 up = getVec4(scriptTable, "up");
        pData->setInBodySpace(pivotA, pivotB, up);
    }
    else
    {
        luaL_error(L, "Value of constraint cinfo member 'constraintSpace' must be either \"world\" or \"body\" "
                   "(got \"%s\")", temp.c_str());
    }

    auto pConstraint = new hkpConstraintInstance(pA, pB, pData);
    pData->removeReference();

    return pConstraint;
}
