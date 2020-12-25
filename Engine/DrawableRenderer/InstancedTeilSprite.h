#ifndef _ITSprite_H_
#define _ITSprite_H_

#include "DrawableRenderer.h"
#include "../other/Transform.h"

#include <Serializer_Include.h>

class InstancedTeilSprite : public Drawable
{
private:
    static ComponentRegister<InstancedTeilSprite> reg;
public:

    struct InstancedTeilSpriteVertex
    {
        XMFLOAT3 pos;
        XMFLOAT2 tex0;
    };

    class Instance : public Component {
    private:
        static ComponentRegister<InstancedTeilSprite::Instance> reg;
    public:
        Transform* transform;
        XMFLOAT4 rect;

        static std::vector<std::string> getRequirements() {
            std::vector<std::string> toRet = std::vector<std::string>();
            toRet.push_back(typeid(Transform).name());
            return toRet;
        };
        static bool getIsLonly() { return false; }; 
        void setRequirements(std::vector <Component*> toSet) {
            transform = (Transform*)toSet[0];
        }
        virtual void set(SerializedObject& from) {
            from >> rect;
        };
        virtual void afterSet() {
        }
        virtual SerializedObject serialize() {
            SerializedObject so;
            return serialize(so);
        };
        virtual SerializedObject serialize(SerializedObject& inTo) {
            inTo << rect;
            inTo << typeid(InstancedTeilSprite::Instance).name();
            return inTo;
        };
    };

    struct InMemInstance {
        XMFLOAT4 rect;
        XMMATRIX mat;
    };

    InstancedTeilSprite()
    {
        m_shaderPath = "";
        m_texturePath = "";
        m_instancen = std::vector<Instance*>();
        m_pVertexBuffer = NULL;
        m_pColorMap = NULL;
        m_pColorMapSampler = NULL;
        m_pAlphaBlendState = NULL;
        m_instanceBuffer = NULL;
    }
    ~InstancedTeilSprite()
    {
    }

    virtual bool registerShader(DrawableRenderer* renderer)
    {
        if (m_shaderPath._Equal("")) {
            ::MessageBox(renderer->m_hWnd, L"es ist kein shader path angegeben", L"error", MB_OK);
            return false;
        }

        D3D11_INPUT_ELEMENT_DESC shaderInputLayout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        UINT numLayoutElements = ARRAYSIZE(shaderInputLayout);

        m_shaderContainer = renderer->addShaderByPath(m_shaderPath, shaderInputLayout, numLayoutElements);

        return true;
    }
    virtual bool load(DrawableRenderer* renderer)
    {
        if (m_texturePath._Equal("")) {
            ::MessageBox(renderer->m_hWnd, L"es ist kein texture path angegeben", L"error", MB_OK);
            return false;
        }

        std::wstring tPath = std::wstring(m_texturePath.begin(), m_texturePath.end());

        HRESULT hr = ::D3DX11CreateShaderResourceViewFromFile(
            renderer->m_pD3DDevice, tPath.c_str(), 0, 0, &m_pColorMap, 0);
        if (FAILED(hr)) {
            ::MessageBox(renderer->m_hWnd, L"Unable to load texture", L"ERROR", MB_OK);
            return false;
        }

        D3D11_SAMPLER_DESC textureDesc;
        ::ZeroMemory(&textureDesc, sizeof(textureDesc));
        textureDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        textureDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        textureDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        textureDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        textureDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        textureDesc.MaxLOD = D3D11_FLOAT32_MAX;
        hr = renderer->m_pD3DDevice->CreateSamplerState(&textureDesc, &m_pColorMapSampler);
        if (FAILED(hr)) {
            ::MessageBox(renderer->m_hWnd, L"Unable to create texture sampler state", L"ERROR", MB_OK);
            return false;
        }

        ID3D11Resource* pColorMapRes;
        m_pColorMap->GetResource(&pColorMapRes);
        D3D11_TEXTURE2D_DESC colorMapDesc;
        ((ID3D11Texture2D*)pColorMapRes)->GetDesc(&colorMapDesc);
        pColorMapRes->Release();
        float halfWidth = (float)colorMapDesc.Width / 2.0f;
        float halfHeight = (float)colorMapDesc.Height / 2.0f;

        InstancedTeilSpriteVertex vertices[] =
        {
            { XMFLOAT3(0.5f,  0.5f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
            { XMFLOAT3(0.5f, -0.5f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
            { XMFLOAT3(-0.5f, -0.5f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

            { XMFLOAT3(-0.5f, -0.5f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
            { XMFLOAT3(-0.5f,  0.5f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
            { XMFLOAT3(0.5f,  0.5f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
        };

        D3D11_BUFFER_DESC vertexDesc;
        ::ZeroMemory(&vertexDesc, sizeof(vertexDesc));
        vertexDesc.Usage = D3D11_USAGE_DEFAULT;
        vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexDesc.ByteWidth = sizeof(InstancedTeilSpriteVertex) * 6;

        D3D11_SUBRESOURCE_DATA resourceData;
        ZeroMemory(&resourceData, sizeof(resourceData));
        resourceData.pSysMem = vertices;

        hr = renderer->m_pD3DDevice->CreateBuffer(&vertexDesc, &resourceData, &m_pVertexBuffer);
        if (FAILED(hr)) {
            return false;
        }

        D3D11_BUFFER_DESC constDesc0;
        ::ZeroMemory(&constDesc0, sizeof(constDesc0));
        constDesc0.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constDesc0.ByteWidth = sizeof(InMemInstance);
        constDesc0.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
        constDesc0.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constDesc0.MiscFlags = 0;
        hr = renderer->m_pD3DDevice->CreateBuffer(&constDesc0, 0, &m_instanceBuffer);
        if (FAILED(hr)) {
            return false;
        }

        D3D11_BLEND_DESC blendDesc;
        ::ZeroMemory(&blendDesc, sizeof(blendDesc));
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;
        float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

        renderer->m_pD3DDevice->CreateBlendState(&blendDesc, &m_pAlphaBlendState);
        renderer->m_pD3DContext->OMSetBlendState(m_pAlphaBlendState, blendFactor, 0xFFFFFFFF);

        return true;
    }
    virtual void unload(DrawableRenderer* renderer)
    {
        if (m_pVertexBuffer)
            m_pVertexBuffer->Release();
        m_pVertexBuffer = NULL;
        if (m_pColorMap)
            m_pColorMap->Release();
        m_pColorMap = NULL;
        if (m_pColorMapSampler)
            m_pColorMapSampler->Release();
        m_pColorMapSampler = NULL;
        if (m_pAlphaBlendState)
            m_pAlphaBlendState->Release();
        m_pAlphaBlendState = NULL;
        if (m_instanceBuffer)
            m_instanceBuffer->Release();
        m_instanceBuffer = NULL;
    }

    virtual void render(DrawableRenderer* renderer, XMMATRIX projectionMatrix)
    {
        if (!isActiv)
            return;

        UINT stride = sizeof(InstancedTeilSpriteVertex);
        UINT offset = 0;

        renderer->m_pD3DContext->IASetInputLayout(m_shaderContainer->m_pInputLayout);
        renderer->m_pD3DContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
        renderer->m_pD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        renderer->m_pD3DContext->VSSetShader(m_shaderContainer->m_pVS, 0, 0);
        renderer->m_pD3DContext->PSSetShader(m_shaderContainer->m_pPS, 0, 0);
        renderer->m_pD3DContext->PSSetShaderResources(0, 1, &m_pColorMap);
        renderer->m_pD3DContext->PSSetSamplers(0, 1, &m_pColorMapSampler);

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
        void* dPtr;

        InMemInstance temp;

        for (int i = 0; i < m_instancen.size(); i++) {
            //if (!m_instancen[i]->isActiv)
            //    continue;
            XMMATRIX world = m_instancen[i]->transform->GetWorldMatrix();
            temp.mat = XMMatrixMultiply(world, projectionMatrix);
            temp.mat = XMMatrixTranspose(temp.mat);

            temp.rect = m_instancen[i]->rect;

            renderer->m_pD3DContext->Map(m_instanceBuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            dPtr = mappedResource.pData;
            memcpy(dPtr, &temp, sizeof(InMemInstance));
            renderer->m_pD3DContext->Unmap(m_instanceBuffer, 0);

            renderer->m_pD3DContext->VSSetConstantBuffers(0, 1, &m_instanceBuffer);

            renderer->m_pD3DContext->Draw(6, 0);
        }
    }

    static std::vector<std::string> getRequirements() {
        std::vector<std::string> toRet = std::vector<std::string>();
        return toRet;
    };
    static bool getIsLonly() { return false; }; 
    void setRequirements(std::vector <Component*> toSet) {
    }
    virtual void set(SerializedObject& from) {
        int size, temp0, temp1;
        from >> size;
        for (int i = 0; i < size; i++) {
            from >> temp0;
            from >> temp1;
            m_instancenIDs.push_back(std::pair<int,int>(temp1, temp0));
        }
        from >> m_shaderPath >> m_texturePath;
    };
    virtual void afterSet() {
        for (int i = 0; i < m_instancenIDs.size(); i++)
            m_instancen.push_back((InstancedTeilSprite::Instance*)ObjectStorrage::getInstance()->getById(m_instancenIDs[i].first)->findComponents<InstancedTeilSprite::Instance>()[m_instancenIDs[i].second]);
    }
    virtual SerializedObject serialize() {
        SerializedObject so;
        return serialize(so);
    };
    virtual SerializedObject serialize(SerializedObject& inTo) {
        inTo << m_texturePath << m_shaderPath;
        int size = m_instancenIDs.size();
        for (int i = size - 1; i > -1; i--) {
            inTo << m_instancenIDs[i].first;
            inTo << m_instancenIDs[i].second;
        }
        inTo << size;
        inTo << typeid(InstancedTeilSprite).name();
        return inTo;
    };

    void addInstance(Instance* instance) {
        m_instancen.push_back(instance);
        std::vector<InstancedTeilSprite::Instance*> v = instance->gameobject->findComponents<InstancedTeilSprite::Instance>();
        for (int i = 0; i < v.size(); i++) {
            if (v[i] == instance)
            {
                m_instancenIDs.push_back(std::pair<int, int>(instance->gameobject->getID(), i));
                break;
            }
        }
    }
    void remInstance(Instance* instance) {
        for (int i = 0; i < m_instancen.size(); i++)
            if (m_instancen[i] == instance) {
                m_instancen.erase(m_instancen.begin() + i);
                m_instancenIDs.erase(m_instancenIDs.begin() + i);
                break;
            }
    }

    std::string m_texturePath;
    std::string m_shaderPath;

private:
    std::vector<Instance*> m_instancen;
    std::vector<std::pair<int, int>> m_instancenIDs;

private:
    ID3D11Buffer* m_pVertexBuffer;
    ID3D11ShaderResourceView* m_pColorMap;
    ID3D11SamplerState* m_pColorMapSampler;
    ID3D11BlendState* m_pAlphaBlendState;
    ID3D11Buffer* m_instanceBuffer;
    RenderedShader* m_shaderContainer;
};
ComponentRegister<InstancedTeilSprite> InstancedTeilSprite::reg(typeid(InstancedTeilSprite).name());
ComponentRegister<InstancedTeilSprite::Instance> InstancedTeilSprite::Instance::reg(typeid(InstancedTeilSprite::Instance).name());
#endif