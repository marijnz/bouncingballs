#include <Physics/Constraint/Data/Hinge/hkpHingeConstraintData.h>

static hkpConstraintInstance* createHinge(ScriptTableWrapper& scriptTable)
{
    auto L = scriptTable.getState();

    auto pA = getRigidBody(scriptTable, "A", false);
    auto pB = getRigidBody(scriptTable, "B", true);

    auto pData = new hkpHingeConstraintData();
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
        auto pivot = getVec4(scriptTable, "pivot");
        hkVector4 axis = getVec4(scriptTable, "axis");
        pData->setInWorldSpace(pA->getTransform(), pB->getTransform(), pivot, axis);
    }
    else if(temp == "body")
    {
        auto pivotA = getVec4(scriptTable, "pivotA");
        auto pivotB = getVec4(scriptTable, "pivotB");
        auto axisA = getVec4(scriptTable, "axisA");
        auto axisB = getVec4(scriptTable, "axisB");
        pData->setInBodySpace(pivotA, pivotB, axisA, axisB);
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
