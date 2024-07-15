#pragma once

#include <limits>
#include <memory>

#ifdef WIN32
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#else
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif
#include "imgui/backends/imgui_impl_glfw.h"

#include "Buffer.h"
#include "GPUContext.h"
#include "Renderer.h"
#include "SwapChain.h"
#include "Texture.h"
#include "raven/passes/ImGuiPass.h"
#include "raven/util/Camera.h"

namespace raven {
    class Application : GPUContext {
    public:
        struct ApplicationSettings {
            vk::SampleCountFlagBits m_msaaSamples = vk::SampleCountFlagBits::e1;

            /*
             * RayTracing
             * VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME
             */
            std::vector<const char *> m_deviceExtensions = {};
        };

        Application(std::string &appName, ApplicationSettings appSettings, std::shared_ptr<Renderer> renderer, uint32_t requiredQueueFamilies);

        void setVSync(bool vsync);

        void resizeWindow(int width, int height) override;

        void run();

        Camera *getCamera() override {
            return m_camera.get();
        }

        [[nodiscard]] vk::Semaphore getTimelineSemaphore() const { return m_timelineSemaphore; }
        [[nodiscard]] uint64_t getTimelineSemaphoreValue() const { return m_timelineSemaphoreValue; }

        [[nodiscard]] vk::Semaphore getBinarySemaphore() const { return m_binarySemaphore; }

        [[nodiscard]] ImGuiContext *getImGuiContext() const { return m_imGuiContext; }

    protected:
        void getDeviceExtensions(std::vector<const char *> &extensions) const override;
        void getInstanceExtensions(std::vector<const char *> &extensions) const override;

    private:
        std::string m_appName;
        bool m_vsync = true;
        ApplicationSettings m_applicationSettings;

        ImGuiContext *m_imGuiContext = nullptr;

        std::shared_ptr<Camera> m_camera;

        std::shared_ptr<Renderer> m_renderer;

        std::shared_ptr<SwapChain> m_present;

        GLFWwindow *m_window{};
        int32_t m_width = 1280;
        int32_t m_height = 720;

        vk::SurfaceKHR m_surface{};

        std::shared_ptr<ImGuiPass> m_guiPass;

        void init() override;

        void loop();
        void update();
        void render();

        void reloadShader();

        void shutdown() override;

        // glfw specific
        void initWindow();
        static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
        void internalResizeWindow(int width, int height);
        void releaseWindow();

        void createSurface();

        void assertMSAASamplesSupported(vk::SampleCountFlagBits msaaSamples);

        // imgui
        void initImGui();
        void shutdownImGui();
        // void recreateSwapchainImGui();
        vk::DescriptorPool m_imGuiDescriptorPool{};
        bool m_imGuiInitialized = false;
        void recordImGui();

        uint32_t m_queueFamilyIndex{};

        vk::CommandPool m_commandPool{};
        vk::CommandBuffer m_commandBuffer; // destroyed implicitly with the command pool

        vk::Semaphore m_timelineSemaphore{};
        uint64_t m_timelineSemaphoreValue = 0U;

        vk::Semaphore m_binarySemaphore{};

        void createBlitPassResources() {
            m_queueFamilyIndex = findQueueFamilyIndex();
            createCommandPool();
            createCommandBuffers();
            createSyncObjects();
        }

        void releaseBlitPassResources() {
            releaseSyncObjects();
            vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        }

        uint32_t findQueueFamilyIndex() {
            Queues::QueueFamilyIndices queueFamilyIndices = m_queues->findQueueFamilies(m_physicalDevice);
            return queueFamilyIndices.graphicsFamily.value();
        }

        void createCommandPool() {
            vk::CommandPoolCreateInfo poolInfo{};
            poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
            poolInfo.queueFamilyIndex = m_queueFamilyIndex;

            if (m_device.createCommandPool(&poolInfo, nullptr, &m_commandPool) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create command pool!");
            }
        }

        void createCommandBuffers() {
            vk::CommandBufferAllocateInfo allocInfo{};
            allocInfo.commandPool = m_commandPool;
            allocInfo.level = vk::CommandBufferLevel::ePrimary;
            allocInfo.commandBufferCount = 1;

            if (m_device.allocateCommandBuffers(&allocInfo, &m_commandBuffer) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to allocate command buffers!");
            }
        }

        void createSyncObjects() {
            vk::SemaphoreTypeCreateInfo timelineCreateInfo{};
            timelineCreateInfo.semaphoreType = vk::SemaphoreType::eTimeline;
            timelineCreateInfo.initialValue = 0;

            vk::SemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.pNext = &timelineCreateInfo;

            if (m_device.createSemaphore(&semaphoreInfo, nullptr, &m_timelineSemaphore) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create timeline semaphore!");
            }

            semaphoreInfo.pNext = nullptr;
            if (m_device.createSemaphore(&semaphoreInfo, nullptr, &m_binarySemaphore) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create binary semaphore!");
            }
        }

        void releaseSyncObjects() const {
            m_device.destroySemaphore(m_timelineSemaphore, nullptr);
            m_device.destroySemaphore(m_binarySemaphore, nullptr);
        }

        void executeBlitPass(const std::function<void(vk::CommandBuffer)> &recordCommands,
            const uint32_t waitTimelineSemaphoreCount, const vk::Semaphore *waitTimelineSemaphore, const uint64_t *waitTimelineSemaphoreValue) {
            {
                // wait for the previous frame to finish
                vk::SemaphoreWaitInfo waitInfo{};
                waitInfo.semaphoreCount = 1;
                waitInfo.pSemaphores = &m_timelineSemaphore;
                waitInfo.pValues = &m_timelineSemaphoreValue;
                if (m_device.waitSemaphores(&waitInfo, UINT64_MAX) != vk::Result::eSuccess) {
                    throw std::runtime_error("Failed to wait for timeline semaphore!");
                }
            }

            const auto &commandBuffer = m_commandBuffer;
            commandBuffer.reset();
            constexpr vk::CommandBufferBeginInfo beginInfo{};
            if (commandBuffer.begin(&beginInfo) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to begin command buffer!");
            }
            recordCommands(commandBuffer);
            commandBuffer.end();

            m_timelineSemaphoreValue++;
            vk::TimelineSemaphoreSubmitInfo timelineSubmitInfo{};
            timelineSubmitInfo.waitSemaphoreValueCount = waitTimelineSemaphoreCount;
            timelineSubmitInfo.pWaitSemaphoreValues = waitTimelineSemaphoreValue;
            timelineSubmitInfo.signalSemaphoreValueCount = 2;
            const uint64_t signalSemaphoreValues[2] = { m_timelineSemaphoreValue, 0 };
            timelineSubmitInfo.pSignalSemaphoreValues = signalSemaphoreValues;

            vk::SubmitInfo submitInfo{};
            submitInfo.pNext = &timelineSubmitInfo;
            submitInfo.waitSemaphoreCount = waitTimelineSemaphoreCount;
            submitInfo.pWaitSemaphores = waitTimelineSemaphore;
            const std::vector<vk::PipelineStageFlags> waitDstStageMask = {vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eColorAttachmentOutput};
            submitInfo.pWaitDstStageMask = waitDstStageMask.data();
            submitInfo.signalSemaphoreCount = 2;
            const vk::Semaphore signalSemaphores[2] = { m_timelineSemaphore, m_binarySemaphore };
            submitInfo.pSignalSemaphores = signalSemaphores;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &m_commandBuffer;

            if (m_queues->getQueue(Queues::GRAPHICS).submit(1, &submitInfo, VK_NULL_HANDLE) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to submit command buffer to queue!");
            }
        }
    };
} // namespace raven
