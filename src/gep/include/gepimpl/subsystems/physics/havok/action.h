#pragma once
#include "gepimpl/havok/util.h"

namespace gep
{
    class HavokRigidBodySyncAction : public hkpUnaryAction
    {
    public:

        HavokRigidBodySyncAction(){}
        virtual ~HavokRigidBodySyncAction(){}

        virtual void applyAction(const hkStepInfo& stepInfo);

        virtual hkpAction* clone(const hkArray<hkpEntity*>& newEntities, const hkArray<hkpPhantom*>& newPhantoms) const;

    protected:
    private:
    };
}
