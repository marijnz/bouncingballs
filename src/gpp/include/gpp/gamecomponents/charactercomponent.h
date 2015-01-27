#pragma once

#include "gpp/gameComponents/physicsComponent.h"

namespace gpp
{
    class CharacterComponent :
        public PhysicsComponent
    {
    public:

        CharacterComponent();
        virtual ~CharacterComponent();

        virtual void initalize() override;
        virtual void update(float elapsedMS) override;
        virtual void destroy() override;

        gep::ICharacterRigidBody* createCharacterRigidBody(const gep::CharacterRigidBodyCInfo& cinfo);
        inline gep::ICharacterRigidBody* getCharacterRigidBody() { return m_pCharacterRigidBody.get(); }

        inline gep::Event<gep::CharacterInput*>* getInputUpdateEvent() { return &m_event_updateCharacterInput; }

        LUA_BIND_REFERENCE_TYPE_BEGIN

            LUA_BIND_FUNCTION(createCharacterRigidBody)
            LUA_BIND_FUNCTION(getInputUpdateEvent)
            LUA_BIND_FUNCTION(getCharacterRigidBody)

            // PhysicsComponent interface
            //LUA_BIND_FUNCTION(createRigidBody)
            LUA_BIND_FUNCTION(getRigidBody)
            LUA_BIND_FUNCTION(getContactPointEvent)

            // Component interface
            LUA_BIND_FUNCTION(setState)
            LUA_BIND_FUNCTION(getState)
         LUA_BIND_REFERENCE_TYPE_END;

    private:
        gep::SmartPtr<gep::ICharacterRigidBody> m_pCharacterRigidBody;
        gep::Event<gep::CharacterInput*> m_event_updateCharacterInput;
    };

    template<>
    struct ComponentMetaInfo<CharacterComponent>
    {
        static const char* name() { return "CharacterComponent"; }
        static const gep::int32 initializationPriority() { return 0; }
        static const gep::int32 updatePriority() { return 20; }
    };

}
