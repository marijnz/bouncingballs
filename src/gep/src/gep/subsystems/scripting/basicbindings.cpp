#include "stdafx.h"
#include "gepimpl/subsystems/scripting.h"

#include "gep/math3d/vec2.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/quaternion.h"
#include "gep/math3d/mat3.h"
#include "gep/math3d/mat4.h"

#include "gep/globalManager.h"
#include "gep/settings.h"

void gep::ScriptingManager::makeBasicBindings()
{
    auto scripting = static_cast<IScriptingManager*>(this);

    scripting->bind<ivec2>("Vec2i");
    scripting->bind<uvec2>("Vec2u");
    scripting->bind<vec2>("Vec2");
    scripting->bind<vec3>("Vec3");
    //scripting->bind<Quaternion>("_Quaternion");
    scripting->bind<mat3>("Mat3");
    scripting->bind<mat4>("Mat4");
    scripting->bind<Color>("Color");

    scripting->bind<ISettings>("Settings", g_globalManager.getSettings());
}
