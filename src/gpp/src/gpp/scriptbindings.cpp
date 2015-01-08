#include "stdafx.h"
#include "gpp/game.h"

#include "gep/globalManager.h"
#include "gep/cameras.h"

#include "gep/interfaces/scripting.h"
#include "gep/interfaces/physics.h"
#include "gep/interfaces/renderer.h"
#include "gep/interfaces/inputHandler.h"
#include "gep/interfaces/logging.h"
#include "gep/interfaces/animation.h"
#include "gep/interfaces/sound.h"

#include "gep/interfaces/physics/contact.h"
#include "gep/interfaces/physics/characterController.h"
#include "gep/interfaces/physics/entity.h"

#include "gep/math3d/vec2.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/quaternion.h"

#include "gpp/gameComponents/cameraComponent.h"
#include "gpp/gameComponents/physicsComponent.h"
#include "gpp/gameComponents/renderComponent.h"
#include "gpp/gameComponents/scriptComponent.h"
#include "gpp/gameComponents/characterComponent.h"
#include "gpp/gameComponents/animationComponent.h"
#include "gpp/gameComponents/audioComponent.h"


#include "gep/interfaces/events.h"

#include "gpp/stateMachines/state.h"
#include "gpp/stateMachines/stateMachine.h"
#include "gpp/stateMachines/stateMachineFactory.h"

#include <Xinput.h>

void gpp::Game::makeScriptBindings()
{
    g_globalManager.getLogging()->logMessage("Making script bindings...");
    bindEnums();
    bindOther();
}

void gpp::Game::bindOther()
{
    auto* scripting = g_globalManager.getScriptingManager();
    lua::utils::StackChecker check(scripting->getState(), 0);

    // Some basics

    {
        auto L = scripting->getState();
        lua_newtable(L); auto tableIndex = lua_gettop(L);

        // null.__ptr = nullptr
        lua_pushlightuserdata(L, nullptr);
        lua_setfield(L, tableIndex, "__ptr");

        // null.isNull = lua::utils::nullCheck()
        lua_pushcfunction(L, lua::utils::nullCheck);
        lua_setfield(L, tableIndex, "isNull");

        // null.equals = lua::utils::ptrCompare
        lua_pushcfunction(L, lua::utils::ptrCompare);
        lua_setfield(L, tableIndex, "equals");

        lua_setglobal(L, "null");
    }

    // Important types

    scripting->bind<gep::ivec2>("Vec2i");
    scripting->bind<gep::vec2>("Vec2");
    scripting->bind<gep::vec3>("Vec3");
    scripting->bind<gep::Quaternion>("Quaternion");
    scripting->bind<gep::mat3>("Mat3");
    scripting->bind<gep::mat4>("Mat4");
    scripting->bind<gep::ITransform>("Transform");
    scripting->bind<gep::Color>("Color");

    scripting->bind<gpp::Game>("Game", this);



    // State machine related
    //////////////////////////////////////////////////////////////////////////
    scripting->bind<gpp::sm::State>("_State");
    scripting->bind<gpp::sm::EnterEventData>("StateEnterEventData");
    scripting->bind<gpp::sm::EnterEvent_t>("StateEnterEvent");
    scripting->bind<gpp::sm::LeaveEventData>("StateLeaveEventData");
    scripting->bind<gpp::sm::LeaveEvent_t>("StateLeaveEvent");
    scripting->bind<gpp::sm::UpdateEventData>("StateUpdateEventData");
    scripting->bind<gpp::sm::UpdateEvent_t>("StateUpdateEvent");
    scripting->bind<gpp::sm::StateMachine>("_StateMachine");
    scripting->bind<gpp::sm::StateMachineFactory>("StateMachineFactory");

    scripting->bind<gep::WorldCInfo>("WorldCInfo");
    scripting->bind<gep::ICollisionFilter>("CollisionFilter");
    scripting->bind<gep::IWorld>("World");
    scripting->bind<gep::IShape>("Shape");
    scripting->bind<gep::IBoxShape>("BoxShape");
    scripting->bind<gep::ISphereShape>("SphereShape");
    scripting->bind<gep::ICapsuleShape>("SphereShape");
    scripting->bind<gep::IBoundingVolumeShape>("BoundingVolumeShape");
    scripting->bind<gep::IPhantomCallbackShape>("PhantomCallbackShape");
    scripting->bind<gep::RigidBodyCInfo>("RigidBodyCInfo");
    scripting->bind<gep::ITriggerEventArgs>("TriggerEventArgs");
    scripting->bind<gep::IRigidBody>("RigidBody");
    scripting->bind<gpp::PhysicsComponent>("PhysicsComponent");
    scripting->bind<gep::CollisionArgs>("CollisionArgs");
    scripting->bind<gep::ContactPointArgs>("ContactPointArgs");
    scripting->bind<gep::RayCastInput>("RayCastInput");
    scripting->bind<gep::RayCastOutput>("RayCastOutput");

    scripting->bind<gpp::ScriptComponent>("ScriptComponent");
    scripting->bind<gpp::RenderComponent>("RenderComponent");
    scripting->bind<gpp::CameraComponent>("CameraComponent");
    scripting->bind<gpp::AnimationComponent>("AnimationComponent");
    scripting->bind<gpp::CharacterComponent>("CharacterComponent");
    scripting->bind<gpp::AudioComponent>("AudioComponent");
    scripting->bind<gpp::SoundInstanceWrapper>("SoundInstance");

    scripting->bind<gep::ICharacterRigidBody>("CharacterRigidBody");
    scripting->bind<gep::CharacterRigidBodyCInfo>("CharacterRigidBodyCInfo");
    scripting->bind<gep::CharacterInput>("CharacterInput");

    scripting->bind<gpp::GameObject>("GameObject");
    scripting->bind<gpp::GameObjectManager>("GameObjectManager", &g_gameObjectManager);

    scripting->bind<gep::IGamepad>("Gamepad");
    scripting->bind<gep::IInputHandler>("InputHandler", g_globalManager.getInputHandler());
    scripting->bind<gep::IPhysicsSystem>("PhysicsSystem", g_globalManager.getPhysicsSystem());
    scripting->bind<gep::IScriptingManager>("Scripting", g_globalManager.getScriptingManager());
    scripting->bind<gep::IPhysicsFactory>("PhysicsFactory", g_globalManager.getPhysicsSystem()->getPhysicsFactory());
    scripting->bind<gep::ISoundSystem>("SoundSystem", g_globalManager.getSoundSystem());
    scripting->bind<gep::ILogging>("Logging", g_globalManager.getLogging());
    scripting->bind<gep::IRenderer>("Renderer", g_globalManager.getRenderer());


    //Rendering
    scripting->bind<gep::IDebugRenderer>("DebugRenderer", &g_globalManager.getRenderer()->getDebugRenderer() );

    // Events
    scripting->bind<gep::Event<float> >("_UpdateEvent");
    scripting->bind<gep::Event<nullptr_t> >("_PostInitializationEvent");
    scripting->bind<gep::Event<gep::ContactPointArgs*> >("_ContactPointEvent");
    scripting->bind<gep::Event<gep::ScriptTableWrapper> >("_GenericEvent");
    scripting->bind<gep::Event<gep::ITriggerEventArgs*> >("_TriggerEvent");
    scripting->bind<gep::Event<gep::CharacterInput*> >("_CharacterInputEvent");
    scripting->bind<gep::Event<gep::IRigidBody*>>("PhantomCallbackShape");
    scripting->bind<gep::IEventManager >("_EventManager", g_globalManager.getEventManager());

    //Animation
    scripting->bind<gep::IBone>("Bone");
}


void gpp::Game::bindEnums()
{
    auto* scripting = g_globalManager.getScriptingManager();
    lua::utils::StackChecker check(scripting->getState(), 0);

    // Enums
    //////////////////////////////////////////////////////////////////////////
    scripting->bindEnum("TriggerEventType",
                        "Entered",         gep::ITriggerEventArgs::Type::Entered,
                        "Left",            gep::ITriggerEventArgs::Type::Left,
                        "EnteredAndLeft",  gep::ITriggerEventArgs::Type::EnteredAndLeft,
                        "TriggerBodyLeft", gep::ITriggerEventArgs::Type::TriggerBodyLeft,
                        0);
    scripting->bindEnum("ConstraintType",
                        "BallAndSocket", gep::ConstraintType::BallAndSocket,
                        "Hinge", gep::ConstraintType::Hinge,
                        "PointToPlane", gep::ConstraintType::PointToPlane,
                        "Prismatic", gep::ConstraintType::Prismatic,
                        0);
    static_assert(gep::ConstraintType::COUNT == 4, "You forgot to update the enum bindings above.");

    scripting->bindEnum("StateUpdateStepBehavior",
        "Continue",                   gpp::sm::UpdateStepBehavior::Continue,
        "Leave",                      gpp::sm::UpdateStepBehavior::Leave,
        "LeaveWithNoConditionChecks", gpp::sm::UpdateStepBehavior::LeaveWithNoConditionChecks,
        0);

    scripting->bindEnum("EventResult",
        "Handled", gep::EventResult::Handled,
        "Ignored", gep::EventResult::Ignored,
        "Cancel",  gep::EventResult::Cancel,
        0);

    scripting->bindEnum("ComponentState",
        //"Initial",        gpp::IComponent::State::Initial,
        //"PostInitialize", gpp::IComponent::State::PostInitialize,
        "Active",         gpp::IComponent::State::Active,
        "Inactive",       gpp::IComponent::State::Inactive,
        0);

    scripting->bindEnum("CharacterState",
        "OnGround",   gep::CharacterState::OnGround,
        "Jumping",    gep::CharacterState::Jumping,
        "InAir",      gep::CharacterState::InAir,
        "Climbing",   gep::CharacterState::Climbing,
        "Flying",     gep::CharacterState::Flying,
        "UserState0", gep::CharacterState::UserState0,
        "UserState1", gep::CharacterState::UserState1,
        "UserState2", gep::CharacterState::UserState2,
        "UserState3", gep::CharacterState::UserState3,
        "UserState4", gep::CharacterState::UserState4,
        "UserState5", gep::CharacterState::UserState5,
        0);

    scripting->bindEnum("CollisionArgsCallbackSource",
        "A",     gep::CollisionArgs::CallbackSource::A,
        "B",     gep::CollisionArgs::CallbackSource::B,
        "World", gep::CollisionArgs::CallbackSource::World,
        0);

    scripting->bindEnum("MotionType",
        "Invalid",        gep::MotionType::Invalid,
        "Dynamic",        gep::MotionType::Dynamic,
        "SphereInertia",  gep::MotionType::SphereInertia,
        "BoxInertia",     gep::MotionType::BoxInertia,
        "Keyframed",      gep::MotionType::Keyframed,
        "Fixed",          gep::MotionType::Fixed,
        "ThinBoxInertia", gep::MotionType::ThinBoxInertia,
        "Character",      gep::MotionType::Character,
        0);

    scripting->bindEnum("ScriptLoadOptions",
        "Default",           gep::IScriptingManager::LoadOptions::Default,
        "PathIsRelative",    gep::IScriptingManager::LoadOptions::PathIsRelative,
        "PathIsAbsolute",    gep::IScriptingManager::LoadOptions::PathIsAbsolute,
        "IsImportantScript", gep::IScriptingManager::LoadOptions::IsImportantScript,
        0);

    scripting->bindEnum("ShapeType",
        "Sphere",         gep::ShapeType::Sphere,
        "Cylinder",       gep::ShapeType::Cylinder,
        "Triangle",       gep::ShapeType::Triangle,
        "Box",            gep::ShapeType::Box,
        "Capsule",        gep::ShapeType::Capsule,
        "ConvexVertices", gep::ShapeType::ConvexVertices,
        0);

    // Bind the key enum
    scripting->bindEnum("Key",
        "LButton", gep::Key::LButton,
        "RButton", gep::Key::RButton,
        "Cancel", gep::Key::Cancel,
        "MButton", gep::Key::MButton,
        "ButtonX1", gep::Key::ButtonX1,
        "ButtonX2", gep::Key::ButtonX2,
        "Back", gep::Key::Back,
        "Tab", gep::Key::Tab,
        "Clear", gep::Key::Clear,
        "Return", gep::Key::Return,
        "Shift", gep::Key::Shift,
        "Control", gep::Key::Control,
        "Menu", gep::Key::Menu,
        "Pause", gep::Key::Pause,
        "Capital", gep::Key::Capital,
        "Kana", gep::Key::Kana,
        "Hangul", gep::Key::Hangul,
        "Junja", gep::Key::Junja,
        "Final", gep::Key::Final,
        "Hanja", gep::Key::Hanja,
        "Kanji", gep::Key::Kanji,
        "Escape", gep::Key::Escape,
        "Convert", gep::Key::Convert,
        "NonConvert", gep::Key::NonConvert,
        "Accept", gep::Key::Accept,
        "ModeChange", gep::Key::ModeChange,
        "Space", gep::Key::Space,
        "Prior", gep::Key::Prior,
        "Next", gep::Key::Next,
        "End", gep::Key::End,
        "Home", gep::Key::Home,
        "Left", gep::Key::Left,
        "Up", gep::Key::Up,
        "Right", gep::Key::Right,
        "Down", gep::Key::Down,
        "Select", gep::Key::Select,
        "Print", gep::Key::Print,
        "Execute", gep::Key::Execute,
        "Snapshot", gep::Key::Snapshot,
        "Insert", gep::Key::Insert,
        "Del", gep::Key::Del,
        "Help", gep::Key::Help,
        "_0", gep::Key::_0,
        "_1", gep::Key::_1,
        "_2", gep::Key::_2,
        "_3", gep::Key::_3,
        "_4", gep::Key::_4,
        "_5", gep::Key::_5,
        "_6", gep::Key::_6,
        "_7", gep::Key::_7,
        "_8", gep::Key::_8,
        "_9", gep::Key::_9,
        "A", gep::Key::A,
        "B", gep::Key::B,
        "C", gep::Key::C,
        "D", gep::Key::D,
        "E", gep::Key::E,
        "F", gep::Key::F,
        "G", gep::Key::G,
        "H", gep::Key::H,
        "I", gep::Key::I,
        "J", gep::Key::J,
        "K", gep::Key::K,
        "L", gep::Key::L,
        "M", gep::Key::M,
        "N", gep::Key::N,
        "O", gep::Key::O,
        "P", gep::Key::P,
        "Q", gep::Key::Q,
        "R", gep::Key::R,
        "S", gep::Key::S,
        "T", gep::Key::T,
        "U", gep::Key::U,
        "V", gep::Key::V,
        "W", gep::Key::W,
        "X", gep::Key::X,
        "Y", gep::Key::Y,
        "Z", gep::Key::Z,
        "LWin", gep::Key::LWin,
        "RWin", gep::Key::RWin,
        "Apps", gep::Key::Apps,
        "Sleep", gep::Key::Sleep,
        "Numpad0", gep::Key::Numpad0,
        "Numpad1", gep::Key::Numpad1,
        "Numpad2", gep::Key::Numpad2,
        "Numpad3", gep::Key::Numpad3,
        "Numpad4", gep::Key::Numpad4,
        "Numpad5", gep::Key::Numpad5,
        "Numpad6", gep::Key::Numpad6,
        "Numpad7", gep::Key::Numpad7,
        "Numpad8", gep::Key::Numpad8,
        "Numpad9", gep::Key::Numpad9,
        "Multiply", gep::Key::Multiply,
        "Add", gep::Key::Add,
        "Separator", gep::Key::Separator,
        "Subtract", gep::Key::Subtract,
        "Decimal", gep::Key::Decimal,
        "Divide", gep::Key::Divide,
        "F1", gep::Key::F1,
        "F2", gep::Key::F2,
        "F3", gep::Key::F3,
        "F4", gep::Key::F4,
        "F5", gep::Key::F5,
        "F6", gep::Key::F6,
        "F7", gep::Key::F7,
        "F8", gep::Key::F8,
        "F9", gep::Key::F9,
        "F10", gep::Key::F10,
        "F11", gep::Key::F11,
        "F12", gep::Key::F12,
        "F13", gep::Key::F13,
        "F14", gep::Key::F14,
        "F15", gep::Key::F15,
        "F16", gep::Key::F16,
        "F17", gep::Key::F17,
        "F18", gep::Key::F18,
        "F19", gep::Key::F19,
        "F20", gep::Key::F20,
        "F21", gep::Key::F21,
        "F22", gep::Key::F22,
        "F23", gep::Key::F23,
        "F24", gep::Key::F24,
        "NumLock", gep::Key::NumLock,
        "ScrollLock", gep::Key::ScrollLock,
        "LShift", gep::Key::LShift,
        "RShift", gep::Key::RShift,
        "LControl", gep::Key::LControl,
        "RControl", gep::Key::RControl,
        "LMenu", gep::Key::LMenu,
        "RMenu", gep::Key::RMenu,
        "Browser_Back", gep::Key::Browser_Back,
        "Browser_ForwarD", gep::Key::Browser_ForwarD,
        "Browser_RefresH", gep::Key::Browser_RefresH,
        "Browser_Stop", gep::Key::Browser_Stop,
        "Browser_Search", gep::Key::Browser_Search,
        "Browser_Favorites", gep::Key::Browser_Favorites,
        "Browser_Home", gep::Key::Browser_Home,
        "Volume_Mute", gep::Key::Volume_Mute,
        "Volume_Down", gep::Key::Volume_Down,
        "Volume_Up", gep::Key::Volume_Up,
        "Media_NextTrack", gep::Key::Media_NextTrack,
        "Media_PrevTrack", gep::Key::Media_PrevTrack,
        "Media_Stop", gep::Key::Media_Stop,
        "Media_Play_Pause", gep::Key::Media_Play_Pause,
        "Launch_Mail", gep::Key::Launch_Mail,
        "Launch_Media_select", gep::Key::Launch_Media_select,
        "Launch_App1", gep::Key::Launch_App1,
        "Launch_App2", gep::Key::Launch_App2,
        "Oem_1", gep::Key::Oem_1,
        "Oem_Plus", gep::Key::Oem_Plus,
        "Oem_Comma", gep::Key::Oem_Comma,
        "Oem_Minus", gep::Key::Oem_Minus,
        "Oem_Period", gep::Key::Oem_Period,
        "Oem_2", gep::Key::Oem_2,
        "Oem_3", gep::Key::Oem_3,
        "Oem_4", gep::Key::Oem_4,
        "Oem_5", gep::Key::Oem_5,
        "Oem_6", gep::Key::Oem_6,
        "Oem_7", gep::Key::Oem_7,
        "Oem_8", gep::Key::Oem_8,
        "Oem_102", gep::Key::Oem_102,
        "Processkey", gep::Key::Processkey,
        "Packet", gep::Key::Packet,
        "Attn", gep::Key::Attn,
        "CrSel", gep::Key::CrSel,
        "ExSel", gep::Key::ExSel,
        "EraseEof", gep::Key::EraseEof,
        "Play", gep::Key::Play,
        "Zoom", gep::Key::Zoom,
        "NoName", gep::Key::NoName,
        "Pa1", gep::Key::Pa1,
        "Oem_Clear", gep::Key::Oem_Clear,
        0);

    // Bind gamepad button enum
    scripting->bindEnum("Button",
        "Up",			XINPUT_GAMEPAD_DPAD_UP,
        "Down",			XINPUT_GAMEPAD_DPAD_DOWN,
        "Left",			XINPUT_GAMEPAD_DPAD_LEFT,
        "Right",		XINPUT_GAMEPAD_DPAD_RIGHT,
        "Start",		XINPUT_GAMEPAD_START,
        "Back",			XINPUT_GAMEPAD_BACK,
        "LeftThumb",	XINPUT_GAMEPAD_LEFT_THUMB,
        "RightThumb",	XINPUT_GAMEPAD_RIGHT_THUMB,
        "LeftShoulder",	XINPUT_GAMEPAD_LEFT_SHOULDER,
        "RightShoulder",XINPUT_GAMEPAD_RIGHT_SHOULDER,
        "A",			XINPUT_GAMEPAD_A,
        "B",			XINPUT_GAMEPAD_B,
        "X",			XINPUT_GAMEPAD_X,
        "Y",			XINPUT_GAMEPAD_Y,
        0);
}
