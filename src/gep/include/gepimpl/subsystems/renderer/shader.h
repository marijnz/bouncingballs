#pragma once

#include "gep/interfaces/resourceManager.h"
#include "gep/container/hashmap.h"
#include "gep/math3d/vec2.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/mat3.h"
#include "gep/math3d/mat4.h"
#include "gep/math3d/color.h"

struct ID3DX11Effect;
struct ID3D11Device;
struct ID3DX11EffectTechnique;
struct ID3D11DeviceContext;
struct ID3DX11EffectVectorVariable;
struct ID3DX11EffectMatrixVariable;
struct ID3DX11EffectShaderResourceVariable;
struct ID3DX11EffectScalarVariable;
struct ID3DX11EffectPass;

namespace gep
{
    // forward declarations
    class Shader;
    class Renderer;
    class Vertexbuffer;
    class Texture2D;
    

    /// \brief interface for loading shaders
    class IShaderLoader : public IResourceLoader
    {
    public:
        virtual Shader* loadResource(Shader* pInPlace) = 0;
        virtual IResource* loadResource(IResource* pInPlace) override;
        virtual const char* getResourceType() override;
        virtual void deleteResource(IResource* pResource) override;
        virtual void release() override;
    };

    /// \brief loads a shader from a fx file
    class ShaderFileLoader : public IShaderLoader
    {
    private:
        std::string m_filename;
        Renderer* m_pRenderer;
        bool m_isRegistered;

    public:
        ShaderFileLoader(const char* filename);
        ~ShaderFileLoader();
        virtual Shader* loadResource(Shader* pInPlace) override;
        virtual void postLoad(ResourcePtr<IResource> pResource) override;
        virtual ShaderFileLoader* moveToHeap() override;
        virtual const char* getResourceId() override;
    };

    /// \brief shader constant base class
    class ShaderConstantBase
    {
    protected:
        std::string m_name;
        Shader* m_pLastValidShader;
        ResourcePtr<Shader> m_pShader;
        bool m_isValid;
    public:
        ShaderConstantBase();
        virtual ~ShaderConstantBase(){}
        ShaderConstantBase(const char* name, ResourcePtr<Shader> pShader);
    };

    /// \brief a shader constant
    template <class T>
    class ShaderConstant : public ShaderConstantBase
    {
        //static_assert(false, "type not supported");
    };

    template <>
    class ShaderConstant<mat3> : public ShaderConstantBase
    {
    private:
        ID3DX11EffectMatrixVariable* m_pVar;
        void checkUpToDate();
    public:
        inline ShaderConstant(){}
        ShaderConstant(const char* name, ResourcePtr<Shader> pShader);
        void set(const mat3& value);
    };

    template <>
    class ShaderConstant<mat4> : public ShaderConstantBase
    {
    private:
        ID3DX11EffectMatrixVariable* m_pVar;
        void checkUpToDate();
    public:
        inline ShaderConstant(){}
        ShaderConstant(const char* name, ResourcePtr<Shader> pShader);
        void set(mat4& value);
        void setArray(ArrayPtr<mat4> arr);
    };

    template <>
    class ShaderConstant<vec2> : public ShaderConstantBase
    {
    private:
        ID3DX11EffectVectorVariable* m_pVar;
        void checkUpToDate();
    public:
        inline ShaderConstant(){}
        ShaderConstant(const char* name, ResourcePtr<Shader> pShader);
        void set(const vec2& value);
    };

    template <>
    class ShaderConstant<vec3> : public ShaderConstantBase
    {
    private:
        ID3DX11EffectVectorVariable* m_pVar;
        void checkUpToDate();
    public:
        inline ShaderConstant(){}
        ShaderConstant(const char* name, ResourcePtr<Shader> pShader);
        void set(const vec3& value);
    };

    template <>
    class ShaderConstant<Color> : public ShaderConstantBase
    {
    private:
        ID3DX11EffectVectorVariable* m_pVar;
        void checkUpToDate();
    public:
        inline ShaderConstant(){}
        ShaderConstant(const char* name, ResourcePtr<Shader> pShader);
        void set(Color& value);
    };

    template<>
    class ShaderConstant<Texture2D> : public ShaderConstantBase
    {
    private:
        ID3DX11EffectShaderResourceVariable* m_pVar;
        void checkUpToDate();
    public:
        inline ShaderConstant(){}
        ShaderConstant(const char* name, ResourcePtr<Shader> pShader);
        void set(ResourcePtr<Texture2D> pTexture);
    };

    template<>
    class ShaderConstant<uint32> : public ShaderConstantBase
    {
    private:
        ID3DX11EffectScalarVariable* m_pVar;
        void checkUpToDate();
    public:
        inline ShaderConstant(){}
        ShaderConstant(const char* name, ResourcePtr<Shader> pShader);
        void set(uint32 value);
    };

    /// \brief a shader
    class Shader : public IResource
    {
    private:
        ID3DX11Effect* m_pEffect;
        IShaderLoader* m_pLoader;
        ID3D11Device* m_pDevice;
        ID3DX11EffectTechnique* m_pTechnique;
        ID3D10Blob* m_pByteCode;
        ID3DX11EffectPass* m_pPass;
        Hashmap<uint32, ID3D11InputLayout*, DontHashPolicy> m_inputLayouts;

    public:
        Shader(ID3D11Device* pDevice);
        ~Shader();

        inline ID3DX11Effect* getEffect()
        {
            return m_pEffect;
        }
        inline ID3DX11EffectTechnique* getTechnique()
        {
            GEP_ASSERT(m_pTechnique != nullptr, "not finalized yet");
            return m_pTechnique;
        }

        void loadFromFX(const char* filename);
        void use(ID3D11DeviceContext* pContext, Vertexbuffer* pVertexbuffer);

        //IResource interface
        virtual IResource* getSuperResource() override;
        virtual const char* getResourceType() override;
        virtual IShaderLoader* getLoader() override;
        virtual void setLoader(IResourceLoader* loader) override;
        virtual void unload() override;
        virtual void finalize() override;
        virtual uint32 getFinalizeOptions() override;
        virtual bool isLoaded() override;
    };
}
