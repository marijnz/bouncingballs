#include "stdafx.h"
#include "gpp/gameComponents/renderComponent.h"
#include "gep/globalManager.h"
#include "gep/interfaces/renderer.h"
#include "gep/interfaces/logging.h"


#include "gep/interfaces/scripting.h"
#include "gep/interfaces/animation.h"

#include "gep/ArrayPtr.h"
#include "gep/math3d/mat4.h"

#include <array>


gpp::RenderComponent::RenderComponent():
    Component(),
    m_path(),
    m_pModel(nullptr),
    m_scale(1, 1, 1),
    m_extractionCallbackId(0),
    m_bones()
{

}

gpp::RenderComponent::~RenderComponent()
{
    m_pModel = nullptr;
}

void gpp::RenderComponent::initalize()
{
    if(m_path.empty())
    {
        g_globalManager.getLogging()->logError("Render Component's path on GameObject %s hasn't been set!", m_pParentGameObject->getGuid().c_str());
        GEP_ASSERT(false);
    }
    else
    {
        m_pModel = g_globalManager.getRenderer()->loadModel(m_path.c_str());
    }

    if (m_state != State::Initial) { return; } // User already set a different state.
    setState(State::Active);
}

void gpp::RenderComponent::update(float elapsedMS)
{
}

void gpp::RenderComponent::destroy()
{
    setState(State::Inactive);
}

void gpp::RenderComponent::extract(gep::IRendererExtractor& extractor)
{
    m_pModel->setBones(m_bones.toArray());
    m_pModel->extract(extractor, m_pParentGameObject->getWorldTransformationMatrix() * gep::mat4::scaleMatrix(m_scale));
}

void gpp::RenderComponent::getBoneNames(gep::DynamicArray<const char*>& names)
{
    m_pModel->getBoneNames(names);
}

void gpp::RenderComponent::setBoneMapping(const gep::DynamicArray<gep::uint32>& boneIds)
{
    m_boneMapping = boneIds;
}

void gpp::RenderComponent::applyBoneTransformations(const gep::ArrayPtr<gep::mat4>& transformations)
{
    GEP_ASSERT(m_boneMapping.length() <= transformations.length(), "Bonemapping does not match transformations");
    m_bones.resize(transformations.length());

    int i = 0;
    for (auto index :m_boneMapping)
    {
        m_bones[i] = transformations[index];
        i++;
    }

}

void gpp::RenderComponent::setState(State::Enum state)
{
    if (m_state == state) { return; }

    m_state = state;

    switch (state)
    {
    case State::Active:
        {
            if (m_extractionCallbackId.id == 0)
            {
                m_extractionCallbackId = g_globalManager.getRendererExtractor()->registerExtractionCallback(
                    std::bind(&RenderComponent::extract,this,std::placeholders::_1));
            }
        }
        break;
    case State::Inactive:
        {
            if (m_extractionCallbackId.id != 0)
            {
                g_globalManager.getRendererExtractor()->deregisterExtractionCallback(m_extractionCallbackId);
                m_extractionCallbackId.id = 0;
            }
        }
        break;
    default:
        GEP_ASSERT(false, "Invalid State!", m_state, state);
        break;
    }
}
