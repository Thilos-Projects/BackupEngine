#ifndef _DRAWABLERENDERER_H_
#define _DRAWABLERENDERER_H_

#include <D3D11.h>
#include <map>
#include <vector>

#include <Serializer.h>

#include "../Core/Dx11Base.h"

class DrawableRenderer; 

class Drawable : public Component {
public:
    virtual bool registerShader(DrawableRenderer* renderer) = 0;
    virtual bool load(DrawableRenderer* renderer) = 0;
    virtual void unload(DrawableRenderer* renderer) = 0;

    virtual void render(DrawableRenderer* renderer, XMMATRIX projectionMatrix) = 0;

    virtual void setRequirements(std::vector <Component*> toSet) = 0;
    virtual void set(SerializedObject& from) = 0;
    virtual void afterSet() = 0;
    virtual SerializedObject serialize() = 0;
    virtual SerializedObject serialize(SerializedObject& inTo) = 0;
};

struct RenderedShader {
    ID3D11VertexShader* m_pVS;
    ID3D11PixelShader* m_pPS;
    ID3D11InputLayout* m_pInputLayout;
};

class DrawableRenderer : public Dx11Base , public Component
{
private:
    static ComponentRegister<DrawableRenderer> reg;
public:
    DrawableRenderer()
    {
        m_allShaders = std::map<std::string, RenderedShader>();
        m_allDrawable = std::vector<Drawable*>();
        m_allDrawableIDs = std::vector<std::pair<int, int>>();
    }
    ~DrawableRenderer() {}

    RenderedShader* addShaderByPath(std::string path, D3D11_INPUT_ELEMENT_DESC* inputShaderInputLayout, UINT numLayoutElements)
    {
        if (m_allShaders.find(path) != m_allShaders.end())
            return &m_allShaders[path];

        RenderedShader shaderContainer;

        ID3DBlob* pVSBuffer = NULL;
        std::wstring sPath = std::wstring(path.begin(), path.end());
        bool res = CompileShader(sPath.c_str(), "VS_Main", "vs_5_0", &pVSBuffer);
        if (res == false) {
            ::MessageBox(m_hWnd, L"Unable to load vertex shader", L"ERROR", MB_OK);
            return NULL;
        }

        HRESULT hr;
        hr = m_pD3DDevice->CreateVertexShader(
            pVSBuffer->GetBufferPointer(),
            pVSBuffer->GetBufferSize(),
            0, &shaderContainer.m_pVS);
        if (FAILED(hr)) {
            if (pVSBuffer)
                pVSBuffer->Release();
            ::MessageBox(m_hWnd, L"Unable to create vertex shader", L"ERROR", MB_OK);
            return NULL;
        }

        hr = m_pD3DDevice->CreateInputLayout(
            inputShaderInputLayout, numLayoutElements,
            pVSBuffer->GetBufferPointer(),
            pVSBuffer->GetBufferSize(),
            &shaderContainer.m_pInputLayout);
        if (FAILED(hr)) {
            ::MessageBox(m_hWnd, L"Unable to create Input layout", L"ERROR", MB_OK);
            return NULL;
        }

        pVSBuffer->Release();
        pVSBuffer = NULL;

        ID3DBlob* pPSBuffer = NULL;
        res = CompileShader(sPath.c_str(), "PS_Main", "ps_5_0", &pPSBuffer);
        if (res == false) {
            ::MessageBox(m_hWnd, L"Unable to load pixel shader", L"ERROR", MB_OK);
            return NULL;
        }

        hr = m_pD3DDevice->CreatePixelShader(
            pPSBuffer->GetBufferPointer(),
            pPSBuffer->GetBufferSize(),
            0, &shaderContainer.m_pPS);
        if (FAILED(hr)) {
            ::MessageBox(m_hWnd, L"Unable to create pixel shader", L"ERROR", MB_OK);
            return NULL;
        }

        pPSBuffer->Release();
        pPSBuffer = NULL;

        m_allShaders[path] = shaderContainer;
        return &m_allShaders[path];
    }
    RenderedShader* getShaderByPath(std::string path)
    {
        if (m_allShaders.find(path) != m_allShaders.end())
            return &m_allShaders[path];
        return NULL;
    }

    void addDrawable(Drawable* drawable)
    {
        for (int i = 0; i < m_allDrawable.size(); i++) {
            if (m_allDrawable[i] == drawable)
                return;
        }
        m_allDrawable.push_back(drawable);
        std::vector<Drawable*> suchergebnis = drawable->gameobject->findComponents<Drawable>();
        for (int i = 0; i < suchergebnis.size(); i++) {
            if (suchergebnis[i] == drawable)
                m_allDrawableIDs.push_back(std::pair<int, int>(drawable->gameobject->getID(), i));
        }
    }
    void remDrawable(Drawable* drawable)
    {
        for (int i = 0; i < m_allDrawable.size(); i++) {
            if (m_allDrawable[i] == drawable) {
                m_allDrawable.erase(m_allDrawable.begin() + i);
                return;
            }
        }
    }

    virtual bool LoadContent()
    {
        for (int i = 0; i < m_allDrawable.size(); i++) {
            if (!m_allDrawable[i]->registerShader(this))return false;
        }
        for (int i = 0; i < m_allDrawable.size(); i++) {
            if (!m_allDrawable[i]->load(this))return false;
        }

        return true;
    }
    virtual void UnloadContent()
    {
        for (int i = 0; i < m_allDrawable.size(); i++) {
            m_allDrawable[i]->unload(this);
        }
        m_allDrawable.clear();
        for (std::map<std::string, RenderedShader>::iterator it = m_allShaders.begin(); it != m_allShaders.end(); ++it) {
            if (it->second.m_pPS)
                it->second.m_pPS->Release();
            if (it->second.m_pVS)
                it->second.m_pVS->Release();
            if (it->second.m_pInputLayout)
                it->second.m_pInputLayout->Release();
        }
        m_allShaders.clear();
    }

    virtual void Render()
    {
        if (m_pD3DContext == NULL)
            return;

        m_pD3DContext->ClearRenderTargetView(m_pD3DRenderTargetView, m_BackgroundColor);

        for (int i = 0; i < m_allDrawable.size(); i++) {
            m_allDrawable[i]->render(this, m_currentCamera->getProjectionMatrix());
        }

        m_pSwapChain->Present(0, 0);
    }

    virtual void clear() {
        UnloadContent();
        Dx11Base::clear();
    }

    static std::vector<std::string> getRequirements() {
        return std::vector<std::string>();
    };
    static bool getIsLonly() { return true; }; 
    void setRequirements(std::vector <Component*> toSet) {
    }
    virtual void set(SerializedObject& from) {
        int size;
        from >> size;
        int temp0;
        int temp1;
        for (int i = 0; i < size; i++) {
            from >> temp0;
            from >> temp1;
            m_allDrawableIDs.push_back(std::pair<int,int>(temp1,temp0));
        }
        from >> size;
        for (int i = 0; i < size; i++) {
            from >> temp0;
            m_buttonGameObjectIDs.push_back(temp0);
        }
        from >> m_BackgroundColor[3] >> m_BackgroundColor[2] >> m_BackgroundColor[1] >> m_BackgroundColor[0] >> m_windowed >> m_refreshRateD >> m_refreshRateN >> m_colorFormat >> m_currentCameraID;
    };
    virtual void afterSet() {
        for (int i = 0; i < m_buttonGameObjectIDs.size(); i++) {
            m_buttons.push_back((Button*)ObjectStorrage::getInstance()->getById(m_buttonGameObjectIDs[i])->findComponents<Button>()[0]);
        }
        for (int i = 0; i < m_allDrawableIDs.size(); i++) {
            m_allDrawable.push_back((Drawable*)ObjectStorrage::getInstance()->getById(m_allDrawableIDs[i].first)->findComponents<Drawable>()[m_allDrawableIDs[i].second]);
        }
        m_currentCamera = ObjectStorrage::getInstance()->getById(m_currentCameraID)->findComponents<Kamera>()[0];
    }
    virtual SerializedObject serialize() {
        SerializedObject so;
        return serialize(so);
    };
    virtual SerializedObject serialize(SerializedObject& inTo) {
        inTo << m_currentCameraID << m_colorFormat << m_refreshRateN << m_refreshRateD << m_windowed << m_BackgroundColor[0] << m_BackgroundColor[1] << m_BackgroundColor[2] << m_BackgroundColor[3];
        int size = m_buttons.size();
        for (int i = size - 1; i > -1; i--)
            inTo << m_buttonGameObjectIDs[i];
        inTo << size; 
        size = m_allDrawableIDs.size();
        for (int i = size - 1; i > -1; i--) {
            inTo << m_allDrawableIDs[i].first;
            inTo << m_allDrawableIDs[i].second;
        }
        inTo << size;
        inTo << typeid(DrawableRenderer).name();
        return inTo;
    };

public:
    float m_BackgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

protected:
    std::map<std::string, RenderedShader> m_allShaders;
    std::vector<Drawable*> m_allDrawable;
    std::vector<std::pair<int,int>> m_allDrawableIDs;
};
ComponentRegister<DrawableRenderer> DrawableRenderer::reg(typeid(DrawableRenderer).name());

#endif