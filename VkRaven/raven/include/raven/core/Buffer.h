#pragma once

#include <cstring>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#include "GPUContext.h"

namespace raven {
#define RAVEN_BUFFER_RELEASE(buffer) \
if (buffer) buffer->release()

    class Buffer {
    public:
        struct BufferSettings {
            uint64_t m_sizeBytes;
            vk::BufferUsageFlags m_bufferUsages;
            vk::MemoryPropertyFlags m_memoryProperties;
            std::optional<vk::MemoryAllocateFlagBits> m_memoryAllocateFlagBits{};

            std::string m_name = "undefined";

            bool m_exportFlag = false;
        };

        Buffer(GPUContext *gpuContext, BufferSettings settings) : m_gpuContext(gpuContext), m_bufferSettings(std::move(settings)) {
            createBuffer();

            m_gpuContext->getDebug()->setName(m_buffer, m_bufferSettings.m_name);
            m_gpuContext->getDebug()->setName(m_bufferMemory, m_bufferSettings.m_name);
        }

        ~Buffer() {
            release();
        }

        void release() {
            if (m_buffer) {
                m_gpuContext->m_device.destroyBuffer(m_buffer);
            }
            if (m_bufferMemory) {
                m_gpuContext->m_device.freeMemory(m_bufferMemory);
            }
            m_buffer = nullptr;
            m_bufferMemory = nullptr;
        }

        static std::shared_ptr<Buffer> fillDeviceWithStagingBuffer(GPUContext *gpuContext, const BufferSettings& settings, void *data) { // upload
            Buffer stagingBuffer(gpuContext, {settings.m_sizeBytes, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent});

            void *stagingMemory;
            vkMapMemory(gpuContext->m_device, stagingBuffer.m_bufferMemory, 0, settings.m_sizeBytes, 0, &stagingMemory); // memory-mapped I/O
            memcpy(stagingMemory, data, settings.m_sizeBytes);
            vkUnmapMemory(gpuContext->m_device, stagingBuffer.m_bufferMemory);

            auto buffer = std::make_shared<Buffer>(gpuContext, settings);

            copyBuffer(gpuContext, stagingBuffer.m_buffer, buffer->m_buffer, settings.m_sizeBytes); // copy contents from staging buffer to high performance memory on GPU, which cannot be accessed directly by the CPU (therefore the staging buffer)

            stagingBuffer.release();

            return buffer;
        }

        void uploadWithStagingBuffer(void *data) {
            Buffer stagingBuffer(m_gpuContext, {m_bufferSettings.m_sizeBytes, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent});

            void *stagingMemory;
            vkMapMemory(m_gpuContext->m_device, stagingBuffer.m_bufferMemory, 0, m_bufferSettings.m_sizeBytes, 0, &stagingMemory); // memory-mapped I/O
            memcpy(stagingMemory, data, m_bufferSettings.m_sizeBytes);
            vkUnmapMemory(m_gpuContext->m_device, stagingBuffer.m_bufferMemory);

            copyBuffer(m_gpuContext, stagingBuffer.m_buffer, m_buffer, m_bufferSettings.m_sizeBytes); // copy contents from staging buffer to high performance memory on GPU, which cannot be accessed directly by the CPU (therefore the staging buffer)

            stagingBuffer.release();
        }

        void downloadWithStagingBuffer(void *data, const uint64_t bytes = UINT64_MAX) {
            uint64_t sizeBytes = bytes == UINT64_MAX ? m_bufferSettings.m_sizeBytes : bytes;

            Buffer stagingBuffer(m_gpuContext, {sizeBytes, vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent});

            copyBuffer(m_gpuContext, m_buffer, stagingBuffer.m_buffer, sizeBytes); // copy contents from staging buffer to high performance memory on GPU, which cannot be accessed directly by the CPU (therefore the staging buffer)

            //            void *stagingMemory;
            //            vkMapMemory(m_device, stagingBuffer.m_bufferMemory, 0, m_bufferSettings.m_sizeBytes, 0, &stagingMemory); // memory-mapped I/O
            //            memcpy(data, stagingMemory, m_bufferSettings.m_sizeBytes);
            //            vkUnmapMemory(m_device, stagingBuffer.m_bufferMemory);
            stagingBuffer.download(data);

            stagingBuffer.release();
        }

        void download(void *data) {
            void *stagingMemory;
            vkMapMemory(m_gpuContext->m_device, m_bufferMemory, 0, m_bufferSettings.m_sizeBytes, 0, &stagingMemory); // memory-mapped I/O
            memcpy(data, stagingMemory, m_bufferSettings.m_sizeBytes);
            vkUnmapMemory(m_gpuContext->m_device, m_bufferMemory);
        }

        void updateHostMemory(uint64_t sizeBytes, void *data) {
            void *memory;
            vkMapMemory(m_gpuContext->m_device, m_bufferMemory, 0, sizeBytes, 0, &memory); // memory-mapped I/O
            memcpy(memory, data, sizeBytes);
            vkUnmapMemory(m_gpuContext->m_device, m_bufferMemory);
        }

        void *mapHostMemory() {
            void *memory;
            vkMapMemory(m_gpuContext->m_device, m_bufferMemory, 0, m_bufferSettings.m_sizeBytes, 0, &memory); // memory-mapped I/O
            return memory;
        }

        void unmapHostMemory() {
            vkUnmapMemory(m_gpuContext->m_device, m_bufferMemory);
        }


        vk::Buffer getBuffer() {
            return m_buffer;
        }

        vk::DeviceMemory getDeviceMemory() {
            return m_bufferMemory;
        }

        [[nodiscard]] uint64_t getSizeBytes() const {
            return m_bufferSettings.m_sizeBytes;
        }

        vk::DeviceAddress getDeviceAddress() {
            vk::BufferDeviceAddressInfo addressInfo;
            addressInfo.buffer = m_buffer;
            return m_gpuContext->m_device.getBufferAddress(&addressInfo);
        }

    private:
        GPUContext *m_gpuContext;

        vk::Buffer m_buffer = nullptr;
        vk::DeviceMemory m_bufferMemory = nullptr;

        BufferSettings m_bufferSettings;

        void createBuffer() {
            vk::BufferCreateInfo bufferInfo;
            bufferInfo.size = m_bufferSettings.m_sizeBytes;
            bufferInfo.usage = m_bufferSettings.m_bufferUsages;
            bufferInfo.sharingMode = vk::SharingMode::eExclusive;
            vk::ExternalMemoryBufferCreateInfo externalMemoryBufferCreateInfo{};
#ifdef WIN32
            externalMemoryBufferCreateInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
#else
            externalMemoryBufferCreateInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd;
#endif
            if (m_bufferSettings.m_exportFlag) {
                bufferInfo.pNext = &externalMemoryBufferCreateInfo;
            }

            if (m_gpuContext->m_device.createBuffer(&bufferInfo, nullptr, &m_buffer) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create buffer!");
            }

            vk::MemoryRequirements memRequirements;
            m_gpuContext->m_device.getBufferMemoryRequirements(m_buffer, &memRequirements);

            vk::MemoryAllocateInfo allocInfo{};
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(m_gpuContext->m_physicalDevice, memRequirements.memoryTypeBits, m_bufferSettings.m_memoryProperties);
            vk::MemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
            if (m_bufferSettings.m_memoryAllocateFlagBits.has_value()) {
                memoryAllocateFlagsInfo.flags = m_bufferSettings.m_memoryAllocateFlagBits.value();
            }
            allocInfo.pNext = &memoryAllocateFlagsInfo;
            vk::ExportMemoryAllocateInfo exportAllocInfo{};
#ifdef WIN32
            exportAllocInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
#else
            exportAllocInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd;
#endif
            if (m_bufferSettings.m_exportFlag) {
                memoryAllocateFlagsInfo.pNext = &exportAllocInfo;
            }

            if (m_gpuContext->m_device.allocateMemory(&allocInfo, nullptr, &m_bufferMemory) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to allocate buffer memory!");
            }

            m_gpuContext->m_device.bindBufferMemory(m_buffer, m_bufferMemory, 0);
        }

        static uint32_t findMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
            vk::PhysicalDeviceMemoryProperties memProperties;
            physicalDevice.getMemoryProperties(&memProperties);

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }

            throw std::runtime_error("Failed to find suitable memory type!");
        }

        static void copyBuffer(GPUContext *gpuContext, vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
            vk::BufferCopy copyRegion{};
            copyRegion.srcOffset = 0; // optional
            copyRegion.dstOffset = 0; // optional
            copyRegion.size = size;

            gpuContext->executeCommands([&](vk::CommandBuffer commandBuffer) {
                commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
            });
        }
    };
} // namespace raven