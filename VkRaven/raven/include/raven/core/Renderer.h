#pragma once

#include "GPUContext.h"
#include "raven/util/Camera.h"
#include "raven/core/Image.h"

namespace raven {
    class Renderer {
    public:
        struct RendererResult {
            Image *m_image = nullptr;

            vk::Semaphore m_waitTimelineSemaphore{};
            uint64_t m_waitTimelineSemaphoreValue = 0U;
        };

        virtual ~Renderer() = default;

        virtual void initResources(GPUContext *gpuContext) = 0;          // called only once on startup
        virtual void initShaderResources(GPUContext *gpuContext) = 0;    // called whenever the shaders are reloaded
        virtual void initSwapchainResources(GPUContext *gpuContext, vk::Extent2D extent) = 0; // called whenever the swapchain size changes

        virtual void recordImGui(GPUContext *gpuContext, Camera *camera) = 0;

        virtual void update(GPUContext *gpuContext, Camera *camera, float deltaTime) = 0;
        virtual void render(GPUContext *gpuContext, Camera *camera, uint32_t activeIndex, RendererResult *rendererResult) = 0;

        virtual void releaseResources() = 0;
        virtual void releaseShaderResources() = 0;
        virtual void releaseSwapchainResources() = 0;
    };
} // namespace raven
