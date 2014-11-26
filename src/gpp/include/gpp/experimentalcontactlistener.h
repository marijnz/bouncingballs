#pragma once

#include "gep\globalManager.h"
#include "gep\interfaces\logging.h"
#include "gep\interfaces\physics\contact.h"

namespace gpp
{
    class ExperimentalContactListener : public gep::IContactListener
    {
    public:
        ExperimentalContactListener(){}
        virtual ~ExperimentalContactListener(){}

        virtual void contactPointCallback(const gep::ContactPointArgs& evt) override
        {
            g_globalManager.getLogging()->logMessage("contactPointCallback!");
        }

        virtual void collisionAddedCallback(const gep::CollisionArgs& evt) override
        {
            g_globalManager.getLogging()->logMessage("collisionAddedCallback!");
        }

        virtual void collisionRemovedCallback(const gep::CollisionArgs& evt) override
        {
            g_globalManager.getLogging()->logMessage("collisionRemovedCallback!");
        }
    };
}
