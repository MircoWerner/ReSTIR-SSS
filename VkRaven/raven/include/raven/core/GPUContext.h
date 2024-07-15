#pragma once

#include <cstring>
#include <iostream>
#include <optional>
#include <regex>
#include <set>
#include <stdexcept>
#include <vector>

#include "raven/RavenInclude.h"

//#include "raven/core/VkExtensions.h"

#include "Queues.h"
#include "VkDebugUtils.h"
#include "raven/util/Camera.h"

namespace raven {
    class GPUContext {
    public:
        explicit GPUContext(uint32_t requiredQueueFamilies);

        virtual void init();

        virtual void shutdown();

        vk::PhysicalDevice m_physicalDevice = VK_NULL_HANDLE; // will be destroyed implicitly when instance is destroyed

        vk::Device m_device{};
        std::shared_ptr<Queues> m_queues;

        uint32_t m_activeIndex = 0;

        [[nodiscard]] uint32_t getMultiBufferedCount() const {
            return MAX_FRAMES_IN_FLIGHT;
        }

        [[nodiscard]] uint32_t getActiveIndex() const {
            return m_activeIndex;
        }

        vk::CommandPool m_graphicsCommandPool{};
        vk::CommandPool m_computeCommandPool{};
        vk::CommandPool m_transferCommandPool{};

        void executeCommands(const std::function<void(vk::CommandBuffer)> &recordCommands, Queues::Queue queue = Queues::Queue::TRANSFER, uint32_t awaitBeforeExecutionCount = 0, vk::Semaphore *awaitBeforeExecution = nullptr, vk::PipelineStageFlags *awaitBeforeExecutionStages = nullptr) {
            vk::CommandBuffer commandBuffer;
            switch (queue) {
                case Queues::GRAPHICS:
                    commandBuffer = m_graphicsCommandBuffers[m_activeIndex];
                    break;
                case Queues::COMPUTE:
                    commandBuffer = m_computeCommandBuffers[m_activeIndex];
                    break;
                case Queues::TRANSFER:
                    commandBuffer = m_transferCommandBuffers[m_activeIndex];
                    break;
                default:
                    throw std::runtime_error("Execute command does not support other queues!");
            }

            vk::CommandBufferBeginInfo beginInfo{};
            beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

            if (commandBuffer.begin(&beginInfo) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to begin command buffer!");
            }

            recordCommands(commandBuffer);

            vkEndCommandBuffer(commandBuffer);

            vk::SubmitInfo submitInfo{};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            submitInfo.waitSemaphoreCount = awaitBeforeExecutionCount;
            submitInfo.pWaitSemaphores = awaitBeforeExecution;
            submitInfo.pWaitDstStageMask = awaitBeforeExecutionStages;

            if (m_queues->getQueue(queue).submit(1, &submitInfo, VK_NULL_HANDLE) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to submit command buffer to queue!");
            }
            m_queues->getQueue(queue).waitIdle();
        }

        [[nodiscard]] VkDebugUtils *getDebug() const {
            return m_debug.get();
        }

        virtual Camera *getCamera() {
            return nullptr;
        }

        PFN_vkVoidFunction getDeviceFunction(const char *name) const {
            return m_device.getProcAddr(name);
        }

        [[nodiscard]] float getDeviceLimitsTimestampPeriod() const {
            return m_deviceLimitsTimestampPeriod;
        }

        virtual void resizeWindow(int width, int height) {}

        float incrementAndReturnDeltaTime() {
            const auto currentTime = std::chrono::high_resolution_clock::now();
            const float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_time).count();
            m_time = currentTime;
            return time;
        }

    protected:
        vk::Instance m_instance{};
        VkDebugUtilsMessengerEXT m_debugMessenger{};

        std::vector<vk::CommandBuffer> m_graphicsCommandBuffers;
        std::vector<vk::CommandBuffer> m_computeCommandBuffers;
        std::vector<vk::CommandBuffer> m_transferCommandBuffers;

        virtual void getDeviceExtensions(std::vector<const char *> &extensions) const;
        virtual void getInstanceExtensions(std::vector<const char *> &extensions) const;

        void incrementActiveIndex() {
            m_activeIndex = (m_activeIndex + 1) % MAX_FRAMES_IN_FLIGHT;
        }

        std::chrono::time_point<std::chrono::system_clock> m_time = std::chrono::high_resolution_clock::now();

    private:
#ifdef NDEBUG
        const bool enableValidationLayers = true;
#else
        const bool enableValidationLayers = true;
#endif
        const std::vector<const char *> validationLayers = {
                "VK_LAYER_KHRONOS_validation"};

        void initVulkan();
        void releaseVulkan();

        void createInstance();
        bool checkValidationLayerSupport();
        static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);
        void setupDebugMessenger();
        static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);
        static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);
        void pickPhysicalDevice();
        void queryPhysicalDeviceProperties();

        void createLogicalDevice();

        void createCommandPool();
        void createCommandBuffers();

        std::shared_ptr<VkDebugUtils> m_debug;

        const uint32_t MAX_FRAMES_IN_FLIGHT = 1; // TODO

        float m_deviceLimitsTimestampPeriod = 0.f;
    };
} // namespace raven