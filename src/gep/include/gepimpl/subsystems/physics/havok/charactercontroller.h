#pragma once

#include "gep/interfaces/physics/characterController.h"

namespace gep
{
    class HavokRigidBody;

    //////////////////////////////////////////////////////////////////////////

    class HavokCharacterRigidBody : public ICharacterRigidBody
    {
        SmartPtr<HavokRigidBody> m_pRigidBody;

        hkRefPtr<hkpCharacterRigidBody> m_pHkCharacterRigidBody;
        hkRefPtr<hkpCharacterContext> m_pHkCharacterContext;

    public:
        HavokCharacterRigidBody(const CharacterRigidBodyCInfo& cinfo);
        ~HavokCharacterRigidBody();

        virtual void initialize() override;

        virtual void destroy() override;

        virtual void update(const CharacterInput& input, CharacterOutput& output) override;

        virtual void checkSupport(float deltaTime, SurfaceInfo& surfaceinfo) override;

        virtual void setLinearVelocity(const vec3& newVelocity, float deltaTime) override;

        virtual IRigidBody* getRigidBody() override;

        virtual CharacterState::Enum getState() const;

    };
}
