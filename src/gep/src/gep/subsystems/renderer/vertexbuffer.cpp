#include "stdafx.h"
#include "gepimpl/subsystems/renderer/vertexbuffer.h"
#include "gep/container/hashmap.h"
#include "gep/exception.h"

const char* gep::Vertexbuffer::dataChannelToString(DataChannel channel)
{
    switch(channel)
    {
    case DataChannel::POSITION:
    case DataChannel::POSITION_2D:
        return "POSITION";
    case DataChannel::COLOR:
        return "COLOR";
    case DataChannel::NORMAL:
        return "NORMAL";
    case DataChannel::BINORMAL:
        return "BINORMAL";
    case DataChannel::TANGENT:
        return "TANGENT";
    case DataChannel::TEXCOORD0:
        return "TEXCOORD";
    case DataChannel::BONE_INDICES:
        return "BLENDINDICES";
    case DataChannel::BONE_WEIGHTS:
        return "BLENDWEIGHT";
    default:
        GEP_ASSERT(false, "value not handeled");
    }
    return nullptr;
}

void gep::Vertexbuffer::fillInputElementDesc(DataChannel channel, D3D11_INPUT_ELEMENT_DESC& desc, uint32& accumulatedOffset)
{
    desc.SemanticName = dataChannelToString(channel);
    desc.SemanticIndex = 0;
    desc.AlignedByteOffset = accumulatedOffset;
    switch(channel)
    {
    case DataChannel::POSITION:
    case DataChannel::NORMAL:
    case DataChannel::BINORMAL:
    case DataChannel::TANGENT:
        desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        accumulatedOffset += 3 * sizeof(float);
        break;
    case DataChannel::POSITION_2D:
    case DataChannel::TEXCOORD0:
        desc.Format = DXGI_FORMAT_R32G32_FLOAT;
        accumulatedOffset += 2 * sizeof(float);
        break;
    case DataChannel::COLOR:
        desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        accumulatedOffset += 4 * sizeof(float);
        break;
    case DataChannel::BONE_INDICES:
        desc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
        accumulatedOffset += 4 * sizeof(uint32);
        break;
    case DataChannel::BONE_WEIGHTS:
        desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        accumulatedOffset += 4 * sizeof(float);
        break;
    default:
        GEP_ASSERT(false, "value not handeled");
    }
    desc.InputSlot = 0;
    desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    desc.InstanceDataStepRate = 0;
}

gep::uint32 gep::Vertexbuffer::dataChannelByteCount(DataChannel channel)
{
    switch(channel)
    {
    case DataChannel::POSITION:
    case DataChannel::NORMAL:
    case DataChannel::BINORMAL:
    case DataChannel::TANGENT:
        return sizeof(float) * 3;
    case DataChannel::POSITION_2D:
    case DataChannel::TEXCOORD0:
        return sizeof(float) * 2;
    case DataChannel::COLOR:
    case DataChannel::BONE_WEIGHTS:
        return sizeof(float) * 4;
    case DataChannel::BONE_INDICES:
        return sizeof(uint32) * 4;
    default:
        GEP_ASSERT(false, "value not handeled");
    }
    return 0;
}

gep::uint32 gep::Vertexbuffer::getDataChannelSize(DataChannel channel)
{
    switch(channel)
    {
    case DataChannel::POSITION:
    case DataChannel::NORMAL:
    case DataChannel::BINORMAL:
    case DataChannel::TANGENT:
        return 3;
    case DataChannel::POSITION_2D:
    case DataChannel::TEXCOORD0:
        return 2;
    case DataChannel::COLOR:
    case DataChannel::BONE_WEIGHTS:
    case DataChannel::BONE_INDICES:
        return 4;
    default:
        GEP_ASSERT(false, "value not handeled");
    }
    return 0;
}

gep::uint32 gep::Vertexbuffer::primitiveNumElements(Primitive primitive)
{
    switch(primitive)
    {
    case Primitive::Triangle:
        return 3;
    case Primitive::Line:
        return 2;
    default:
        GEP_ASSERT(false, "value not handeled");
    }
    return 0;
}

gep::Vertexbuffer::Vertexbuffer(ID3D11Device* pDevice, ArrayPtr<DataChannel> dataChannels, Primitive primitive, Usage usage) :
    m_pDevice(pDevice),
    m_usage(usage),
    m_primitive(primitive),
    m_isDataUploaded(false),
    m_areIndicesUploaded(false),
    m_hasIndexBuffer(false),
    m_pDataBuffer(nullptr),
    m_pIndexBuffer(nullptr),
    m_uploadedData(0)
{
    m_layout = GEP_NEW_ARRAY(g_stdAllocator, D3D11_INPUT_ELEMENT_DESC, dataChannels.length());
    m_elementSize = 0;
    uint32 offset = 0;
    for(size_t i=0; i<dataChannels.length(); i++)
    {
        fillInputElementDesc(dataChannels[i], m_layout[i], offset);
        //m_layout[i].InputSlot = (UINT)i;
        m_elementSize += dataChannelByteCount(dataChannels[i]);
    }
    m_layoutHash = gep::hashOf(m_layout.getPtr(), m_layout.length() * sizeof(D3D11_INPUT_ELEMENT_DESC));
}

gep::Vertexbuffer::~Vertexbuffer()
{
    GEP_DELETE_ARRAY(g_stdAllocator, m_layout);
    if(m_pIndexBuffer != nullptr)
    {
        m_pIndexBuffer->Release();
        m_pIndexBuffer = nullptr;
    }
    if(m_pDataBuffer != nullptr)
    {
        m_pDataBuffer->Release();
        m_pDataBuffer = nullptr;
    }
}

void gep::Vertexbuffer::upload(ID3D11DeviceContext* pDeviceContext)
{
    if(m_isDataUploaded && m_uploadedData < m_data.length())
    {
        m_pDataBuffer->Release();
        m_pDataBuffer = nullptr;
        m_isDataUploaded = false;
    }
    if(m_data.length() > 0)
    {
        if(!m_isDataUploaded)
        {
            m_dataDesc.Usage = (m_usage == Usage::Dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
            m_dataDesc.ByteWidth = (UINT)(m_data.length() * sizeof(float));
            m_dataDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            m_dataDesc.CPUAccessFlags = (m_usage == Usage::Dynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
            m_dataDesc.MiscFlags = 0;
            m_dataDesc.StructureByteStride = 0;

            HRESULT hr = S_OK;
            D3D11_SUBRESOURCE_DATA initData;
            initData.pSysMem = m_data.toArray().getPtr();
            initData.SysMemPitch = 0;
            initData.SysMemSlicePitch = 0;

            hr = m_pDevice->CreateBuffer(&m_dataDesc, &initData, &m_pDataBuffer);
            if(FAILED(hr))
                throw Exception("Failed to create vertex data buffer");

            m_isDataUploaded = true;
            m_uploadedData = m_data.length();
        }
        else
        {
            D3D11_MAPPED_SUBRESOURCE res;
            HRESULT hr = pDeviceContext->Map(m_pDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
            GEP_ASSERT(SUCCEEDED(hr));
            auto newData = m_data.toArray();
            memcpy(res.pData, newData.getPtr(), newData.length() * sizeof(float));
            pDeviceContext->Unmap(m_pDataBuffer, 0);
        }
    }

    if(m_areIndicesUploaded && m_uploadedIndices < m_indices.length())
    {
        m_pIndexBuffer->Release();
        m_pIndexBuffer = nullptr;
        m_areIndicesUploaded = false;
    }
    if(m_indices.length() > 0)
    {
        m_hasIndexBuffer = true;
        if(!m_areIndicesUploaded)
        {
            m_indexDesc.Usage = (m_usage == Usage::Dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
            m_indexDesc.ByteWidth = (UINT)(m_indices.length() * sizeof( uint32 ));
            m_indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            m_indexDesc.CPUAccessFlags = (m_usage == Usage::Dynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
            m_indexDesc.MiscFlags = 0;

            HRESULT hr = S_OK;
            D3D11_SUBRESOURCE_DATA initData;
            initData.pSysMem = m_indices.toArray().getPtr();
            hr = m_pDevice->CreateBuffer(&m_indexDesc, &initData, &m_pIndexBuffer);
            if(FAILED(hr))
                throw Exception("Failed to create vertex index buffer");

            m_areIndicesUploaded = true;
            m_hasIndexBuffer = true;
            m_uploadedIndices = m_indices.length();
        }
        else
        {
            D3D11_MAPPED_SUBRESOURCE res;
            HRESULT hr = pDeviceContext->Map(m_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
            GEP_ASSERT(SUCCEEDED(hr));
            auto newData = m_indices.toArray();
            memcpy(res.pData, newData.getPtr(), newData.length() * sizeof(uint32));
            pDeviceContext->Unmap(m_pIndexBuffer, 0);
        }
    }
    else
    {
        m_hasIndexBuffer = false;
    }

}

void gep::Vertexbuffer::use(ID3D11DeviceContext* pDeviceContext)
{
    UINT stride = m_elementSize;
    UINT offset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &m_pDataBuffer, &stride, &offset);
    if(m_hasIndexBuffer)
        pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    switch(m_primitive)
    {
    case Primitive::Triangle:
        pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        break;
    case Primitive::Line:
        pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        break;
    default:
        GEP_ASSERT(false, "unhandeled value");
    }
}

void gep::Vertexbuffer::draw(ID3D11DeviceContext* pDeviceContext)
{
    if(m_hasIndexBuffer)
    {
        pDeviceContext->DrawIndexed((UINT)m_indices.length(), 0, 0);
    }
    else
    {
        pDeviceContext->Draw((UINT)(m_data.length() / (m_elementSize / 4)), 0);
    }
}

void gep::Vertexbuffer::draw(ID3D11DeviceContext* pDeviceContext, uint32 startIndex, uint32 numIndices)
{
    if(m_hasIndexBuffer)
    {
        pDeviceContext->DrawIndexed(numIndices, startIndex, 0);
    }
    else
    {
        pDeviceContext->Draw(numIndices, startIndex);
    }
}
