#pragma once

#include "gep/math3d/vec3.h"
#include "gep/math3d/quaternion.h"

#include "gep/ReferenceCounting.h"
#include "gep/interfaces/scripting.h"

namespace gep
{
    class IShape;
    class IRigidBody;

    struct SurfaceInfo
    {
        struct SupportedState
        {
            enum Enum
            {
                Unsupported = 0,
                Sliding = 1,
                Supported = 2
            };
        };

        SupportedState::Enum supportedState;
        vec3 surfaceNormal;
        vec3 surfaceVelocity;
        float surfaceDistanceExcess;
        bool surfaceIsDynamic;

        SurfaceInfo() :
            supportedState(SupportedState::Supported),
            surfaceNormal(0.0f, 0.0f, 1.0f),
            surfaceVelocity(0.0f),
            surfaceDistanceExcess(0.0f),
            surfaceIsDynamic(false)
        {
        }
    };

    struct CharacterInput
    {
        // User input
        //////////////////////////////////////////////////////////////////////////

        /// Input X range -1 to 1 (left / right)
        float inputLR;

        /// Input Y range -1 to 1 (forward / back)
        float inputUD;

        /// Set this if you want the character to try and jump
        bool wantJump;
        // Orientation information
        //////////////////////////////////////////////////////////////////////////

        /// Up vector in world space - should generally point in the opposite direction to gravity
        vec3 up;
        /// Forward vector in world space - point in the direction the character is facing
        vec3 forward;
        // Spatial info
        //////////////////////////////////////////////////////////////////////////

        /// Set this if the character is at a ladder and you want it to start to climb
        bool atLadder;
        /// The surface information.
        SurfaceInfo surfaceInfo;

        // Simulation info
        //////////////////////////////////////////////////////////////////////////

        /// Set this to the timestep between calls to the state machine
        float deltaTime;

        /// Set this to the current position
        vec3 position;

        /// Set this to the current Velocity
        vec3 velocity;
        /// The gravity that is applied to the character when in the air
        vec3 characterGravity;
        CharacterInput() :
            inputLR(0.0f),
            inputUD(0.0f),
            wantJump(false),
            up(0.0f, 0.0f, 1.0f),
            forward(0.0f),
            atLadder(false),
            surfaceInfo(),
            deltaTime(0.0f),
            position(0.0f),
            velocity(0.0f),
            characterGravity(0.0f)
        {
        }

        // Functions needed for scripting
        //////////////////////////////////////////////////////////////////////////

        inline float getInputLR() { return inputLR; }
        inline void setInputLR(float lr) { inputLR = lr; }

        inline float getInputUD() { return inputUD; }
        inline void setInputUD(float ud) { inputUD = ud; }

        inline vec3 getPosition() { return position; }
        inline void setPosition(const vec3& pos) { position = pos; }

        inline bool getWantJump() const { return wantJump; }
        inline void setWantJump(bool value) { wantJump = value; }

        inline gep::vec3 getUp() const { return up; }
        inline void setUp(gep::vec3 value) { up = value; }

        inline gep::vec3 getForward() const { return forward; }
        inline void setForward(gep::vec3 value) { forward = value; }

        inline bool getAtLadder() const { return atLadder; }
        inline void setAtLadder(bool value) { atLadder = value; }

        inline gep::vec3 getVelocity() const { return velocity; }
        inline void setVelocity(gep::vec3 value) { velocity = value; }

        inline gep::vec3 getCharacterGravity() const { return characterGravity; }
        inline void setCharacterGravity(gep::vec3 value) { characterGravity = value; }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getInputLR)
            LUA_BIND_FUNCTION(setInputLR)
            LUA_BIND_FUNCTION(getInputUD)
            LUA_BIND_FUNCTION(setInputUD)
            LUA_BIND_FUNCTION(getPosition)
            LUA_BIND_FUNCTION(setPosition)
            LUA_BIND_FUNCTION(getWantJump)
            LUA_BIND_FUNCTION(setWantJump)
            LUA_BIND_FUNCTION(getUp)
            LUA_BIND_FUNCTION(setUp)
            LUA_BIND_FUNCTION(getForward)
            LUA_BIND_FUNCTION(setForward)
            LUA_BIND_FUNCTION(getAtLadder)
            LUA_BIND_FUNCTION(setAtLadder)
            LUA_BIND_FUNCTION(getVelocity)
            LUA_BIND_FUNCTION(setVelocity)
            LUA_BIND_FUNCTION(getCharacterGravity)
            LUA_BIND_FUNCTION(setCharacterGravity)
        LUA_BIND_REFERENCE_TYPE_END;
    };

    /// The output from the state machine
    struct CharacterOutput
    {
        /// The output velocity of the character
        vec3 velocity;

        /// Modified input velocity, can be used to affect the velocity filtering performed in hkpCharacterContext::update
        vec3 inputVelocity;

        CharacterOutput() :
            velocity(0.0f),
            inputVelocity(0.0f)
        {
        }
    };

    struct CharacterRigidBodyCInfo
    {

        /// The collision filter info.
        /// See gep::RigidBodyCInfo for details
        uint32 collisionFilterInfo;

        /// The shape.
        /// See gep::RigidBodyCInfo for details
        IShape* shape;

        /// Initial position.
        /// See gep::RigidBodyCInfo for details
        vec3 position;

        /// Initial rotation.
        /// See gep::RigidBodyCInfo for details
        Quaternion rotation;

        /// The mass of character.
        /// See gep::RigidBodyCInfo for details
        float mass;

        /// Set friction of character.
        /// See gep::RigidBodyCInfo for details
        float friction;

        /// Set maximum linear velocity (see gep::RigidBodyCInfo for details). Keep in mind that the maximum linear
        /// velocity is restricted as well by hkpCharacterContext. Use hkpCharacterContext.setFilterParameters
        /// to keep both limits in sync when necessary.
        float maxLinearVelocity;

        /// Set maximal allowed penetration depth.
        /// See gep::RigidBodyCInfo for details
        float allowedPenetrationDepth;


        // Character controller specific values
        //////////////////////////////////////////////////////////////////////////

        /// Set up direction
        vec3 up;

        /// Set maximal slope
        float maxSlope;

        /// Set maximal force of character
        float maxForce;

        /// If the character's shape is a capsule, then this is used to define the height of a region around its center
        /// where we redirect contact point normals. The region extends from above the capsule's upper vertex to below
        /// the lower vertex by this height, expressed as a factor of the capsule's radius.
        float unweldingHeightOffsetFactor;


        // Parameters used by checkSupport
        //////////////////////////////////////////////////////////////////////////

        /// Set maximal speed for simplex solver
        float maxSpeedForSimplexSolver;

        /// A character is considered supported if it is less than this distance above its supporting planes.
        float supportDistance;

        /// A character should keep falling until it is this distance or less from its supporting planes.
        float hardSupportDistance;

        /// Constructor. Sets some defaults.
        CharacterRigidBodyCInfo() :
            mass(100.0f),
            maxForce(1000.0f),
            friction(0.0f),
            maxSlope(GetPi<float>::value() / 3.0f),
            unweldingHeightOffsetFactor(0.5f),
            up(0.0f, 0.0f, 1.0f),
            maxLinearVelocity(20.0f),
            allowedPenetrationDepth(-0.1f),
            maxSpeedForSimplexSolver(10.0f),
            collisionFilterInfo(0),
            position(0.0f),
            rotation(),
            supportDistance(0.1f),
            hardSupportDistance(0.0f),
            shape(nullptr)
        {
        }

        LUA_BIND_VALUE_TYPE_BEGIN
        LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(mass)
            LUA_BIND_MEMBER(maxForce)
            LUA_BIND_MEMBER(friction)
            LUA_BIND_MEMBER(maxSlope)
            LUA_BIND_MEMBER(unweldingHeightOffsetFactor)
            LUA_BIND_MEMBER(up)
            LUA_BIND_MEMBER(maxLinearVelocity)
            LUA_BIND_MEMBER(allowedPenetrationDepth)
            LUA_BIND_MEMBER(maxSpeedForSimplexSolver)
            LUA_BIND_MEMBER(collisionFilterInfo)
            LUA_BIND_MEMBER(position)
            LUA_BIND_MEMBER(rotation)
            LUA_BIND_MEMBER(supportDistance)
            LUA_BIND_MEMBER(hardSupportDistance)
            LUA_BIND_MEMBER(shape)
        LUA_BIND_VALUE_TYPE_END
    };

    struct CharacterState
    {
        enum Enum
        {
            InvalidStateID = -1,

            OnGround = 0,
            Jumping,
            InAir,
            Climbing,
            Flying,

            UserState0,
            UserState1,
            UserState2,
            UserState3,
            UserState4,
            UserState5,

            MaxStateID
        };

        inline static const char* toString(CharacterState::Enum enumValue)
        {
            static_assert(CharacterState::MaxStateID == 11, "Keep this method and the enum value in sync!");

            switch (enumValue)
            {
            case gep::CharacterState::OnGround:   return "OnGround";
            case gep::CharacterState::Jumping:    return "Jumping";
            case gep::CharacterState::InAir:      return "InAir";
            case gep::CharacterState::Climbing:   return "Climbing";
            case gep::CharacterState::Flying:     return "Flying";
            case gep::CharacterState::UserState0: return "UserState0";
            case gep::CharacterState::UserState1: return "UserState1";
            case gep::CharacterState::UserState2: return "UserState2";
            case gep::CharacterState::UserState3: return "UserState3";
            case gep::CharacterState::UserState4: return "UserState4";
            case gep::CharacterState::UserState5: return "UserState5";

            case gep::CharacterState::InvalidStateID: return "<<INVALID>>";
            case gep::CharacterState::MaxStateID:     return "<<INVALID(MaxStateID)>>";
            }

            return "<<UNKNOWN>>";
        }

        GEP_DISALLOW_CONSTRUCTION(CharacterState);
    };

    class ICharacterRigidBody : public ReferenceCounted
    {
    public:
        virtual ~ICharacterRigidBody() {}

        virtual IRigidBody* getRigidBody() = 0;

        virtual void initialize() = 0;
        virtual void destroy() = 0;

        virtual void update(const CharacterInput& input, CharacterOutput& output) = 0;

        /// \brief Checks if the character is supported from below, e.g. stands on the ground.
        virtual void checkSupport(float deltaTime, SurfaceInfo& surfaceinfo) = 0;

        virtual void setLinearVelocity(const vec3& newVelocity, float deltaTime) = 0;

        virtual CharacterState::Enum getState() const = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getRigidBody)
            LUA_BIND_FUNCTION(getState)
        LUA_BIND_REFERENCE_TYPE_END
    };
}
