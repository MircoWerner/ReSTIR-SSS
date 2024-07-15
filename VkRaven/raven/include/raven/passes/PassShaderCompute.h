#pragma once

#include "PassShader.h"

namespace raven {
    class PassShaderCompute : public PassShader {
    public:
        PassShaderCompute(GPUContext *gpuContext, std::string name) : PassShader(gpuContext, std::move(name)) {}

        void create() override { PassShader::create(); }

        void release() override { PassShader::release(); }

        void setWorkGroupCount(const uint32_t x, const uint32_t y, const uint32_t z) {
            m_workGroupCount = vk::Extent3D{x, y, z};
        }

        virtual void setGlobalInvocationSize(const uint32_t width, const uint32_t height, const uint32_t depth) {
            const vk::Extent3D workGroupSize = m_shaders[0]->getWorkGroupSize();
            m_workGroupCount = getDispatchSize(width, height, depth, workGroupSize);
        }

        static vk::Extent3D getDispatchSize(const uint32_t width, const uint32_t height, const uint32_t depth, const vk::Extent3D workGroupSize) {
            const uint32_t x = (width + workGroupSize.width - 1) / workGroupSize.width;
            const uint32_t y = (height + workGroupSize.height - 1) / workGroupSize.height;
            const uint32_t z = (depth + workGroupSize.depth - 1) / workGroupSize.depth;
            return vk::Extent3D{x, y, z};
        }

        void execute(const std::function<void(const vk::CommandBuffer &commandBuffer, const vk::PipelineLayout &pipelineLayout)> &recordPushConstants,
                     const std::function<void(const vk::CommandBuffer &commandBuffer)> &recordPipelineBarriers,
                     vk::CommandBuffer &commandBuffer) override {
            assert(m_workGroupCount.width >= 1 && m_workGroupCount.height >= 1 && m_workGroupCount.depth >= 1 && "Global invocation size must be set before executing PassShaderCompute!");

            queryTimeFromGPU();

            commandBuffer.resetQueryPool(m_queryPoolTimestamps, 0, static_cast<uint32_t>(m_timestamps.size()));

            recordPushConstants(commandBuffer, m_pipelineLayout);

            commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_pipeline);

            std::vector<vk::DescriptorSet> descriptorSets;
            getDescriptorSets(descriptorSets);
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_pipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

            commandBuffer.writeTimestamp(vk::PipelineStageFlagBits::eComputeShader, m_queryPoolTimestamps, 0);
            vk::DebugUtilsLabelEXT label{};
            label.pLabelName = m_name.c_str();
            commandBuffer.beginDebugUtilsLabelEXT(&label);
            commandBuffer.dispatch(m_workGroupCount.width, m_workGroupCount.height, m_workGroupCount.depth);
            commandBuffer.endDebugUtilsLabelEXT();
            commandBuffer.writeTimestamp(vk::PipelineStageFlagBits::eComputeShader, m_queryPoolTimestamps, 1);

            recordPipelineBarriers(commandBuffer);
        }

    protected:
        void createPipeline() override {
            assert(m_shaders.size() == 1 && "Only one shader is allowed for PassShaderCompute!");

            const vk::PipelineShaderStageCreateInfo shaderStage = m_shaders[0]->generateShaderStageCreateInfo();

            vk::ComputePipelineCreateInfo pipelineInfo{};
            pipelineInfo.layout = m_pipelineLayout;
            pipelineInfo.stage = shaderStage;

            if (m_gpuContext->m_device.createComputePipelines(VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create compute pipeline!");
            }
        }

        bool getPushConstantRange(vk::PushConstantRange &pushConstantRange) override {
            pushConstantRange.stageFlags = getPushConstantStage();
            pushConstantRange.offset = 0;
            pushConstantRange.size = getPushConstantRangeSize();
            return pushConstantRange.size > 0;
        }

        vk::ShaderStageFlagBits getPushConstantStage() override {
            return vk::ShaderStageFlagBits::eCompute;
        }

    private:
        vk::Extent3D m_workGroupCount = {0, 0, 0};
    };
} // namespace raven