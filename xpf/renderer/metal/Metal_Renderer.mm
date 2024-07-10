#include <common/Common_Renderer.h>
#include <core/Color.h>
#include <core/Image.h>
#include <core/Log.h>
#include <math/m4_t.h>
#include <core/Thickness.h>
#include <renderer/ITexture.h>
#include <core/Time.h>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <string>
#include <memory>
#include <simd/simd.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import "Metal.hpp"
#include <core/Convert_Apple.h>
#include <core/TaskQueue.h>

// #define NS_PRIVATE_IMPLEMENTATION
// #define MTL_PRIVATE_IMPLEMENTATION
// #define MTK_PRIVATE_IMPLEMENTATION
// #define CA_PRIVATE_IMPLEMENTATION
// #include "Metal.hpp"
// #include <AppKit/AppKit.hpp>
// #include <MetalKit/MetalKit.hpp>

namespace xpf::resources {
std::string_view xpf_metal(); // implemented in generated xpf_resources.cpp
std::vector<byte_t> xpf_metallib();
}

namespace xpf {

std::shared_ptr<ITexture> Metal_CreateTexture(id<MTLDevice> device, const Image& image);
id<MTLTexture> GetMTLTexture(const ITexture* pTexture);

std::shared_ptr<IBuffer> Metal_CreateBuffer(id<MTLDevice> device, const byte_t* pbyte, size_t size);
id<MTLBuffer> GetMTLBuffer(const IBuffer* pBuffer);

class MetalRenderer : public CommonRenderer
{
protected:
    // https://metashapes.com/blog/opengl-metal-projection-matrix-problem/
    static inline m4_t m_metal_adjust_matrix = m4_t(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 0.5f, 0.5f,
            0, 0, 0, 1);

    NSWindow* m_nswindow;

    id<MTLDevice> m_gpu;
    id<MTLCommandQueue> m_queue;
    CAMetalLayer* m_swapchain;
    id<MTLTexture> m_screenTexture;
    bool m_antialiasing_enabled = false;

    id<MTLLibrary> m_metalLibrary;
    id<MTLRenderPipelineState> m_metalRenderPSO;

    MTLClearColor m_metal_background_color;

    id<MTLSamplerState> m_linearSampler;
    id<MTLSamplerState> m_nearestSampler;

public:
    MetalRenderer() = default;

    virtual GLFWwindow* Initialize(RendererOptions&& optionsIn) override
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        if (CommonRenderer::Initialize(std::move(optionsIn)) == nullptr)
            return nullptr;

        if (m_options.is_projection_ortho2d) {
            m_projection_matrix = m_metal_adjust_matrix * m_projection_matrix;
        }

        m_gpu = MTLCreateSystemDefaultDevice();
        m_swapchain = [CAMetalLayer layer];
        m_swapchain.pixelFormat = MTLPixelFormatRGBA8Unorm;
        m_swapchain.device = m_gpu;
        m_swapchain.drawableSize = CGSizeMake(m_options.width, m_options.height);
        m_swapchain.opaque = YES;
        m_swapchain.displaySyncEnabled = m_options.enable_vsync ? YES : NO;

        if (m_options.metal_transactional_rendering)
        {
            // https://developer.apple.com/documentation/quartzcore/cametallayer/1478157-presentswithtransaction?language=objc
            m_swapchain.presentsWithTransaction = YES;
        }

        if (m_options.show_stats)
        {
            m_swapchain.developerHUDProperties = @{
                @"mode": @"default",
                // @"logging": @"default"
            };
        }

        m_nswindow = glfwGetCocoaWindow(m_pWindow);
        m_nswindow.contentView.layer = m_swapchain;
        m_nswindow.contentView.wantsLayer = YES;

        m_metal_background_color = MTLClearColorMake(
            m_options.background_color.rf(),
            m_options.background_color.gf(),
            m_options.background_color.bf(),
            m_options.background_color.af());

        CreateRenderPipeline();
        CreateTextureSamplers();
        m_queue = [m_gpu newCommandQueue];

        m_options.capabilities |= RendererCapability::ShaderInstancedRenderedText;

        return m_pWindow;
    }

    virtual void Shutdown() override {}

    virtual void OnResize(int32_t width, int32_t height) override
    {
        CommonRenderer::OnResize(width, height);
 
        if (m_options.is_projection_ortho2d) {
            // CommonRenderer::OnResize updates the projection matrix, we need to fix it here once again
            m_projection_matrix = m_metal_adjust_matrix * m_projection_matrix;
        }

        m_swapchain.drawableSize = CGSizeMake(width, height);
    }

    void CreateRenderPipeline()
    {
        NSError* error = nullptr;
        m_metalLibrary = [m_gpu newLibraryWithSource: ToNS(xpf::resources::xpf_metal()) options: nil error: &error];
        if (error)
        {
            Log::error(to_string(error.localizedDescription));
            std::exit(-1);
        }

        id<MTLFunction> vertexShader = [m_metalLibrary newFunctionWithName: @"vertexShader"];
        id<MTLFunction> fragmentShader = [m_metalLibrary newFunctionWithName: @"fragmentShader"];

        MTLVertexDescriptor* vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
        vertexDescriptor.layouts[30].stride = sizeof(VertexPositionColorTextureCoords);
        vertexDescriptor.layouts[30].stepRate = 1;
        vertexDescriptor.layouts[30].stepFunction = MTLVertexStepFunctionPerVertex;

        vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
        vertexDescriptor.attributes[0].offset = 0;
        vertexDescriptor.attributes[0].bufferIndex = 30;

        vertexDescriptor.attributes[1].format = MTLVertexFormatFloat4;
        vertexDescriptor.attributes[1].offset = sizeof(VertexPositionColor::position);
        vertexDescriptor.attributes[1].bufferIndex = 30;

        vertexDescriptor.attributes[2].format = MTLVertexFormatFloat2;
        vertexDescriptor.attributes[2].offset = sizeof(VertexPositionColor);
        vertexDescriptor.attributes[2].bufferIndex = 30;

        MTLRenderPipelineDescriptor* renderPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        renderPipelineDescriptor.label = @"XPF Rendering Pipeline";
        renderPipelineDescriptor.vertexFunction = vertexShader;
        renderPipelineDescriptor.fragmentFunction = fragmentShader;
        renderPipelineDescriptor.vertexDescriptor = vertexDescriptor;
        if (m_options.sample_count > 0 && [m_gpu supportsTextureSampleCount: m_options.sample_count])
        {
            renderPipelineDescriptor.rasterSampleCount = m_options.sample_count;

            MTLTextureDescriptor* textureDescriptor = [[MTLTextureDescriptor alloc] init];
            textureDescriptor.textureType = MTLTextureType2DMultisample;
            textureDescriptor.width = m_options.width;
            textureDescriptor.height = m_options.height;
            textureDescriptor.sampleCount = renderPipelineDescriptor.rasterSampleCount;
            textureDescriptor.pixelFormat = m_swapchain.pixelFormat;
            m_screenTexture = [m_gpu newTextureWithDescriptor: textureDescriptor];

            m_antialiasing_enabled = true;
        }

        // https://delasign.com/blog/metal-fragment-shader-alpha/
        MTLRenderPipelineColorAttachmentDescriptor* colorAttachment = renderPipelineDescriptor.colorAttachments[0];
        colorAttachment.pixelFormat = m_swapchain.pixelFormat;
        colorAttachment.blendingEnabled = YES;
        colorAttachment.rgbBlendOperation = MTLBlendOperationAdd;
        colorAttachment.alphaBlendOperation = MTLBlendOperationAdd;
        colorAttachment.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        colorAttachment.sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
        colorAttachment.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        colorAttachment.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

        m_metalRenderPSO = [m_gpu newRenderPipelineStateWithDescriptor:renderPipelineDescriptor error: &error];
        if (error)
        {
            Log::error(to_string(error.localizedDescription));
            std::exit(-1);
        }
    }

    void CreateTextureSamplers()
    {
        MTLSamplerDescriptor* samplerDesc = [MTLSamplerDescriptor new];
        samplerDesc.rAddressMode = MTLSamplerAddressModeRepeat;
        samplerDesc.sAddressMode = MTLSamplerAddressModeRepeat;
        samplerDesc.tAddressMode = MTLSamplerAddressModeRepeat;
        samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
        samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
        samplerDesc.mipFilter = MTLSamplerMipFilterNotMipmapped;
        m_linearSampler = [m_gpu newSamplerStateWithDescriptor:samplerDesc];

        samplerDesc.minFilter = MTLSamplerMinMagFilterNearest;
        samplerDesc.magFilter = MTLSamplerMinMagFilterNearest;
        m_nearestSampler = [m_gpu newSamplerStateWithDescriptor:samplerDesc];
    }

    virtual void RenderScope(std::function<void()>&& fn) override
    {
        @autoreleasepool
        {
            fn();
        }
    }

    virtual void Render() override
    {
        m_frame_count++;
        #pragma pack(push)
        #pragma pack(1)
        struct VertexData
        {
            m4_t transform;
            int commandId = 0;
        };
        #pragma pack(pop)
        
        m_renderStats = {};
        const IBuffer* pCurrentBuffer = nullptr;
        const ITexture* pCurrentTexture = nullptr;
        VertexData vertexData;
        vertexData.transform = m_projection_matrix;
        std::vector<m4_t> transforms({vertexData.transform});
        MTLScissorRect clipRegion;
        uint32_t applyClipRegion = 0;

        id<CAMetalDrawable> surface = [m_swapchain nextDrawable];
        MTLRenderPassDescriptor* pass = [MTLRenderPassDescriptor renderPassDescriptor];
        MTLRenderPassColorAttachmentDescriptor* passColorAttachment = pass.colorAttachments[0];
        passColorAttachment.clearColor = m_metal_background_color;
        passColorAttachment.loadAction  = MTLLoadActionClear;
        passColorAttachment.storeAction = m_antialiasing_enabled ? MTLStoreActionMultisampleResolve : MTLStoreActionStore;
        passColorAttachment.texture = m_antialiasing_enabled ? m_screenTexture : surface.texture;
        if (m_antialiasing_enabled)
            passColorAttachment.resolveTexture = surface.texture;

        id<MTLCommandBuffer> buffer = [m_queue commandBuffer];
        id<MTLRenderCommandEncoder> encoder = [buffer renderCommandEncoderWithDescriptor: pass];
        [encoder setRenderPipelineState: m_metalRenderPSO];
        [encoder setVertexBytes: (const byte_t*)&vertexData length:sizeof(vertexData) atIndex:0];
        [encoder setViewport:(MTLViewport){0.0, 0.0, float(m_options.width), float(m_options.height), 0.0, 1.0 }];

        m_builder.Build().ForEachCommand([&](auto& spCommand)
        {
            uint32_t instanceCount = 1;
            MTLPrimitiveType primitiveType = MTLPrimitiveTypeTriangle;

//            @autoreleasepool
            {
                if (spCommand->commandId == RenderCommandId::transform)
                {
                    const RenderTransformCommand& command = *static_cast<RenderTransformCommand*>(spCommand.get());
                    switch (command.type)
                    {
                        case RenderTransformCommand::MultiplyPush:
                            vertexData.transform = vertexData.transform * command.transform;
                            transforms.push_back(vertexData.transform);
                            break;
                        case RenderTransformCommand::Push:
                            vertexData.transform = m_projection_matrix * command.transform;
                            transforms.push_back(vertexData.transform);
                            break;
                        case RenderTransformCommand::Pop:
                            transforms.pop_back();
                            vertexData.transform = transforms.back();
                            break;
                    }

                    [encoder setVertexBytes: (const byte_t*)&vertexData length:sizeof(vertexData) atIndex:0];
                }
                else if (spCommand->commandId == RenderCommandId::clip)
                {
                    const RenderClipCommand& command = *static_cast<RenderClipCommand*>(spCommand.get());
                    clipRegion.x = command.clipRegion.x;
                    clipRegion.y = command.clipRegion.y;
                    clipRegion.width = command.clipRegion.width;
                    clipRegion.height = command.clipRegion.height;
                    switch (command.type)
                    {
                        case RenderClipCommand::Push:
                            applyClipRegion++;
                            break;
                        case RenderClipCommand::Pop:
                            [encoder setScissorRect:clipRegion];
                            applyClipRegion--;
                            break;
                    }
                }
                else
                {
                    const RenderDrawCommand& command = *static_cast<RenderDrawCommand*>(spCommand.get());

                    if (vertexData.commandId != (int32_t)command.commandId)
                    {
                        vertexData.commandId = (int32_t)command.commandId;
                        [encoder setVertexBytes: (const byte_t*)&vertexData length:sizeof(vertexData) atIndex:0];
                    }

                    {
                        #pragma pack(push)
                        #pragma pack(1)
                        struct FragmentData
                        {
                            int32_t commandId = 0;
                            float width = 0;
                            float height = 0;
                            float scale = 0;
                            CornerRadius cornerRadius;
                            Thickness borderThickness;
                            v4_t borderColor;
                            float time = 0;
                            int dataOffset = 0;
                        };
                        #pragma pack(pop)

                        FragmentData fdata;
                        fdata.commandId = int(command.commandId);
                        fdata.time = Time::GetTime();
                        size_t fdataSize = sizeof(fdata);
                        [encoder setVertexBuffer: nullptr offset: 0 atIndex: 1];

                        if (spCommand->commandId == RenderCommandId::rounded_rectangle ||
                            spCommand->commandId == RenderCommandId::rounded_rectangle_with_border ||
                            spCommand->commandId == RenderCommandId::rounded_rectangle_with_border_dots)
                        {
                            const RenderRoundedRectangleCommand& cmd = *static_cast<RenderRoundedRectangleCommand*>(spCommand.get());
                            fdata.width = cmd.size.x;
                            fdata.height = cmd.size.y;
                            fdata.cornerRadius = cmd.cornerRadius;
                            fdata.borderThickness = cmd.borderThickness;
                            fdata.borderColor = cmd.borderColor;
                        }
                        else if (spCommand->commandId == RenderCommandId::glyphs)
                        {
                            primitiveType = MTLPrimitiveTypeTriangleStrip;
                            const RenderGlyphsCommand& cmd = *static_cast<RenderGlyphsCommand*>(spCommand.get());
                            fdata.scale = cmd.scale;

                            [encoder setVertexBytes:(const void*)cmd.instanceData.data() length: cmd.instanceData.size() * sizeof(cmd.instanceData[0]) atIndex: 1];
                            instanceCount = cmd.glyphCount;

                            if (cmd.spBuffer != nullptr)
                            {
                                const IBuffer* pBuffer = cmd.spBuffer.get();
                                if (pCurrentBuffer != pBuffer)
                                {
                                    pCurrentBuffer = pBuffer;
                                    [encoder setFragmentBuffer: GetMTLBuffer(pBuffer) offset: 0 atIndex: 2];
                                    m_renderStats.bufferSwitches++;
                                }
                            }
                        }
                        else if (spCommand->commandId == RenderCommandId::glyph)
                        {
                            const RenderGlyphCommand& cmd = *static_cast<RenderGlyphCommand*>(spCommand.get());
                            fdata.scale = cmd.scale;
                            fdata.dataOffset = cmd.glyphStartOffset;

                            if (cmd.spBuffer != nullptr)
                            {
                                const IBuffer* pBuffer = cmd.spBuffer.get();
                                if (pCurrentBuffer != pBuffer)
                                {
                                    pCurrentBuffer = pBuffer;
                                    [encoder setFragmentBuffer: GetMTLBuffer(pBuffer) offset: 0 atIndex: 2];
                                    m_renderStats.bufferSwitches++;
                                }
                            }
                        }

                        [encoder setFragmentBytes: &fdata length:fdataSize atIndex:1];
                    }

                    const ITexture* pTexture = command.spTexture.get();
                    if (pTexture != nullptr)
                    {
                        if (pCurrentTexture != pTexture)
                        {
                            pCurrentTexture = pTexture;
                            [encoder setFragmentTexture: GetMTLTexture(pTexture) atIndex: 0];
                            m_renderStats.textureSwitches++;

                            switch(pCurrentTexture->GetInterpolation())
                            {
                                case ITexture::Interpolation::None:
                                    [encoder setFragmentSamplerState: m_nearestSampler atIndex: 0];
                                    break;
                                case ITexture::Interpolation::Linear:
                                    [encoder setFragmentSamplerState: m_linearSampler atIndex: 0];
                                    break;
                            }
                        }
                    }
                    else if (pCurrentTexture != nullptr)
                    {
                        pCurrentTexture = nullptr;
                        [encoder setFragmentTexture: nullptr atIndex: 0];
                        m_renderStats.textureSwitches++;
                    }

                    if (applyClipRegion != 0)
                        [encoder setScissorRect:clipRegion];

                    [encoder setVertexBytes:(const void*)command.verts.data() length: command.verts.size() * sizeof(command.verts[0]) atIndex: 30];
                    [encoder drawPrimitives:primitiveType vertexStart:0  vertexCount: command.count instanceCount: instanceCount];
                    m_renderStats.vertexCount += command.count;
                    m_renderStats.drawCount++;
                }
            }
        });

        [encoder endEncoding];

        if (m_captureScreen_frameCount > 0)
        {
            static std::vector<byte_t> s_bytes;
            s_bytes.resize(m_captureScreen_region.width * m_captureScreen_region.height * 4); // 4 bytes for RGBA
            [m_antialiasing_enabled ? m_screenTexture : surface.texture
                getBytes: s_bytes.data()
                bytesPerRow: m_captureScreen_region.width * 4
                bytesPerImage: s_bytes.size()
                fromRegion: MTLRegionMake2D(m_captureScreen_region.x, m_captureScreen_region.y, m_captureScreen_region.width, m_captureScreen_region.height)
                mipmapLevel: 0
                slice: 0];
            Image img(s_bytes, m_captureScreen_region.w, m_captureScreen_region.h, xpf::PixelFormat::R8G8B8A8);

            m_captureScreen_frameCount--;
            m_captureScreen_callback(std::move(img));

            s_bytes.clear();
        }

        if (m_options.metal_transactional_rendering)
            [surface present];
        else
            [buffer presentDrawable: surface];

        [buffer commit];
    }

    virtual std::shared_ptr<ITexture> CreateTexture(const Image& img) override
    {
        return Metal_CreateTexture(m_gpu, img);
    }

    virtual std::shared_ptr<ITexture> CreateTexture(std::string_view filename) override
    {
        return CreateTexture(Image::LoadImage(filename));
    }

    virtual std::shared_ptr<IBuffer> CreateBuffer(const byte_t* pbyte, size_t size) override
    {
        return Metal_CreateBuffer(m_gpu, pbyte, size);
    }
};

std::unique_ptr<IRenderer> create_metal_renderer()
{
    return std::make_unique<MetalRenderer>();
}

} // xpf