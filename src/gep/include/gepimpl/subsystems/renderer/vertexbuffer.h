#pragma once

#include <d3d11.h>
#include "gep/container/DynamicArray.h"

namespace gep
{

    class Vertexbuffer
    {
    public:
        enum class DataChannel
        {
            POSITION,
            POSITION_2D,
            COLOR,
            NORMAL,
            BINORMAL,
            TANGENT,
            TEXCOORD0,
            BONE_INDICES,
            BONE_WEIGHTS
        };

        enum class Usage
        {
            Static,
            Dynamic
        };

        enum class Primitive
        {
            Triangle,
            Line
        };
    private:
        ID3D11Device* m_pDevice;
        D3D11_BUFFER_DESC m_dataDesc;
        D3D11_BUFFER_DESC m_indexDesc;
        ID3D11Buffer* m_pDataBuffer;
        ID3D11Buffer* m_pIndexBuffer;
        ArrayPtr<D3D11_INPUT_ELEMENT_DESC> m_layout;
        uint32 m_layoutHash;
        DynamicArray<float> m_data;
        DynamicArray<uint32> m_indices;
        uint32 m_elementSize;
        Usage m_usage;
        Primitive m_primitive;
        bool m_isDataUploaded;
        bool m_areIndicesUploaded;
        bool m_hasIndexBuffer;
        size_t m_uploadedData;
        size_t m_uploadedIndices;

        const char* dataChannelToString(DataChannel channel);
        void fillInputElementDesc(DataChannel channel, D3D11_INPUT_ELEMENT_DESC& desc, uint32& accumulatedOffset);
        uint32 dataChannelByteCount(DataChannel channel);
        uint32 primitiveNumElements(Primitive primitive);

    public:
        Vertexbuffer(ID3D11Device* pDevice, ArrayPtr<DataChannel> dataChannels, Primitive primitive, Usage usage);
        ~Vertexbuffer();

        void upload(ID3D11DeviceContext* pDeviceContext);
        void use(ID3D11DeviceContext* pDeviceContext);
        void draw(ID3D11DeviceContext* pDeviceContext);
        void draw(ID3D11DeviceContext* pDeviceContext, uint32 startIndex, uint32 numIndices);

        inline void addData(float x, float y)
        {
            size_t cur = m_data.length();
            m_data.resize(cur+2);
            m_data[cur] = x;
            m_data[cur+1] = y;
        }

        inline void addData(float x, float y, float z)
        {
            size_t cur = m_data.length();
            m_data.resize(cur+3);
            m_data[cur] = x;
            m_data[cur+1] = y;
            m_data[cur+2] = z;
        }

        inline void addData(float r, float g, float b, float a)
        {
            size_t cur = m_data.length();
            m_data.resize(cur+4);
            m_data[cur] = r;
            m_data[cur+1] = g;
            m_data[cur+2] = b;
            m_data[cur+3] = a;
        }

        inline uint32 getCurrentNumVertices() const { return (uint32)m_data.length() / (m_elementSize / 4); }
        inline DynamicArray<float>& getData() { return m_data; }
        inline DynamicArray<uint32>& getIndices() { return m_indices; }
        inline const ArrayPtr<D3D11_INPUT_ELEMENT_DESC> getLayout() const { return m_layout; }
        inline uint32 getLayoutHash() const { return m_layoutHash; }
        static uint32 getDataChannelSize(DataChannel channel);
    };

}
