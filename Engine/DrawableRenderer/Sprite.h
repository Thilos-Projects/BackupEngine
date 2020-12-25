#ifndef _Sprite_H_
#define _Sprite_H_

#include "DrawableRenderer.h"
#include "../other/Transform.h"

#include <Serializer_Include.h>

class Sprite : public Drawable
{
private:
    static ComponentRegister<Sprite> reg;
public:

    struct SpriteVertex
    {
        XMFLOAT3 pos;
        XMFLOAT2 tex0;
    };

    Transform* transform;

    Sprite()
    {
        transform = NULL;
        m_shaderPath = "";
        m_texturePath = "";
        m_pVertexBuffer = NULL;
        m_pColorMap = NULL;
        m_pColorMapSampler = NULL;
        m_pAlphaBlendState = NULL;
        m_localTransformMatrixe = NULL;
        m_shaderContainer = NULL;
    }
    ~Sprite()
    {
    }

    virtual bool registerShader(DrawableRenderer* renderer)
    {
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

        SpriteVertex vertices[] =
        {
            { XMFLOAT3(halfWidth,  halfHeight, 1.0f), XMFLOAT2(1.0f, 0.0f) },
            { XMFLOAT3(halfWidth, -halfHeight, 1.0f), XMFLOAT2(1.0f, 1.0f) },
            { XMFLOAT3(-halfWidth, -halfHeight, 1.0f), XMFLOAT2(0.0f, 1.0f) },

            { XMFLOAT3(-halfWidth, -halfHeight, 1.0f), XMFLOAT2(0.0f, 1.0f) },
            { XMFLOAT3(-halfWidth,  halfHeight, 1.0f), XMFLOAT2(0.0f, 0.0f) },
            { XMFLOAT3(halfWidth,  halfHeight, 1.0f), XMFLOAT2(1.0f, 0.0f) },
        };

        D3D11_BUFFER_DESC vertexDesc;
        ::ZeroMemory(&vertexDesc, sizeof(vertexDesc));
        vertexDesc.Usage = D3D11_USAGE_DEFAULT;
        vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexDesc.ByteWidth = sizeof(SpriteVertex) * 6;

        D3D11_SUBRESOURCE_DATA resourceData;
        ZeroMemory(&resourceData, sizeof(resourceData));
        resourceData.pSysMem = vertices;

        hr = renderer->m_pD3DDevice->CreateBuffer(&vertexDesc, &resourceData, &m_pVertexBuffer);
        if (FAILED(hr)) {
            return false;
        }

        D3D11_BUFFER_DESC constDesc;
        ::ZeroMemory(&constDesc, sizeof(constDesc));
        constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constDesc.ByteWidth = sizeof(XMMATRIX);
        constDesc.Usage = D3D11_USAGE_DEFAULT;
        hr = renderer->m_pD3DDevice->CreateBuffer(&constDesc, 0, &m_localTransformMatrixe);
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
        if (m_localTransformMatrixe)
            m_localTransformMatrixe->Release();
        m_localTransformMatrixe = NULL;
    }

    virtual void render(DrawableRenderer* renderer, XMMATRIX projectionMatrix)
    {

        UINT stride = sizeof(SpriteVertex);
        UINT offset = 0;

        renderer->m_pD3DContext->IASetInputLayout(m_shaderContainer->m_pInputLayout);
        renderer->m_pD3DContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
        renderer->m_pD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        renderer->m_pD3DContext->VSSetShader(m_shaderContainer->m_pVS, 0, 0);
        renderer->m_pD3DContext->PSSetShader(m_shaderContainer->m_pPS, 0, 0);
        renderer->m_pD3DContext->PSSetShaderResources(0, 1, &m_pColorMap);
        renderer->m_pD3DContext->PSSetSamplers(0, 1, &m_pColorMapSampler);

        XMMATRIX world = transform->GetWorldMatrix();
        XMMATRIX mvp = XMMatrixMultiply(world, projectionMatrix);
        mvp = XMMatrixTranspose(mvp);

        renderer->m_pD3DContext->UpdateSubresource(m_localTransformMatrixe, 0, 0, &mvp, 0, 0);
        renderer->m_pD3DContext->VSSetConstantBuffers(0, 1, &m_localTransformMatrixe);

        renderer->m_pD3DContext->Draw(6, 0);
    }

    static std::vector<std::string> getRequirements() {
        std::vector<std::string> outPut = std::vector<std::string > ();
        outPut.push_back(typeid(Transform).name());
        return outPut;
    };
    static bool getIsLonly() { return false; }; 
    void setRequirements(std::vector <Component*> toSet) {
        transform = (Transform*)toSet[0];
    }
    virtual void set(SerializedObject& from) {
        from >> m_shaderPath >> m_texturePath;
    };
    virtual void afterSet() {
    }
    virtual SerializedObject serialize() {
        SerializedObject so;
        return serialize(so);
    };
    virtual SerializedObject serialize(SerializedObject& inTo) {
        inTo << m_texturePath << m_shaderPath;
        inTo << typeid(Sprite).name();
        return inTo;
    };

    std::string m_texturePath;
    std::string m_shaderPath;

private:
    ID3D11Buffer* m_pVertexBuffer;
    ID3D11ShaderResourceView* m_pColorMap;
    ID3D11SamplerState* m_pColorMapSampler;
    ID3D11BlendState* m_pAlphaBlendState;
    ID3D11Buffer* m_localTransformMatrixe;
    RenderedShader* m_shaderContainer;
};
ComponentRegister<Sprite> Sprite::reg(typeid(Sprite).name());
#endif