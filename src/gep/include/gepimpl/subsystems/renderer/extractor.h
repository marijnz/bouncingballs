#pragma once

#include "gep/interfaces/renderer.h"
#include "gepimpl/subsystems/renderer/renderer.h"
#include "gep/memory/allocators.h"
#include "gep/types.h"
#include "gep/interfaces/resourceManager.h"
#include "gep/math3d/vec2.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/mat4.h"
#include "gep/math3d/color.h"
#include "gep/traits.h"
#include "gep/threading/semaphore.h"
#include "gep/interfaces/updateFramework.h"

namespace gep
{
    //forward declarations
    class Model;
    class RendererExtractor;

    enum class CommandType : uint16
    {
        Invalid,
        FirstCommand,
        RenderModel,
        Text,
        TextBillboard,
        RenderLines,
        RenderLines2D,
        Camera,
        DebugMarkerBegin,
        DebugMarkerEnd
    };

    struct CommandBase
    {
        friend class RendererExtractor;
    private:
        CommandType type;
        uint16 offsetNext;
    public:
        CommandType getType() const { return type; }
    };

    struct CommandRenderModel : public CommandBase
    {
        static const CommandType TYPE = CommandType::RenderModel;
        ResourcePtr<Model> model;
        mat4 modelMatrix;
        ArrayPtr<mat4> bones;
    };

    struct LineInfo {
        vec3 start, end;
    };

    struct LineInfo2D {
        vec2 start, end;
    };

    struct CommandRenderLines : public CommandBase
    {
        static const CommandType TYPE = CommandType::RenderLines;
        Color color;
        ArrayPtr<LineInfo> lines;
        uint32 startIndex;
    };

    struct CommandRenderLines2D : public CommandBase
    {
        static const CommandType TYPE = CommandType::RenderLines2D;
        Color color;
        ArrayPtr<LineInfo2D> lines;
        uint32 startIndex;
    };

    struct CommandDrawText : public CommandBase
    {
        static const CommandType TYPE = CommandType::Text;
        uvec2 position;
        Color color;
        union
        {
            const char* text;
            RenderTextInfo textInfo;
        };
    };

    struct CommandDrawTextBillboard : public CommandBase
    {
        static const CommandType TYPE = CommandType::TextBillboard;
        vec3 position;
        Color color;
        union
        {
            const char* text;
            RenderTextInfo textInfo;
        };
    };

    struct CommandCamera : public CommandBase
    {
        static const CommandType TYPE = CommandType::Camera;
        mat4 viewMatrix;
        mat4 projectionMatrix;
    };

    struct CommandDebugMarkerBegin : public CommandBase
    {
        static const CommandType TYPE = CommandType::DebugMarkerBegin;
        const wchar_t* name;
    };

    struct CommandDebugMarkerEnd : public CommandBase
    {
        static const CommandType TYPE = CommandType::DebugMarkerEnd;
    };

    class Context2D : public IContext2D
    {
    private:
        RendererExtractor& m_extractor;

    public:
        Context2D(RendererExtractor& extractor);

        void printText(const vec2& screenPositionNormalized, const char* text, Color color = Color::white()) override;
    };

    class RendererExtractor : public IRendererExtractor
    {
    private:
        struct Pool
        {
            StackAllocator* pAllocator;
            void* pStart;
            bool hasData;

            inline Pool()
            {
                pAllocator = new StackAllocator(true, 1024 * 1024 * 1);
                pStart = pAllocator->getMarker();
                hasData = false;
            }

            inline ~Pool()
            {
                DELETE_AND_NULL(pAllocator)
            }
        };

        static const uint32 NUM_POOLS = 2;

        Pool m_pools[NUM_POOLS];
        StackAllocator* m_pCurrentAllocator;
        CommandBase* m_pLastCommand;
        DynamicArray<std::function<void(IRendererExtractor& extractor)>> m_callbacks;
        bool m_isExtracting;
        uint32 m_nextPoolToFill;
        uint32 m_nextPoolToRead;
        Semaphore m_fullPoolSync;
        Semaphore m_emptyPoolSync;
        Context2D m_context2d;


        void* doMakeCommand(size_t size, CommandType type);
    public:
        RendererExtractor();
        ~RendererExtractor();

        template <class T>
        T& makeCommand()
        {
            static_assert(std::is_convertible<T*, CommandBase*>::value == true, "the given type is not a renderer extractor command");
            return *(T*)doMakeCommand(sizeof(T), T::TYPE);
        }

        virtual CallbackId registerExtractionCallback(std::function<void(IRendererExtractor& extractor)> callback) override;
        virtual void deregisterExtractionCallback(CallbackId callbackId) override;
        virtual void extract() override;
        virtual IContext2D& getContext2D() override;
        virtual void setCamera(ICamera* pCamera) override;

        virtual void beginDebugMarker(const char* name) override;
        virtual void endDebugMarker() override;

        CommandBase* startReadCommands();
        void endReadCommands();
        CommandBase* nextCommand(CommandBase* lastCommand);

        template <class T>
        static T* command_cast(CommandBase* base)
        {
            GEP_ASSERT(base->type == T::TYPE, "casting to wrong type");
            return (T*)base;
        }

        virtual IAllocator* getCurrentAllocator() override { return m_pCurrentAllocator; }
    };
}

