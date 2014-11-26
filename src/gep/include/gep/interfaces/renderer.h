#pragma once

#include "gep/interfaces/subsystem.h"
#include "gep/math3d/vec2.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/vec4.h"
#include "gep/interfaces/scripting.h"
#include "gep/math3d/mat4.h"
#include "gep/math3d/color.h"
#include "gep/math3d/quaternion.h"
#include <functional>

#include "gep/interfaces/resourceManager.h"
#include "gep/interfaces/updateFramework.h"
#include "gep/container/DynamicArray.h"

namespace gep
{
    // forward declarations
    class IModel;

    /// \brief Interface for debug drawing
    class IDebugRenderer
    {
    public:
        virtual ~IDebugRenderer(){}
        /// \brief draws a 3d line
        virtual void drawLine(const vec3& start, const vec3& end, Color color = Color::white()) = 0;
        /// \brief draws a 2d line
        virtual void drawLine(const vec2& start, const vec2& end, Color color = Color::white()) = 0;
        /// \brief draws a 3d arrow
        virtual void drawArrow(const vec3& start, const vec3& end, Color color = Color::white()) = 0;
        /// \brief draws a 3d axis aligned box
        virtual void drawBox(const vec3& min, const vec3& max, Color color = Color::white()) = 0;

        /// \brief prints text on the screen (2D)
        virtual void printText(const vec2& screenPositionNormalized, const char* text, Color color = Color::white()) = 0;
        /// \brief prints text in the world (3D)
        virtual void printText(const vec3& worldPosition, const char* text, Color color = Color::white()) = 0;

        virtual void drawLocalAxes(
            const vec3& objectPosition,
            const Quaternion& objectRotation,
            float axesScale = 10.0f,
            Color colorX = Color::red(),
            Color colorY = Color::green(),
            Color colorZ = Color::blue()) = 0;

        virtual void drawLocalAxes(
            const vec3& objectPosition,
            float axesScale = 10.0f,
            Color colorX = Color::red(),
            Color colorY = Color::green(),
            Color colorZ = Color::blue()) = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION_PTR(static_cast<void(IDebugRenderer::*)(const vec2&, const char*, Color)>(&printText), "_printText2D")
            LUA_BIND_FUNCTION_PTR(static_cast<void(IDebugRenderer::*)(const vec3&, const char*, Color)>(&printText), "_printText3D")
            LUA_BIND_FUNCTION_PTR(static_cast<void(IDebugRenderer::*)(const vec3&, const vec3&, Color)>(&drawLine), "_drawLine3D")
            LUA_BIND_FUNCTION_PTR(static_cast<void(IDebugRenderer::*)(const vec2&, const vec2&, Color)>(&drawLine), "_drawLine2D")
            LUA_BIND_FUNCTION_NAMED(drawArrow, "_drawArrow")
            LUA_BIND_FUNCTION_NAMED(drawBox, "_drawBox")
            LUA_BIND_FUNCTION_NAMED(drawLocalAxesFromScript, "_drawLocalAxes")
        LUA_BIND_REFERENCE_TYPE_END

    private:
        void drawLocalAxesFromScript(lua::TableWrapper args)
        {
            vec3 position(DO_NOT_INITIALIZE);
            if(!args.tryGet("position", position))
            {
                throw ScriptExecutionException("DebugDrawer:drawLocalAxes => Missing required argument 'position' (vec3)");
            }

            Quaternion rotation(DO_NOT_INITIALIZE);
            bool hasRotation = args.tryGet("rotation", rotation);

            float axesScale = 1.0f;
            args.tryGet("axesScale", axesScale);

            Color colorX = Color::red();
            args.tryGet("colorX", colorX);

            Color colorY = Color::green();
            args.tryGet("colorY", colorY);

            Color colorZ = Color::blue();
            args.tryGet("colorZ", colorZ);

            if (hasRotation)
                drawLocalAxes(position, rotation, axesScale, colorX, colorY, colorZ);
            else
                drawLocalAxes(position,           axesScale, colorX, colorY, colorZ);
        }
    };

    /// \brief interface for drawing 2d elements
    class IContext2D
    {
    public:
        virtual ~IContext2D(){}
        /// \brief prints text on the screen
        virtual void printText(const vec2& screenPositionNormalized, const char* text, Color color = Color::white()) = 0;
    };

    /// \brief Renderer interface
    class IRenderer : public ISubsystem
    {
        friend class InputHandler;
    public:
        virtual ~IRenderer(){}
        /// \brief returns the debug renderer interface
        virtual IDebugRenderer& getDebugRenderer() = 0;
        /// \brief loads a model from a file
        virtual ResourcePtr<IModel> loadModel(const char* path) = 0;
        virtual ResourcePtr<IModel> loadModel(ReferenceCounted* pDataHolder, ArrayPtr<vec4> vertices, ArrayPtr<uint32> indices) = 0;
        /// \brief creates a new texture generator
        virtual ResourcePtr<IResource> createGeneratedTexture(uint32 width, uint32 height, const char* resourceId, std::function<void(ArrayPtr<uint8>)> generatorFunction) = 0;

        virtual uint32 getScreenWidth() const = 0;
        virtual uint32 getScreenHeight() const = 0;

        virtual bool getVSyncEnabled() const = 0;
        virtual void setVSyncEnabled(bool value) = 0;

        virtual bool getAdaptiveVSyncEnabled() const = 0;
        virtual void setAdaptiveVSyncEnabled(bool value) = 0;

        virtual uvec2 toAbsoluteScreenPosition(const vec2& screenPosNormalized) const = 0;
        virtual vec2 toNormalizedScreenPosition(const uvec2& screenPos) const = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getScreenWidth)
            LUA_BIND_FUNCTION(getScreenHeight)
            LUA_BIND_FUNCTION(toAbsoluteScreenPosition)
            LUA_BIND_FUNCTION(toNormalizedScreenPosition)
        LUA_BIND_REFERENCE_TYPE_END

    private:
        virtual HWND getWindowHandle() = 0;
    };

    struct GEP_API Ray
    {
        vec3 from;
        vec3 direction;

        Ray(vec3 from, vec3 direction):
            from(from),
            direction(direction)
        {
        }

        LUA_BIND_VALUE_TYPE_BEGIN
            LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(from)
            LUA_BIND_MEMBER(direction)
        LUA_BIND_VALUE_TYPE_END
    };

    /// \brief Camera interface
    class GEP_API ICamera
    {
    public:
        virtual ~ICamera() =0 {};
        virtual const mat4 getViewMatrix() const = 0;
        virtual const mat4 getProjectionMatrix() const = 0;
    };

    /// \brief Renderer extractor interface
    class IRendererExtractor
    {
    public:
        virtual ~IRendererExtractor(){}
        virtual CallbackId registerExtractionCallback(std::function<void(IRendererExtractor& extractor)> callback) = 0;
        virtual void deregisterExtractionCallback(CallbackId callbackId) = 0;

        /// \brief runs the extraction
        virtual void extract() = 0;

        /// \brief gets the 2d draw interface
        virtual IContext2D& getContext2D() = 0;

        /// \brief sets the camera to be used
        virtual void setCamera(ICamera* camera) = 0;

        /// \brief debugging markers
        virtual void beginDebugMarker(const char* name) = 0;
        virtual void endDebugMarker() = 0;

        virtual IAllocator* getCurrentAllocator() = 0;
    };


    /// \brief model interface
    class IModel : public IResource
    {
    public:
        virtual ~IModel() {}
        virtual void extract(IRendererExtractor& extractor, mat4 modelMatrix) = 0;

        virtual void setDebugDrawingEnabled(bool value) = 0;
        virtual bool getDebugDrawingEnabled() const = 0;
        /// \brief Convenience function to toggle whether this model draws a debug version of itself
        /// (usually drawing its wireframe, bones, etc).
        virtual void toggleDebugDrawing() = 0;
        virtual void setBones(const ArrayPtr<gep::mat4>& transformations) = 0;
        virtual void getBoneNames(DynamicArray<const char*>& names) = 0;
    };

    class IAnimation
    {
    public:
        virtual ~IAnimation() = 0 {}

        virtual void update(float elapsedSeconds) = 0;
    };

    struct DebugMarkerSection
    {
    private:
        IRendererExtractor& m_extractor;

    public:
        DebugMarkerSection(IRendererExtractor& extractor, const char* name)
            : m_extractor(extractor)
        {
            extractor.beginDebugMarker(name);
        }

        ~DebugMarkerSection()
        {
            m_extractor.endDebugMarker();
        }
    };

};

