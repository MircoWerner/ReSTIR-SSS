#pragma once
#include "PassShader.h"

#include <vector>

namespace raven {
#define RAVEN_PASS_RELEASE(pass) \
    if (pass)                    \
    pass->release()

    class Pass {
    public:
        struct PassSettings {
            std::string m_name = "undefined";

            bool m_semaphoreExportFlag = false;
        };

        Pass(GPUContext *gpuContext, PassSettings settings) : m_gpuContext(gpuContext), m_passSettings(std::move(settings)) {}

        virtual ~Pass() = default;

        virtual void create() {
            findQueueFamily();
            createCommandPool();
            createCommandBuffers();
            createSyncObjects();
        }

        virtual void release() {
            releaseSyncObjects();
            vkDestroyCommandPool(m_gpuContext->m_device, m_commandPool, nullptr);
        }

        void recreateSyncObjects() {
            releaseSyncObjects();
            createSyncObjects();
        }

        virtual void execute(const std::function<void()> &updateStorageBuffers,
                             const std::function<void()> &updateUniforms,
                             const std::function<void(vk::CommandBuffer &commandBuffer)> &recordCommandBuffer,
                             const uint32_t waitTimelineSemaphoreCount, const vk::Semaphore *waitTimelineSemaphore, const uint64_t *waitTimelineSemaphoreValue, vk::PipelineStageFlags *waitDstStageMask) {
            {
                // wait for the previous frame to finish
                vk::SemaphoreWaitInfo waitInfo{};
                waitInfo.semaphoreCount = 1;
                waitInfo.pSemaphores = &m_timelineSemaphore;
                waitInfo.pValues = &m_timelineSemaphoreValue;
                if (m_gpuContext->m_device.waitSemaphores(&waitInfo, UINT64_MAX) != vk::Result::eSuccess) {
                    throw std::runtime_error("Failed to wait for timeline semaphore!");
                }
            }

            updateStorageBuffers();
            updateUniforms();

            auto &commandBuffer = m_commandBuffer;
            commandBuffer.reset();
            constexpr vk::CommandBufferBeginInfo beginInfo{};
            if (commandBuffer.begin(&beginInfo) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to begin recording command buffer!");
            }
            recordCommandBuffer(commandBuffer);
            commandBuffer.end();

            m_timelineSemaphoreValue++;
            vk::TimelineSemaphoreSubmitInfo timelineSubmitInfo{};
            timelineSubmitInfo.waitSemaphoreValueCount = waitTimelineSemaphoreCount;
            timelineSubmitInfo.pWaitSemaphoreValues = waitTimelineSemaphoreValue;
            timelineSubmitInfo.signalSemaphoreValueCount = 1;
            timelineSubmitInfo.pSignalSemaphoreValues = &m_timelineSemaphoreValue;

            vk::SubmitInfo submitInfo{};
            submitInfo.pNext = &timelineSubmitInfo;
            submitInfo.waitSemaphoreCount = waitTimelineSemaphoreCount;
            submitInfo.pWaitSemaphores = waitTimelineSemaphore;
            submitInfo.pWaitDstStageMask = waitDstStageMask;
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &m_timelineSemaphore;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &m_commandBuffer;

            if (m_gpuContext->m_queues->getQueue(m_queueType).submit(1, &submitInfo, VK_NULL_HANDLE) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to submit command buffer!");
            }
        };

        std::string getName() const { return m_passSettings.m_name; }

        [[nodiscard]] vk::Semaphore getTimelineSemaphore() const { return m_timelineSemaphore; }
        [[nodiscard]] uint64_t getTimelineSemaphoreValue() const { return m_timelineSemaphoreValue; }

    protected:
        GPUContext *m_gpuContext;

        PassSettings m_passSettings;

        uint32_t m_queueFamilyIndex{};
        Queues::Queue m_queueType = Queues::GRAPHICS;

        vk::CommandPool m_commandPool{};
        vk::CommandBuffer m_commandBuffer; // destroyed implicitly with the command pool

        vk::Semaphore m_timelineSemaphore{};
        uint64_t m_timelineSemaphoreValue = 0U;

        virtual void findQueueFamily() = 0;

    private:
        void createCommandPool() {
            vk::CommandPoolCreateInfo poolInfo{};
            poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
            poolInfo.queueFamilyIndex = m_queueFamilyIndex;

            if (m_gpuContext->m_device.createCommandPool(&poolInfo, nullptr, &m_commandPool) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create command pool!");
            }
        }

        void createCommandBuffers() {
            vk::CommandBufferAllocateInfo allocInfo{};
            allocInfo.commandPool = m_commandPool;
            allocInfo.level = vk::CommandBufferLevel::ePrimary;
            allocInfo.commandBufferCount = 1;

            if (m_gpuContext->m_device.allocateCommandBuffers(&allocInfo, &m_commandBuffer) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to allocate command buffers!");
            }
        }

        void createSyncObjects() {
            m_timelineSemaphoreValue = 0U;

            vk::SemaphoreTypeCreateInfo timelineCreateInfo{};
            timelineCreateInfo.semaphoreType = vk::SemaphoreType::eTimeline;
            timelineCreateInfo.initialValue = 0;

            vk::ExportSemaphoreCreateInfo esci{};
            if (m_passSettings.m_semaphoreExportFlag) {
#ifdef WIN32
                auto handleType = vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32;
#else
                auto handleType = vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueFd;
#endif
                esci.handleTypes = handleType;
                timelineCreateInfo.pNext = &esci;
            }

            vk::SemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.pNext = &timelineCreateInfo;

            if (m_gpuContext->m_device.createSemaphore(&semaphoreInfo, nullptr, &m_timelineSemaphore) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create timeline semaphore!");
            }
        }

        void releaseSyncObjects() const {
            m_gpuContext->m_device.destroySemaphore(m_timelineSemaphore, nullptr);
        }
    };
} // namespace raven