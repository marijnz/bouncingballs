#pragma once
#include "gep/math3d/vec3.h"
#include "gep/math3d/quaternion.h"
#include "gep/math3d/transform.h"
#include "gep/interfaces/updateFramework.h"

#include <functional>

#include "gep/ReferenceCounting.h"
#include "gep/interfaces/scripting.h"
#include "gep/interfaces/events.h"

namespace gep
{
    class IContactListener;
    class IShape;
    class IRigidBody;

    class IPhysicsEntity : public ReferenceCounted
    {
        friend class IPhysicsFactory;
    public:
        virtual ~IPhysicsEntity() {}

        virtual void initialize() = 0;

        virtual void addContactListener(IContactListener* listener) = 0;
        virtual void removeContactListener(IContactListener* listener) = 0;

        virtual void activate() = 0;
        virtual void requestDeactivation() = 0;
        virtual bool isActive() const = 0;

        virtual void setUserData(ScriptTableWrapper table) = 0;
        virtual ScriptTableWrapper getUserData() = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(setUserData)
            LUA_BIND_FUNCTION(getUserData)
        LUA_BIND_REFERENCE_TYPE_END;
    };

    /// \brief Used for trigger volume events.
    class ITriggerEventArgs
    {
    public:
        struct Type
        {
            enum Enum
            {
                Entered = 1,
                Left = 2,
                EnteredAndLeft = Entered | Left,

                /// Fired for any overlapping body when the trigger body leaves the world or is
                /// deleted.
                TriggerBodyLeft = 4 | Left
            };

            GEP_DISALLOW_CONSTRUCTION(Type);
        };

    public:

        virtual IRigidBody* getRigidBody() = 0;
        virtual Type::Enum getEventType() = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION_PTR(static_cast<IRigidBody*(ITriggerEventArgs::*)()>(&getRigidBody), "getRigidBody")
            LUA_BIND_FUNCTION_PTR(static_cast<Type::Enum(ITriggerEventArgs::*)()>(&getEventType), "getEventType")
        LUA_BIND_REFERENCE_TYPE_END;
    };

    /// \brief Purely static struct that serves as an enum.
    struct MotionType
    {
        enum Enum
        {
            Invalid,
            Dynamic,
            SphereInertia,
            BoxInertia,
            Keyframed,
            Fixed,
            ThinBoxInertia,
            Character,
            MaxId
        };

        GEP_DISALLOW_CONSTRUCTION(MotionType);
    };

    struct RigidBodyCInfo
    {
        uint32 collisionFilterInfo;
        float mass;

        IShape* shape;
        MotionType::Enum motionType;
        float restitution;
        vec3 position;
        Quaternion rotation;
        float friction;
        vec3 linearVelocity;
        vec3 angularVelocity;
        float linearDamping;
        float angularDamping;
        float gravityFactor;
        float rollingFrictionMultiplier;
        float maxLinearVelocity;
        float maxAngularVelocity;
        bool enableDeactivation;
        // ??? solverDeactivation;
        float timeFactor;
        // ??? localFrame;
        // ??? collisionResponse;
        uint16 contactPointCallbackDelay;
        // ??? qualityType;
        int8 autoRemoveLevel;
        uint8 responseModifierFlags;
        int8 numShapeKeysInContactPointProperties;

        bool isTriggerVolume;

        struct SolverDeactivation
        {
            enum Enum
            {
                Invalid, ///<
                Off,     ///< No solver deactivation
                Low,     ///< Very conservative deactivation, typically no visible artifacts.
                Medium,  ///< Normal deactivation, no serious visible artifacts in most cases
                High,    ///< Fast deactivation, visible artifacts
                Max      ///< Very fast deactivation, visible artifacts
            };

            GEP_DISALLOW_CONSTRUCTION(SolverDeactivation);
        };

        inline RigidBodyCInfo() :
            collisionFilterInfo(0U),
            mass(0.0f),
            shape(nullptr),
            motionType(MotionType::Fixed),
            restitution(0.4f),
            position(0.0f),
            rotation(),
            friction(0.5f),
            linearVelocity(0.0f),
            angularVelocity(0.0f),
            linearDamping(0.0f),
            angularDamping(0.0f),
            gravityFactor(1.0f),
            rollingFrictionMultiplier(0.0f),
            maxLinearVelocity(200.0f),
            maxAngularVelocity(200.0f),
            enableDeactivation(true),
            timeFactor(1.0f),
            contactPointCallbackDelay(0xffff),
            autoRemoveLevel(0),
            responseModifierFlags(0),
            numShapeKeysInContactPointProperties(0),
            isTriggerVolume(false)
        {
        }

        LUA_BIND_VALUE_TYPE_BEGIN
        LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(collisionFilterInfo)
            LUA_BIND_MEMBER(mass)
            LUA_BIND_MEMBER(shape)
            LUA_BIND_MEMBER(motionType)
            LUA_BIND_MEMBER(restitution)
            LUA_BIND_MEMBER(position)
            LUA_BIND_MEMBER(rotation)
            LUA_BIND_MEMBER(friction)
            LUA_BIND_MEMBER(linearVelocity)
            LUA_BIND_MEMBER(angularVelocity)
            LUA_BIND_MEMBER(linearDamping)
            LUA_BIND_MEMBER(angularDamping)
            LUA_BIND_MEMBER(gravityFactor)
            LUA_BIND_MEMBER(rollingFrictionMultiplier)
            LUA_BIND_MEMBER(maxLinearVelocity)
            LUA_BIND_MEMBER(maxAngularVelocity)
            LUA_BIND_MEMBER(enableDeactivation)
            LUA_BIND_MEMBER(timeFactor)
            LUA_BIND_MEMBER(contactPointCallbackDelay)
            LUA_BIND_MEMBER(autoRemoveLevel)
            LUA_BIND_MEMBER(responseModifierFlags)
            LUA_BIND_MEMBER(numShapeKeysInContactPointProperties)
            LUA_BIND_MEMBER(isTriggerVolume)
        LUA_BIND_VALUE_TYPE_END
    };

    class IRigidBody : public IPhysicsEntity
    {
    public:
        typedef std::function<void(const IRigidBody*)> PositionChangedCallback;

        virtual ~IRigidBody(){}

        virtual uint32 getCollisionFilterInfo() const = 0;
        virtual void setCollisionFilterInfo(uint32 value) = 0;

        virtual float getMass() const = 0;
        virtual void setMass(float value) = 0;

        virtual MotionType::Enum getMotionType() const = 0;
        virtual void setMotionType(MotionType::Enum value) = 0;

        virtual float getRestitution() const = 0;
        virtual void setRestitution(float value) = 0;

        virtual vec3 getPosition() const = 0;
        virtual void setPosition(const vec3& value) = 0;

        virtual Quaternion getRotation() const = 0;
        virtual void setRotation(const Quaternion& value) = 0;

        virtual void setInitialTransform(const ITransform* transform) = 0;

        virtual float getFriction() const = 0;
        virtual void setFriction(float value) = 0;

        virtual vec3 getLinearVelocity() const = 0;
        virtual void setLinearVelocity(const vec3& value) = 0;

        virtual vec3 getAngularVelocity() const = 0;
        virtual void setAngularVelocity(const vec3 & value) = 0;

        virtual float getLinearDamping() const = 0;
        virtual void setLinearDamping(float value) = 0;

        virtual float getAngularDamping() const = 0;
        virtual void setAngularDamping(float value) = 0;

        virtual float getGravityFactor() const = 0;
        virtual void setGravityFactor(float value) = 0;

        virtual float getRollingFrictionMultiplier() const = 0;
        virtual void setRollingFrictionMultiplier(float value) = 0;

        virtual float getMaxLinearVelocity() const = 0;
        virtual void setMaxLinearVelocity(float value) = 0;

        virtual float getMaxAngularVelocity() const = 0;
        virtual void setMaxAngularVelocity(float value) = 0;

        virtual float getTimeFactor() const = 0;
        virtual void setTimeFactor(float value) = 0;

        virtual uint16 getContactPointCallbackDelay() const = 0;
        virtual void setContactPointCallbackDelay(uint16 value) = 0;

        /// \brief converts this rigid body to a trigger volume.
        virtual void convertToTriggerVolume() = 0;
        virtual bool isTriggerVolume() const = 0;
        /// \brief Returns a pointer to the event object that will trigger
        ///        events related to this rigid body's trigger volume properties.
        /// \remark Will return \c nullptr in case this rigid body is not a trigger volume!
        virtual Event<ITriggerEventArgs*>* getTriggerEvent() = 0;

        virtual CallbackId registerSimulationCallback(PositionChangedCallback callback) = 0;
        virtual void deregisterSimulationCallback(CallbackId id) = 0;

        virtual void applyForce(float deltaSeconds, const vec3& force) = 0;
        virtual void applyForceAt(float deltaSeconds, const vec3& force, const vec3& point)  = 0;
        virtual void applyTorque(float deltaSeconds, const vec3& torque)  = 0;

        virtual void applyLinearImpulse(const vec3& impulse)  = 0;
        virtual void applyAngularImpulse(const vec3& impulse)  = 0;
        virtual void applyPointImpulse(const vec3& impulse, const vec3& point)  = 0;

        virtual void reset() = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN

            LUA_BIND_FUNCTION(getCollisionFilterInfo)
            LUA_BIND_FUNCTION(setCollisionFilterInfo)

            LUA_BIND_FUNCTION(getLinearVelocity)
            LUA_BIND_FUNCTION(setLinearVelocity)
            LUA_BIND_FUNCTION(getAngularVelocity)
            LUA_BIND_FUNCTION(setAngularVelocity)
            LUA_BIND_FUNCTION(getPosition)
            LUA_BIND_FUNCTION(setPosition)
            LUA_BIND_FUNCTION(getRotation)
            LUA_BIND_FUNCTION(setRotation)
            LUA_BIND_FUNCTION(getTimeFactor)
            LUA_BIND_FUNCTION(setTimeFactor)
            LUA_BIND_FUNCTION(getMaxLinearVelocity)
            LUA_BIND_FUNCTION(setMaxLinearVelocity)
            LUA_BIND_FUNCTION(getMaxAngularVelocity)
            LUA_BIND_FUNCTION(setMaxAngularVelocity)
            LUA_BIND_FUNCTION(getFriction)
            LUA_BIND_FUNCTION(setFriction)
            LUA_BIND_FUNCTION(getRestitution)
            LUA_BIND_FUNCTION(setRestitution)
            LUA_BIND_FUNCTION(getMotionType)
            LUA_BIND_FUNCTION(setMotionType)
            LUA_BIND_FUNCTION(getMass)
            LUA_BIND_FUNCTION(setMass)

            LUA_BIND_FUNCTION(convertToTriggerVolume)
            LUA_BIND_FUNCTION(isTriggerVolume)
            LUA_BIND_FUNCTION(getTriggerEvent)

            LUA_BIND_FUNCTION(applyForce)
            LUA_BIND_FUNCTION(applyForceAt)
            LUA_BIND_FUNCTION(applyTorque)

            LUA_BIND_FUNCTION(applyLinearImpulse)
            LUA_BIND_FUNCTION(applyAngularImpulse)
            LUA_BIND_FUNCTION(applyPointImpulse)


            // From IPhysicsEntity
            LUA_BIND_FUNCTION(activate)
            LUA_BIND_FUNCTION(requestDeactivation)
            LUA_BIND_FUNCTION(isActive)

            LUA_BIND_FUNCTION(reset)

            LUA_BIND_FUNCTION(setUserData)
            LUA_BIND_FUNCTION(getUserData)

        LUA_BIND_REFERENCE_TYPE_END;
    };

    //TODO: Needs more wrapping!
    class ICollidable
    {
    public:
        virtual ~ICollidable(){}

        virtual void setOwner(IPhysicsEntity* owner) = 0;
        virtual IPhysicsEntity* getOwner() = 0;
        virtual const IPhysicsEntity* getOwner() const = 0;

        virtual void setShape(IShape* shape) = 0;
        virtual IShape* getShape() = 0;
        virtual const IShape* getShape() const = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
        LUA_BIND_REFERENCE_TYPE_END;
    };
}
