#include <renderer/common/Common_Renderer.h>
#include <renderer/ITexture.h>
#include <core/Image.h>
#include <core/Log.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <wrl/client.h>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#undef LoadImage

using namespace Microsoft::WRL;
using namespace DirectX;

namespace xpf::resources {
std::string_view dx11_shaders();
}

namespace xpf {

static void ThrowIfFailed(HRESULT hr, xpf::string_viewex str = "")
{
    if (hr == S_OK) return;
    xpf::stringex msg = "DX error: " + xpf::stringex::to_hex(hr, 8);
    msg.appendex(" - ").appendex(str);
    xpf::Log::error(msg);
    throw std::exception();
}

static std::string to_string(const ComPtr<ID3DBlob>& sp)
{
    if (sp == nullptr) return "";
    return std::string(reinterpret_cast<char*>(sp->GetBufferPointer()), sp->GetBufferSize());
}

class DirectXTexture : public ITexture
{
public:
    ComPtr<ID3D11Texture2D> m_spTexture;
    ComPtr<ID3D11ShaderResourceView> m_spTextureView;
    const textureid_t m_textureid;
    const uint32_t m_width;
    const uint32_t m_height;
    static inline textureid_t s_id = 0;

public:
    DirectXTexture(ComPtr<ID3D11Texture2D>&& sp, ComPtr<ID3D11ShaderResourceView>&& spView, uint32_t width, uint32_t height)
        : m_spTexture(std::move(sp))
        , m_spTextureView(std::move(spView))
        , m_textureid(++s_id)
        , m_width(width)
        , m_height(height)
    { }

    virtual textureid_t GetId() const override { return s_id; }
    virtual uint32_t GenerateMipMaps() override { return 1; }
    virtual uint32_t GetWidth() const override { return m_width; }
    virtual uint32_t GetHeight() const override { return m_height; }
};

class DirectXRenderer : public CommonRenderer
{
protected:
    HWND m_hWnd = NULL;
    D3D_DRIVER_TYPE                 m_driverType = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL               m_featureLevel = D3D_FEATURE_LEVEL_11_0;
    ComPtr<ID3D11Device1>           m_spDevice;
    ComPtr<ID3D11DeviceContext1>    m_spDeviceContext;
    ComPtr<IDXGISwapChain1>         m_spSwapChain;
    ComPtr<ID3D11RenderTargetView>  m_spRenderTargetView;
    ComPtr<ID3D11VertexShader>      m_spVertexShader;
    ComPtr<ID3D11PixelShader>       m_spPixelShader;
    ComPtr<ID3D11InputLayout>       m_spVertexLayout;
    ComPtr<ID3D11SamplerState>      m_spSamplerState;
    ComPtr<ID3D11Buffer>            m_spVertexConstantsBuffer;
    struct VertexConstants
    {
        m4_t modelViewProj;
    };

    ComPtr<ID3D11Buffer>            m_spPixelConstantsBuffer;
    struct PixelConstants
    {
        RenderCommandId commandId;
        float width;
        float height;
        CornerRadius cornerRadius;
        Thickness borderThickness;
        v4_t fillColor;
    };

    ComPtr<ID3D11BlendState1> m_spBlendStateBlend;
    uint32_t m_vsync_interval = 0;

public:
    DirectXRenderer() = default;

    virtual GLFWwindow* Initialize(RendererOptions&& optionsIn) override
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        if (CommonRenderer::Initialize(std::move(optionsIn)) == nullptr)
            return nullptr;

        m_hWnd = glfwGetWin32Window(m_pWindow);
        InitDevice();

        m_vsync_interval = m_options.enable_vsync ? 1 : 0;

        return m_pWindow;
    }

    virtual void Shutdown() override { Cleanup(); }
    virtual void Render() override
    {
        m_frame_count++;
        m_renderStats = {};

        // static float tt = 0;
        // tt += .01;
        // if (tt > 1.0)
        //     tt = 0;
        // FLOAT backgroundColor[4] = { tt, 0.2f, 0.6f, 1.0f };
        m_spDeviceContext->ClearRenderTargetView(m_spRenderTargetView.Get(), m_background_color.to_floats());

        RECT winRect;
        GetClientRect(m_hWnd, &winRect);
        D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)(winRect.right - winRect.left), (FLOAT)(winRect.bottom - winRect.top), 0.0f, 1.0f };
        m_spDeviceContext->RSSetViewports(1, &viewport);

        m_spDeviceContext->OMSetRenderTargets(1, m_spRenderTargetView.GetAddressOf(), nullptr);

        m_spDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_spDeviceContext->IASetInputLayout(m_spVertexLayout.Get());

        m_spDeviceContext->VSSetShader(m_spVertexShader.Get(), nullptr, 0);
        m_spDeviceContext->VSSetConstantBuffers(0, 1, m_spVertexConstantsBuffer.GetAddressOf());

        m_spDeviceContext->PSSetShader(m_spPixelShader.Get(), nullptr, 0);
        m_spDeviceContext->PSSetSamplers(0, 1, &m_spSamplerState);
        m_spDeviceContext->PSSetConstantBuffers(0, 1, m_spPixelConstantsBuffer.GetAddressOf());

        m4_t transform = m4_t::identity;
        std::vector<m4_t> transforms({transform});
        UpdateVertexConstantsBuffer(transform);

        {
            ComPtr<ID3D11ShaderResourceView> spEmptyTexture;

            D3D11_SUBRESOURCE_DATA vertexSubresourceData;
            D3D11_BUFFER_DESC vertexBufferDesc = {};
            vertexBufferDesc.Usage     = D3D11_USAGE_IMMUTABLE;
            vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            uint32_t stride = 8 * sizeof(float);
            uint32_t offset = 0;

            m_builder.Build().ForEachCommand([&](auto& spCommand)
            {
                if (spCommand->commandId == RenderCommandId::transform)
                {
                    const RenderTransformCommand& command = *static_cast<RenderTransformCommand*>(spCommand.get());
                    switch (command.type)
                    {
                        case RenderTransformCommand::MultiplyPush:
                            transform = transform * command.transform;
                            transforms.push_back(transform);
                            break;
                        case RenderTransformCommand::Push:
                            transform = command.transform;
                            transforms.push_back(transform);
                            break;
                        case RenderTransformCommand::Pop:
                            transforms.pop_back();
                            transform = transforms.back();
                            break;
                    }

                    UpdateVertexConstantsBuffer(transform);
                }
                else
                {
                    const RenderDrawCommand& command = *static_cast<RenderDrawCommand*>(spCommand.get());

                    vertexBufferDesc.ByteWidth = command.verts.size() * sizeof(float);
                    vertexSubresourceData.pSysMem = command.verts.data();

                    ComPtr<ID3D11Buffer> spVB;
                    ThrowIfFailed(m_spDevice->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &spVB));
                    m_spDeviceContext->IASetVertexBuffers(0, 1, spVB.GetAddressOf(), &stride, &offset);

                    if (command.spTexture != nullptr) {
                        const ITexture* pTexture = command.spTexture.get();
                        if (pCurrentTexture != pTexture) {
                            pCurrentTexture = pTexture;
                            m_spDeviceContext->PSSetShaderResources(0, 1, static_cast<const DirectXTexture*>(pTexture)->m_spTextureView.GetAddressOf());
                            m_renderStats.textureSwitches++;
                        }
                    } else if (pCurrentTexture != nullptr) {
                        pCurrentTexture = nullptr;
                        m_spDeviceContext->PSSetShaderResources(0, 1, spEmptyTexture.GetAddressOf());
                        m_renderStats.textureSwitches++;
                    }

                    UpdatePixelConstantBuffer(command);

                    m_spDeviceContext->Draw(command.count, 0);
                    m_renderStats.vertexCount += command.count;
                    m_renderStats.drawCount++;
                }
            });
        }

        ThrowIfFailed(m_spSwapChain->Present(m_vsync_interval, 0));
    }

    virtual void OnResize(int32_t width, int32_t height) override
    {
        CommonRenderer::OnResize(width, height);

        D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)(width), (FLOAT)(height), 0.0f, 1.0f };
        m_spDeviceContext->RSSetViewports(1, &viewport);

        m_spDeviceContext->OMSetRenderTargets(0, 0, 0);
        ThrowIfFailed(m_spSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0));

        m_spRenderTargetView = nullptr;
        ComPtr<ID3D11Texture2D> d3d11FrameBuffer;
        ThrowIfFailed(m_spSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &d3d11FrameBuffer));
        ThrowIfFailed(m_spDevice->CreateRenderTargetView(d3d11FrameBuffer.Get(), NULL, &m_spRenderTargetView));

        UpdateVertexConstantsBuffer(m4_t::identity);
    }

    // Builder methods
    std::shared_ptr<ITexture> CreateTexture(std::string_view filename) override
    {
        return CreateTexture(Image::LoadImage(filename));
    }

    virtual std::shared_ptr<ITexture> CreateTexture(const Image& image) override
    {
        D3D11_TEXTURE2D_DESC textureDesc = {};

        uint32_t bytesPerPixel = 4;
        switch (image.GetPixelFormat())
        {
            case PixelFormat::R8G8B8A8:
                textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                bytesPerPixel = 4;
                break;
            case PixelFormat::GrayScale:
                textureDesc.Format = DXGI_FORMAT_R8_UNORM;
                bytesPerPixel = 1;
                break;
            case PixelFormat::R4G4B4A4:
                textureDesc.Format = DXGI_FORMAT_B4G4R4A4_UNORM;
                bytesPerPixel = 2;
                break;
            default:
                return nullptr;
        }

        // Create Texture
        textureDesc.Width              = image.GetWidth();
        textureDesc.Height             = image.GetHeight();
        textureDesc.MipLevels          = 1;
        textureDesc.ArraySize          = 1;
        //textureDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        textureDesc.SampleDesc.Count   = 1;
        textureDesc.Usage              = D3D11_USAGE_IMMUTABLE;
        textureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
        textureSubresourceData.pSysMem = image.GetData().data();
        textureSubresourceData.SysMemPitch = bytesPerPixel * image.GetWidth();

        ComPtr<ID3D11Texture2D> spTexture;
        ThrowIfFailed(m_spDevice->CreateTexture2D(&textureDesc, &textureSubresourceData, &spTexture));

        ComPtr<ID3D11ShaderResourceView> spTextureView;
        m_spDevice->CreateShaderResourceView(spTexture.Get(), nullptr, &spTextureView);

        return std::make_shared<DirectXTexture>(
            std::move(spTexture),
            std::move(spTextureView),
            image.GetWidth(),
            image.GetHeight());
    }

protected:
    void InitDevice()
    {
        // Create D3D11 Device and Context
        {
            D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
            UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
            #if defined(DEBUG_BUILD)
            creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
            #endif

            ComPtr<ID3D11Device> baseDevice;
            ComPtr<ID3D11DeviceContext> baseDeviceContext;
            ThrowIfFailed(D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE,
                                                0, creationFlags, 
                                                featureLevels, ARRAYSIZE(featureLevels),
                                                D3D11_SDK_VERSION, &baseDevice,
                                                0, &baseDeviceContext));
            
            // Get 1.1 interface of D3D11 Device and Context
            ThrowIfFailed(baseDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)&m_spDevice));
            ThrowIfFailed(baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&m_spDeviceContext));
        }

        {
#ifdef DEBUG_BUILD
            // Set up debug layer to break on D3D11 errors
            ComPtr<ID3D11Debug> d3dDebug;
            m_spDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
            if (d3dDebug)
            {
                ComPtr<ID3D11InfoQueue> d3dInfoQueue;
                if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), &d3dInfoQueue)))
                {
                    d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
                    d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
                }
            }
#endif
        }

        // Create Swap Chain
        {
            // Get DXGI Factory (needed to create Swap Chain)
            ComPtr<IDXGIFactory2> dxgiFactory;
            {
                ComPtr<IDXGIDevice1> dxgiDevice;
                ThrowIfFailed(m_spDevice->QueryInterface(__uuidof(IDXGIDevice1), &dxgiDevice));

                ComPtr<IDXGIAdapter> dxgiAdapter;
                ThrowIfFailed(dxgiDevice->GetAdapter(&dxgiAdapter));

                DXGI_ADAPTER_DESC adapterDesc;
                dxgiAdapter->GetDesc(&adapterDesc);

                OutputDebugStringA("Graphics Device: ");
                OutputDebugStringW(adapterDesc.Description);

                ThrowIfFailed(dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &dxgiFactory));
            }

            DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};
            d3d11SwapChainDesc.Width = m_options.width; // use window width
            d3d11SwapChainDesc.Height = m_options.height; // use window height
            d3d11SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            d3d11SwapChainDesc.SampleDesc.Count = 1;
            d3d11SwapChainDesc.SampleDesc.Quality = 0;
            d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            d3d11SwapChainDesc.BufferCount = 2;
            d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
            d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            d3d11SwapChainDesc.Flags = 0;

            ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(m_spDevice.Get(), m_hWnd, &d3d11SwapChainDesc, 0, 0, &m_spSwapChain));
        }

        // Create Framebuffer Render Target
        {
            ComPtr<ID3D11Texture2D> d3d11FrameBuffer;
            ThrowIfFailed(m_spSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &d3d11FrameBuffer));
            ThrowIfFailed(m_spDevice->CreateRenderTargetView(d3d11FrameBuffer.Get(), nullptr, &m_spRenderTargetView));
        }

        // Create shaders
        ComPtr<ID3DBlob> spVSBlob;
        {
            spVSBlob = CompileShader("vs.hlsl", xpf::resources::dx11_shaders(), "vs_main", "vs_5_0");
            ThrowIfFailed(m_spDevice->CreateVertexShader(spVSBlob->GetBufferPointer(), spVSBlob->GetBufferSize(), nullptr, &m_spVertexShader));

            ComPtr<ID3DBlob> spPSBlob = CompileShader("ps.hlsl", xpf::resources::dx11_shaders(), "ps_main", "ps_5_0");
            ThrowIfFailed(m_spDevice->CreatePixelShader(spPSBlob->GetBufferPointer(), spPSBlob->GetBufferSize(), nullptr, &m_spPixelShader));
        }

        // Create Input Layout
        {
            D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
            {
                { "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
            };

            ThrowIfFailed(m_spDevice->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), spVSBlob->GetBufferPointer(), spVSBlob->GetBufferSize(), &m_spVertexLayout));
        }

        // Create Vertex Constant Buffer
        {
            D3D11_BUFFER_DESC constantBufferDesc = {};
            // ByteWidth must be a multiple of 16, per the docs
            constantBufferDesc.ByteWidth      = sizeof(VertexConstants) + 0xf & 0xfffffff0;
            constantBufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
            constantBufferDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
            constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            ThrowIfFailed(m_spDevice->CreateBuffer(&constantBufferDesc, nullptr, &m_spVertexConstantsBuffer));
            UpdateVertexConstantsBuffer(m4_t::identity);
        }

        // Create Pixel Constant Buffer
        {
            D3D11_BUFFER_DESC constantBufferDesc = {};
            // ByteWidth must be a multiple of 16, per the docs
            constantBufferDesc.ByteWidth      = sizeof(PixelConstants) + 0xf & 0xfffffff0;
            constantBufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
            constantBufferDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
            constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            ThrowIfFailed(m_spDevice->CreateBuffer(&constantBufferDesc, nullptr, &m_spPixelConstantsBuffer));
        }

        m_spSamplerState = CreateSamplerState();

        {
            D3D11_BLEND_DESC1 blendState;
            ZeroMemory(&blendState, sizeof(blendState));
            blendState.RenderTarget[0].BlendEnable = TRUE;
            blendState.RenderTarget[0].RenderTargetWriteMask = 0x0f;
            blendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            blendState.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            blendState.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            blendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            blendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
            blendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
            m_spDevice->CreateBlendState1(&blendState, &m_spBlendStateBlend);

            float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            UINT sampleMask = 0xffffffff;
            m_spDeviceContext->OMSetBlendState(m_spBlendStateBlend.Get(), blendFactor, sampleMask);
        }
    }

    void UpdateVertexConstantsBuffer(const m4_t& transform)
    {
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        m_spDeviceContext->Map(m_spVertexConstantsBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        VertexConstants* pconstants = (VertexConstants*)(mappedSubresource.pData);
        pconstants->modelViewProj = (m_projection_matrix * transform).transpose();
        m_spDeviceContext->Unmap(m_spVertexConstantsBuffer.Get(), 0);
    }

    void UpdatePixelConstantBuffer(const RenderDrawCommand& cmd)
    {
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        m_spDeviceContext->Map(m_spPixelConstantsBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        PixelConstants& constants = *(PixelConstants*)(mappedSubresource.pData);
        constants.commandId = cmd.commandId;

        if (cmd.commandId == RenderCommandId::rounded_rectangle ||
            cmd.commandId == RenderCommandId::rounded_rectangle_with_border ||
            cmd.commandId == RenderCommandId::rounded_rectangle_with_border_dots)
        {
            const RenderRoundedRectangleCommand& cmd2 = (const RenderRoundedRectangleCommand&)(cmd);
            constants.width = cmd2.size.w;
            constants.height = cmd2.size.h;
            constants.cornerRadius = cmd2.cornerRadius;
            if (cmd.commandId == RenderCommandId::rounded_rectangle_with_border)
            {
                constants.borderThickness = cmd2.borderThickness;
                constants.fillColor = cmd2.fillColor;
            }
        }

        m_spDeviceContext->Unmap(m_spPixelConstantsBuffer.Get(), 0);
    }

    ComPtr<ID3D11SamplerState> CreateSamplerState()
    {
        // Create Sampler State
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.BorderColor[0] = 1.0f;
        samplerDesc.BorderColor[1] = 1.0f;
        samplerDesc.BorderColor[2] = 1.0f;
        samplerDesc.BorderColor[3] = 1.0f;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

        ComPtr<ID3D11SamplerState> spSamplerState;
        ThrowIfFailed(m_spDevice->CreateSamplerState(&samplerDesc, &spSamplerState));
        return spSamplerState;
    }

    void Cleanup()
    {
        //if (m_spImmediateContext) m_spImmediateContext->ClearState();
    }

    static ComPtr<ID3DBlob> CompileShader(std::string_view filename, std::string_view src, std::string_view entryPoint, std::string_view target)
    {
        ComPtr<ID3DBlob> shaderBlob;
        ComPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3DCompile(
            src.data(), src.length(),
            /*pSourceName:*/ filename.data(),
            /*pDefines - D3D_SHADER_MACRO:*/ nullptr,
            /*pInclude - ID3DInclude:*/ nullptr,
            /*pEntrypoint:*/ entryPoint.data(),
            /*pTarget:*/ target.data(), // https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/specifying-compiler-targets
            /*Flags1:*/ D3DCOMPILE_DEBUG,  // https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/d3dcompile-constants
            /*Flags2:*/0,
            /*ppCode:*/&shaderBlob,
            /*ppErrorMsgs:*/&errorBlob
            );
        ThrowIfFailed(hr, to_string(errorBlob));

        return shaderBlob;
    }

    // https://github.com/walbourn/directx-sdk-samples/blob/main/Direct3D11Tutorials/Tutorial03/Tutorial03.cpp
};

std::unique_ptr<IRenderer> create_directx_renderer()
{
    return std::make_unique<DirectXRenderer>();
}

} // xpf