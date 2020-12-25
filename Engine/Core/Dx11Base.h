#ifndef _DX11BASE_H_
#define _DX11BASE_H_

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <dxerr.h>
#include <xnamath.h>
#include <dinput.h>
#include <xinput.h>

#include <iostream>

#include <Serializer_Include.h>
#include <Logger.h>

#include "Button.h"
#include "Kamera.h"

class Dx11Base
{
public:
    Dx11Base() { m_buttons = std::vector<Button*>(); }
    virtual ~Dx11Base() {}

    bool Initialize(HWND hWnd, HINSTANCE hInst) {
        LTRACE("DX11BASE::INITIALIZE::STARTING");
        m_hWnd = hWnd;
        m_hInst = hInst;

        RECT rc;
        ::GetClientRect(hWnd, &rc);
        UINT nWidth = rc.right - rc.left;
        UINT nHeight = rc.bottom - rc.top;

        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        ::ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
        swapChainDesc.BufferCount = 1;
        swapChainDesc.BufferDesc.Width = nWidth;
        swapChainDesc.BufferDesc.Height = nHeight;
        swapChainDesc.BufferDesc.Format = m_colorFormat;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = m_refreshRateN;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = m_refreshRateD;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = (HWND)hWnd;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Windowed = m_windowed;

        D3D_FEATURE_LEVEL featureLevel;
        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };
        UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE, D3D_DRIVER_TYPE_SOFTWARE
        };
        UINT numDriverTypes = ARRAYSIZE(driverTypes);

        UINT flags = 0;
#ifdef _DEBUG
        flags = D3D11_CREATE_DEVICE_DEBUG;
#endif

        HRESULT hr = ::D3D11CreateDeviceAndSwapChain(
            NULL,
            D3D_DRIVER_TYPE_HARDWARE,
            NULL,
            flags,
            featureLevels,
            numFeatureLevels,
            D3D11_SDK_VERSION,
            &swapChainDesc,
            &m_pSwapChain,
            &m_pD3DDevice,
            &featureLevel,
            &m_pD3DContext
        );

        if (FAILED(hr)) {
            LERROR("DX11BASE::INITIALIZE::create Device and Swapchain failed");
            ::MessageBox(hWnd, TEXT("A DX11 Video Card is Required"), TEXT("ERROR"), MB_OK);
            return false;
        }

        ID3D11Texture2D* pBackBuffer;
        hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
        if (FAILED(hr)) {
            LERROR("DX11BASE::INITIALIZE::Unable to get back buffer");
            ::MessageBox(hWnd, TEXT("Unable to get back buffer"), TEXT("ERROR"), MB_OK);
            return false;
        }

        hr = m_pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pD3DRenderTargetView);

        if (pBackBuffer != NULL)
            pBackBuffer->Release();

        if (FAILED(hr)) {
            LERROR("DX11BASE::INITIALIZE::Unable to create render target view");
            ::MessageBox(hWnd, TEXT("Unable to create render target view"), TEXT("ERROR"), MB_OK);
            return false;
        }

        m_pD3DContext->OMSetRenderTargets(1, &m_pD3DRenderTargetView, NULL);

        D3D11_VIEWPORT viewPort;
        viewPort.Width = (float)nWidth;
        viewPort.Height = (float)nHeight;
        viewPort.MinDepth = 0.0f;
        viewPort.MaxDepth = 1.0f;
        viewPort.TopLeftX = 0;
        viewPort.TopLeftY = 0;
        m_pD3DContext->RSSetViewports(1, &viewPort);

        return LoadContent();
    }

    void Terminate() {
        LTRACE("DX11BASE::TERMINATIN::STARTED");
        UnloadContent();

        if (m_pD3DRenderTargetView != NULL)
            m_pD3DRenderTargetView->Release();
        m_pD3DRenderTargetView = NULL;
        if (m_pSwapChain != NULL)
            m_pSwapChain->Release();
        m_pSwapChain = NULL;
        if (m_pD3DContext != NULL)
            m_pD3DContext->Release();
        m_pD3DContext = NULL;
        if (m_pD3DDevice != NULL)
            m_pD3DDevice->Release();
        m_pD3DDevice = NULL;
        LTRACE("DX11BASE::TERMINATIN::FINISCHED");
    }

    bool CompileShader(LPCWSTR szFilePath, LPCSTR szFunc, LPCSTR szShaderModel, ID3DBlob** buffer) {
        LTRACE("DX11BASE::COMPILINGSCHADER::STARTED");

        DWORD flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        flags |= D3DCOMPILE_DEBUG;
#endif

        HRESULT hr;
        ID3DBlob* errBuffer = 0;
        hr = ::D3DX11CompileFromFile(
            szFilePath, 0, 0, szFunc, szShaderModel,
            flags, 0, 0, buffer, &errBuffer, 0);

        if (FAILED(hr)) {
            if (errBuffer != NULL) {
                ::OutputDebugStringA((char*)errBuffer->GetBufferPointer());
                errBuffer->Release();
            }
            return false;
        }

        if (errBuffer != NULL)
            errBuffer->Release();
        LTRACE("DX11BASE::COMPILINGSCHADER::FINISCHED");
        return true;
    }

    virtual void clear() {
        m_buttons.clear();
    }

    void Quit() {
        LTRACE("DX11BASE::QUIT::REQUESTED");
        ::PostQuitMessage(0);
    }

    virtual bool LoadContent() = 0;
    virtual void UnloadContent() = 0;

    virtual void addButton(Button* button)
    {
        for (int i = 0; i < m_buttons.size(); i++) {
            if (m_buttons[i] == button)
                return;
        }
        m_buttons.push_back(button);
        m_buttonGameObjectIDs.push_back(button->gameobject->getID());
    }
    virtual void remButton(Button* button)
    {
        for (int i = 0; i < m_buttons.size(); i++) {
            if (m_buttons[i] == button) {
                m_buttons.erase(m_buttons.begin() + i);
                m_buttonGameObjectIDs.erase(m_buttonGameObjectIDs.begin() + i);
                return;
            }
        }
    }

    virtual void checkButtons(bool down, bool up) {
        if (m_buttons.size() == 0)
            return;

        XMMATRIX toScreenSpaceMatrix = m_currentCamera->getProjectionMatrix();

        POINT mPoint;
        GetCursorPos(&mPoint);
        ScreenToClient(m_hWnd, &mPoint);

        UINT count = 1;
        D3D11_VIEWPORT vPort;
        m_pD3DContext->RSGetViewports(&count, &vPort);

        for (Button* b : m_buttons)
            b->checkKlicked(mPoint.x, mPoint.y, toScreenSpaceMatrix, vPort, down, up);
    }
    virtual void Render() = 0;

    void setKamera(Kamera* mainKamera) {
        m_currentCamera = mainKamera;
        m_currentCameraID = mainKamera->gameobject->getID();
    }

    HWND m_hWnd;
    HINSTANCE m_hInst;
    ID3D11Device* m_pD3DDevice;
    ID3D11DeviceContext* m_pD3DContext;
    ID3D11RenderTargetView*	m_pD3DRenderTargetView;
    IDXGISwapChain* m_pSwapChain;

    DXGI_FORMAT m_colorFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    int m_refreshRateN = 60;
    int m_refreshRateD = 1;
    bool m_windowed = true;


protected:
    Kamera* m_currentCamera;
    int m_currentCameraID;
    std::vector<Button*> m_buttons;
    std::vector<int> m_buttonGameObjectIDs;

};


#endif