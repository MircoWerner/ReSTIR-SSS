#pragma once

#include "PassShader.h"
#include "PassShaderCompute.h"

namespace raven {
    class PassShaderRayTracing : public PassShaderCompute {
    public:
        PassShaderRayTracing(GPUContext *gpuContext, std::string name) : PassShaderCompute(gpuContext, std::move(name)) {
            vk::PhysicalDeviceProperties2 deviceProperties;
            deviceProperties.pNext = &m_rtPipelineProperties;
            m_gpuContext->m_physicalDevice.getProperties2(&deviceProperties);
        }

        void create() override {
            m_rayGroups = createRayGroups();
            PassShaderCompute::create();
            createShaderBindingTable();
        }

        void release() override {
            RAVEN_BUFFER_RELEASE(m_sbtBufferRayGen);
            RAVEN_BUFFER_RELEASE(m_sbtBufferMiss);
            RAVEN_BUFFER_RELEASE(m_sbtBufferHit);
            PassShaderCompute::release();
        }

        void setGlobalInvocationSize(const uint32_t width, const uint32_t height, const uint32_t depth) override {
            m_invocationSize = vk::Extent3D{width, height, depth};
        }

        void execute(const std::function<void(const vk::CommandBuffer &commandBuffer, const vk::PipelineLayout &pipelineLayout)> &recordPushConstants,
                     const std::function<void(const vk::CommandBuffer &commandBuffer)> &recordPipelineBarriers,
                     vk::CommandBuffer &commandBuffer) override {
            assert(m_invocationSize.width >= 1 && m_invocationSize.height >= 1 && m_invocationSize.depth >= 1 && "Invocation size must be set before executing PassShaderRayTracing!");

            queryTimeFromGPU();

            commandBuffer.resetQueryPool(m_queryPoolTimestamps, 0, static_cast<uint32_t>(m_timestamps.size()));

            recordPushConstants(commandBuffer, m_pipelineLayout);

            commandBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, m_pipeline);

            std::vector<vk::DescriptorSet> descriptorSets;
            getDescriptorSets(descriptorSets);
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_pipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

            commandBuffer.writeTimestamp(vk::PipelineStageFlagBits::eRayTracingShaderKHR, m_queryPoolTimestamps, 0);
            vk::DebugUtilsLabelEXT label{};
            label.pLabelName = m_name.c_str();
            commandBuffer.beginDebugUtilsLabelEXT(&label);
            commandBuffer.traceRaysKHR(&m_rayGenShaderSBTEntry, &m_missShaderSBTEntry, &m_hitShaderSBTEntry, &m_callableShaderSBTEntry, m_invocationSize.width, m_invocationSize.height,
                                   m_invocationSize.depth);
            commandBuffer.endDebugUtilsLabelEXT();
            commandBuffer.writeTimestamp(vk::PipelineStageFlagBits::eRayTracingShaderKHR, m_queryPoolTimestamps, 1);

            recordPipelineBarriers(commandBuffer);
        }

    protected:
        void createPipeline() override {
            assert(!m_shaders.empty() && "PassShaderRayTracing requires at least one ray generation shader!");

            std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(m_shaders.size());
            for (uint32_t i = 0; i < m_shaders.size(); i++) {
                shaderStages[i] = m_shaders[i]->generateShaderStageCreateInfo();
            }

            uint32_t numRayGenGroups = m_rayGroups.getRayGenGroupCount();
            uint32_t numMissGroups = m_rayGroups.getRayMissGroupCount();
            uint32_t numHitGroups = m_rayGroups.getRayHitGroupCount();
            uint32_t numGroups = numRayGenGroups + numMissGroups + numHitGroups;

            std::vector<vk::RayTracingShaderGroupCreateInfoKHR> groups(numGroups);
            uint32_t groupIndex = 0;
            for (const auto &group: m_rayGroups.rayGenGroups) {
                groups[groupIndex++] = createRayGenGroup(group.rayGenShaderIndex);
            }
            for (const auto &group: m_rayGroups.rayMissGroups) {
                groups[groupIndex++] = createMissGroup(group.missShaderIndex);
            }
            for (const auto &group: m_rayGroups.rayHitGroups) {
                if (group.isTriangleHitGroup()) {
                    groups[groupIndex++] = createTriangleHitGroup(group.hitShaderIndex);
                } else {
                    groups[groupIndex++] = createProceduralHitGroup(group.hitShaderIndex, group.intersectionShaderIndex);
                }
            }

            vk::RayTracingPipelineCreateInfoKHR pipelineInfo{};
            pipelineInfo.sType = vk::StructureType::eRayTracingPipelineCreateInfoKHR;
            pipelineInfo.stageCount = shaderStages.size();
            pipelineInfo.pStages = shaderStages.data();
            pipelineInfo.groupCount = groups.size();
            pipelineInfo.pGroups = groups.data();
            pipelineInfo.maxPipelineRayRecursionDepth = 1;
            pipelineInfo.layout = m_pipelineLayout;

            if (m_gpuContext->m_device.createRayTracingPipelinesKHR(VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create ray tracing pipeline!");
            }
        }

        bool getPushConstantRange(vk::PushConstantRange &pushConstantRange) override {
            pushConstantRange.stageFlags = getPushConstantStage();
            pushConstantRange.offset = 0;
            pushConstantRange.size = getPushConstantRangeSize();
            return pushConstantRange.size > 0;
        }

        vk::ShaderStageFlagBits getPushConstantStage() override {
            return vk::ShaderStageFlagBits::eRaygenKHR;
        }

        enum RayHitGroupType {
            eTriangle,
            eProcedural
        };

        struct RayGenGroup {
            uint32_t rayGenShaderIndex = 0;
        };
        struct RayMissGroup {
            uint32_t missShaderIndex = 0;
        };
        struct RayHitGroup {
            uint32_t hitShaderIndex = 0;
            uint32_t intersectionShaderIndex = 0;
            RayHitGroupType type = eTriangle;

            [[nodiscard]] bool isTriangleHitGroup() const {
                return type == eTriangle;
            }

            [[nodiscard]] bool isProceduralHitGroup() const {
                return type == eProcedural;
            }
        };

        struct RayGroups {
            std::vector<RayGenGroup> rayGenGroups;
            std::vector<RayMissGroup> rayMissGroups;
            std::vector<RayHitGroup> rayHitGroups;

            [[nodiscard]] uint32_t getRayGenGroupCount() const {
                return rayGenGroups.size();
            }

            [[nodiscard]] uint32_t getRayMissGroupCount() const {
                return rayMissGroups.size();
            }

            [[nodiscard]] uint32_t getRayHitGroupCount() const {
                return rayHitGroups.size();
            }
        };

        virtual RayGroups createRayGroups() = 0;

    private:
        vk::Extent3D m_invocationSize = {0, 0, 0};

        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR m_rtPipelineProperties{};

        RayGroups m_rayGroups;

        std::shared_ptr<Buffer> m_sbtBufferRayGen;
        std::shared_ptr<Buffer> m_sbtBufferMiss;
        std::shared_ptr<Buffer> m_sbtBufferHit;

        vk::StridedDeviceAddressRegionKHR m_rayGenShaderSBTEntry{};
        vk::StridedDeviceAddressRegionKHR m_missShaderSBTEntry{};
        vk::StridedDeviceAddressRegionKHR m_hitShaderSBTEntry{};
        vk::StridedDeviceAddressRegionKHR m_callableShaderSBTEntry{};

        void createShaderBindingTable() {
            uint32_t numRayGenGroups = m_rayGroups.getRayGenGroupCount();
            uint32_t numMissGroups = m_rayGroups.getRayMissGroupCount();
            uint32_t numHitGroups = m_rayGroups.getRayHitGroupCount();
            uint32_t numGroups = numRayGenGroups + numMissGroups + numHitGroups;

            const uint32_t handleSizeAligned = alignUp(m_rtPipelineProperties.shaderGroupHandleSize, m_rtPipelineProperties.shaderGroupHandleAlignment);
            const auto groupCount = static_cast<uint32_t>(numGroups);
            const uint32_t sbtSize = groupCount * handleSizeAligned;

            Buffer::BufferSettings bufferSettingsRayGen{.m_sizeBytes = static_cast<uint32_t>(numRayGenGroups * handleSizeAligned),
                                                        .m_bufferUsages = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR,
                                                        .m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
                                                        .m_memoryAllocateFlagBits = vk::MemoryAllocateFlagBits::eDeviceAddress,
                                                        .m_name = "sbtRayGen"};
            Buffer::BufferSettings bufferSettingsMiss{.m_sizeBytes = static_cast<uint32_t>(numMissGroups * handleSizeAligned),
                                                      .m_bufferUsages = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR,
                                                      .m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
                                                      .m_memoryAllocateFlagBits = vk::MemoryAllocateFlagBits::eDeviceAddress,
                                                      .m_name = "sbtMiss"};
            Buffer::BufferSettings bufferSettingsHit{.m_sizeBytes = static_cast<uint32_t>(numHitGroups * handleSizeAligned),
                                                     .m_bufferUsages = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR,
                                                     .m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
                                                     .m_memoryAllocateFlagBits = vk::MemoryAllocateFlagBits::eDeviceAddress,
                                                     .m_name = "sbtHit"};
            m_sbtBufferRayGen = std::make_shared<Buffer>(m_gpuContext, bufferSettingsRayGen);
            m_sbtBufferMiss = std::make_shared<Buffer>(m_gpuContext, bufferSettingsMiss);
            m_sbtBufferHit = std::make_shared<Buffer>(m_gpuContext, bufferSettingsHit);

            std::vector<uint8_t> shaderHandleStorage(sbtSize);
            if (m_gpuContext->m_device.getRayTracingShaderGroupHandlesKHR(m_pipeline, 0, groupCount, sbtSize, shaderHandleStorage.data()) != vk::Result::eSuccess) {
                throw std::runtime_error("Cannot retrieve ray tracing shader group handles.");
            }

            {
                m_sbtBufferRayGen = Buffer::fillDeviceWithStagingBuffer(m_gpuContext, bufferSettingsRayGen, shaderHandleStorage.data());
            }
            {
                m_sbtBufferMiss = Buffer::fillDeviceWithStagingBuffer(m_gpuContext, bufferSettingsMiss, shaderHandleStorage.data() + handleSizeAligned * numRayGenGroups);
            }
            {
                m_sbtBufferHit = Buffer::fillDeviceWithStagingBuffer(m_gpuContext, bufferSettingsHit, shaderHandleStorage.data() + handleSizeAligned * (numRayGenGroups + numMissGroups));
            }

            m_rayGenShaderSBTEntry.deviceAddress = m_sbtBufferRayGen->getDeviceAddress();
            m_rayGenShaderSBTEntry.stride = handleSizeAligned;
            m_rayGenShaderSBTEntry.size = handleSizeAligned;

            m_missShaderSBTEntry.deviceAddress = m_sbtBufferMiss->getDeviceAddress();
            m_missShaderSBTEntry.stride = handleSizeAligned;
            m_missShaderSBTEntry.size = handleSizeAligned * numMissGroups;

            m_hitShaderSBTEntry.deviceAddress = m_sbtBufferHit->getDeviceAddress();
            m_hitShaderSBTEntry.stride = handleSizeAligned;
            m_hitShaderSBTEntry.size = handleSizeAligned * numHitGroups;
        }

        static vk::RayTracingShaderGroupCreateInfoKHR createRayGenGroup(uint32_t rayGenShaderIndex) {
            return createGeneralGroup(rayGenShaderIndex);
        }
        static vk::RayTracingShaderGroupCreateInfoKHR createMissGroup(uint32_t missShaderIndex) {
            return createGeneralGroup(missShaderIndex);
        }
        static vk::RayTracingShaderGroupCreateInfoKHR createTriangleHitGroup(uint32_t hitShaderIndex) {
            vk::RayTracingShaderGroupCreateInfoKHR group{};
            group.sType = vk::StructureType::eRayTracingShaderGroupCreateInfoKHR;
            group.type = vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;
            group.generalShader = VK_SHADER_UNUSED_KHR;
            group.closestHitShader = hitShaderIndex;
            group.anyHitShader = VK_SHADER_UNUSED_KHR;
            group.intersectionShader = VK_SHADER_UNUSED_KHR;
            return group;
        }
        static vk::RayTracingShaderGroupCreateInfoKHR createProceduralHitGroup(uint32_t hitShaderIndex, uint32_t intersectionShaderIndex) {
            vk::RayTracingShaderGroupCreateInfoKHR group{};
            group.sType = vk::StructureType::eRayTracingShaderGroupCreateInfoKHR;
            group.type = vk::RayTracingShaderGroupTypeKHR::eProceduralHitGroup;
            group.generalShader = VK_SHADER_UNUSED_KHR;
            group.closestHitShader = hitShaderIndex;
            group.anyHitShader = VK_SHADER_UNUSED_KHR;
            group.intersectionShader = intersectionShaderIndex;
            return group;
        }
        static vk::RayTracingShaderGroupCreateInfoKHR createGeneralGroup(uint32_t generalShaderIndex) {
            vk::RayTracingShaderGroupCreateInfoKHR group{};
            group.sType = vk::StructureType::eRayTracingShaderGroupCreateInfoKHR;
            group.type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
            group.generalShader = generalShaderIndex;
            group.closestHitShader = VK_SHADER_UNUSED_KHR;
            group.anyHitShader = VK_SHADER_UNUSED_KHR;
            group.intersectionShader = VK_SHADER_UNUSED_KHR;
            return group;
        }

        static uint32_t alignUp(uint32_t toAlign, uint32_t multiple) {
            if (multiple == 0) {
                return toAlign;
            }

            uint32_t remainder = toAlign % multiple;
            if (remainder == 0) {
                return toAlign;
            }

            return toAlign + multiple - remainder;
        }
    };
} // namespace raven