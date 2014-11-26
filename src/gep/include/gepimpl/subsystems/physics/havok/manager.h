#pragma once
#include "gep/interfaces/physics/system.h"
#include "gepimpl/subsystems/physics/havok/world.h"
#include "gepimpl/subsystems/physics/havok/entity.h"
#include "gepimpl/subsystems/physics/havok/factory.h"

namespace gep
{
    class IWorld;
    class HavokDisplayManager;

    class HavokPhysicsManager : public IPhysicsSystem
    {
#ifdef GEP_USE_HK_VISUAL_DEBUGGER
        hkVisualDebugger* m_pVisualDebugger;
#endif // GEP_USE_HK_VISUAL_DEBUGGER

        hkpPhysicsContext* m_pPhysicsContext;
        DynamicArray<hkProcess*> m_physicsProcesses;

        HavokPhysicsFactory* m_pFactory;
        HavokDisplayManager* m_pDisplayManager;

        mutable SmartPtr<HavokWorld> m_pWorld;

    public:

        struct Options
        {
            // flags
            //Pattern: 1 << 0, 1 << 1, 1 << 2, ...
            enum Enum
            {
                None = 0,
                DebugDrawing = 1 << 0,
            };

            GEP_DISALLOW_CONSTRUCTION(Options);
        };

        HavokPhysicsManager(uint32 options = Options::None);

        virtual ~HavokPhysicsManager();

        virtual void initialize() override;
        virtual void destroy() override;
        virtual void update(float elapsedTime) override;

        virtual IWorld* getWorld() override { return m_pWorld.get(); }
        virtual const IWorld* getWorld() const override { return m_pWorld.get(); }
        void setWorld(IWorld* value) override;

        virtual void setDebugDrawingEnabled(bool value) override;
        virtual bool getDebugDrawingEnabled() const override;

        virtual IPhysicsFactory* getPhysicsFactory() override { return m_pFactory; }

    private:
        uint32 m_options;
        HavokPhysicsFactoryAllocator m_factoryAllocator;
    };
}
