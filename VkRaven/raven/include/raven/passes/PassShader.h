#pragma once
#include "raven/core/Image.h"
#include "raven/core/Shader.h"

#include <memory>
#include <ranges>
#include <utility>
#include <vector>

namespace raven {
#define RAVEN_PASS_SHADER_RELEASE(passShader) \
    if (passShader)                           \
    passShader->release()

    class PassShader {
    public:
        PassShader(GPUContext *gpuContext, std::string name) : m_gpuContext(gpuContext), m_name(std::move(name)) {
        }

        virtual ~PassShader() = default;

        virtual void execute(const std::function<void(const vk::CommandBuffer &commandBuffer, const vk::PipelineLayout &pipelineLayout)> &recordPushConstants,
                             const std::function<void(const vk::CommandBuffer &commandBuffer)> &recordPipelineBarriers,
                             vk::CommandBuffer &commandBuffer) = 0;

        virtual void create() {
            m_shaders = createShaders();
            createDescriptorSetLayout();
            createDescriptorPool();
            createDescriptorSets();
            createPipelineLayout();
            createPipeline();

            createUniforms();
            createTimestampQueryPool();
        }

        virtual void release() {
            m_gpuContext->m_device.destroyQueryPool(m_queryPoolTimestamps);

            releaseUniforms();

            vkDestroyDescriptorPool(m_gpuContext->m_device, m_descriptorPool, nullptr);
            for (auto &descriptorSetLayout: m_descriptorSetLayouts) {
                vkDestroyDescriptorSetLayout(m_gpuContext->m_device, descriptorSetLayout, nullptr);
            }
            m_gpuContext->m_device.destroyPipeline(m_pipeline);
            vkDestroyPipelineLayout(m_gpuContext->m_device, m_pipelineLayout, nullptr);
            for (const auto &shader: m_shaders) {
                shader->release();
            }
        }

        void updateSpecializationConstants(const int32_t shaderIndex, std::vector<vk::SpecializationMapEntry> specializationMapEntries, std::vector<uint32_t> specializationData) {
            m_shaders[shaderIndex]->updateSpecializationConstants(std::move(specializationMapEntries), std::move(specializationData));
            m_gpuContext->m_device.destroyPipeline(m_pipeline);
            createPipeline();
        }

        std::string getName() { return m_name; }

        void setStorageBuffer(const uint32_t set, const uint32_t binding, Buffer *buffer) {
            const uint32_t setIdx = m_descriptorSetToIndex[set];

            auto &[set_number, create_info, bindings, bindingToIndex] = m_descriptorSetLayoutData[setIdx];
            const auto &bindingLayout = bindings[bindingToIndex[binding]];
            assert(set == set_number);
            assert(binding == bindingLayout.binding);

            vk::DescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = buffer->getBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = buffer->getSizeBytes();

            vk::WriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.dstBinding = binding;
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.descriptorType = bindingLayout.descriptorType;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.pBufferInfo = &bufferInfo;

            for (size_t i = 0; i < m_gpuContext->getMultiBufferedCount(); i++) {
                writeDescriptorSet.dstSet = m_descriptorSets[i][setIdx];
                m_gpuContext->m_device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
            }
        }

        void setStorageImage(const uint32_t set, const uint32_t binding, Image *image) {
            const uint32_t setIdx = m_descriptorSetToIndex[set];

            auto &[set_number, create_info, bindings, bindingToIndex] = m_descriptorSetLayoutData[setIdx];
            const auto &bindingLayout = bindings[bindingToIndex[binding]];
            assert(set == set_number);
            assert(binding == bindingLayout.binding);

            vk::DescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = vk::ImageLayout::eGeneral;
            imageInfo.imageView = image->getImageView();
            imageInfo.sampler = image->getSampler();

            vk::WriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.dstBinding = binding;
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.descriptorType = bindingLayout.descriptorType;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.pImageInfo = &imageInfo;

            for (size_t i = 0; i < m_gpuContext->getMultiBufferedCount(); i++) {
                writeDescriptorSet.dstSet = m_descriptorSets[i][setIdx];
                m_gpuContext->m_device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
            }
        }

        void setStorageImages(const uint32_t set, const uint32_t binding, const std::vector<Image *> &images) {
            const uint32_t setIdx = m_descriptorSetToIndex[set];

            auto &[set_number, create_info, bindings, bindingToIndex] = m_descriptorSetLayoutData[setIdx];
            const auto &bindingLayout = bindings[bindingToIndex[binding]];
            assert(set == set_number);
            assert(binding == bindingLayout.binding);

            std::vector<vk::DescriptorImageInfo> imageInfos(images.size());
            for (uint32_t i = 0; i < images.size(); i++) {
                imageInfos[i].imageLayout = vk::ImageLayout::eGeneral;
                imageInfos[i].imageView = images[i]->getImageView();
                imageInfos[i].sampler = images[i]->getSampler();
            }

            vk::WriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.dstBinding = binding;
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.descriptorType = bindingLayout.descriptorType;
            writeDescriptorSet.descriptorCount = images.size();
            writeDescriptorSet.pImageInfo = imageInfos.data();

            for (size_t i = 0; i < m_gpuContext->getMultiBufferedCount(); i++) {
                writeDescriptorSet.dstSet = m_descriptorSets[i][setIdx];
                m_gpuContext->m_device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
            }
        }

        void setStorageImage(const uint32_t set, const uint32_t binding, Image *image, const uint32_t activeIndex) {
            const uint32_t setIdx = m_descriptorSetToIndex[set];

            auto &[set_number, create_info, bindings, bindingToIndex] = m_descriptorSetLayoutData[setIdx];
            const auto &bindingLayout = bindings[bindingToIndex[binding]];
            assert(set == set_number);
            assert(binding == bindingLayout.binding);

            vk::DescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = vk::ImageLayout::eGeneral;
            imageInfo.imageView = image->getImageView();
            imageInfo.sampler = image->getSampler();

            vk::WriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.dstBinding = binding;
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.descriptorType = bindingLayout.descriptorType;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.pImageInfo = &imageInfo;

            writeDescriptorSet.dstSet = m_descriptorSets[activeIndex][setIdx];
            m_gpuContext->m_device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
        }

        void setSamplerImage(const uint32_t set, const uint32_t binding, Image *image, const vk::ImageLayout imageLayout = vk::ImageLayout::eGeneral) {
            const uint32_t setIdx = m_descriptorSetToIndex[set];

            auto &[set_number, create_info, bindings, bindingToIndex] = m_descriptorSetLayoutData[setIdx];
            const auto &bindingLayout = bindings[bindingToIndex[binding]];
            assert(set == set_number);
            assert(binding == bindingLayout.binding);

            vk::DescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = imageLayout;
            imageInfo.imageView = image->getImageView();
            imageInfo.sampler = image->getSampler();

            vk::WriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.dstBinding = binding;
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.descriptorType = bindingLayout.descriptorType;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.pImageInfo = &imageInfo;

            for (size_t i = 0; i < m_gpuContext->getMultiBufferedCount(); i++) {
                writeDescriptorSet.dstSet = m_descriptorSets[i][setIdx];
                m_gpuContext->m_device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
            }
        }

        void setSamplerImages(const uint32_t set, const uint32_t binding, const std::vector<Image *> &images, const vk::ImageLayout imageLayout = vk::ImageLayout::eGeneral) {
            const uint32_t setIdx = m_descriptorSetToIndex[set];

            auto &[set_number, create_info, bindings, bindingToIndex] = m_descriptorSetLayoutData[setIdx];
            const auto &bindingLayout = bindings[bindingToIndex[binding]];
            assert(set == set_number);
            assert(binding == bindingLayout.binding);

            std::vector<vk::DescriptorImageInfo> imageInfos(images.size());
            for (uint32_t i = 0; i < images.size(); i++) {
                imageInfos[i].imageLayout = imageLayout;
                imageInfos[i].imageView = images[i]->getImageView();
                imageInfos[i].sampler = images[i]->getSampler();
            }

            vk::WriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.dstBinding = binding;
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.descriptorType = bindingLayout.descriptorType;
            writeDescriptorSet.descriptorCount = images.size();
            writeDescriptorSet.pImageInfo = imageInfos.data();

            for (size_t i = 0; i < m_gpuContext->getMultiBufferedCount(); i++) {
                writeDescriptorSet.dstSet = m_descriptorSets[i][setIdx];
                m_gpuContext->m_device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
            }
        }

        void setSamplerImage(const uint32_t set, const uint32_t binding, Image *image, const uint32_t activeIndex, const vk::ImageLayout imageLayout = vk::ImageLayout::eGeneral) {
            const uint32_t setIdx = m_descriptorSetToIndex[set];

            auto &[set_number, create_info, bindings, bindingToIndex] = m_descriptorSetLayoutData[setIdx];
            const auto &bindingLayout = bindings[bindingToIndex[binding]];
            assert(set == set_number);
            assert(binding == bindingLayout.binding);

            vk::DescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = imageLayout;
            imageInfo.imageView = image->getImageView();
            imageInfo.sampler = image->getSampler();

            vk::WriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.dstBinding = binding;
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.descriptorType = bindingLayout.descriptorType;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.pImageInfo = &imageInfo;

            writeDescriptorSet.dstSet = m_descriptorSets[activeIndex][setIdx];
            m_gpuContext->m_device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
        }

        void setAccelerationStructure(const uint32_t set, const uint32_t binding, const vk::AccelerationStructureKHR &as) {
            const uint32_t setIdx = m_descriptorSetToIndex[set];

            auto &[set_number, create_info, bindings, bindingToIndex] = m_descriptorSetLayoutData[setIdx];
            const auto &bindingLayout = bindings[bindingToIndex[binding]];
            assert(set == set_number);
            assert(binding == bindingLayout.binding);

            vk::WriteDescriptorSetAccelerationStructureKHR descriptorSetAS{};
            descriptorSetAS.accelerationStructureCount = 1;
            descriptorSetAS.pAccelerationStructures = &as;

            vk::WriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.dstBinding = binding;
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.descriptorType = bindingLayout.descriptorType;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.pNext = &descriptorSetAS;

            for (size_t i = 0; i < m_gpuContext->getMultiBufferedCount(); i++) {
                writeDescriptorSet.dstSet = m_descriptorSets[i][setIdx];
                m_gpuContext->m_device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
            }
        }

        std::shared_ptr<Uniform> getUniform(const uint32_t set, const uint32_t binding) { return m_uniforms[set][binding]; }

        [[nodiscard]] float getTime() const { return m_timeValid ? m_timeInMs : NAN; }
        [[nodiscard]] float getTimeAveraged() const { return m_timeValid ? m_timeInMsAveraged : NAN; }

        void invalidateTime() { m_timeValid = false; }

    protected:
        GPUContext *m_gpuContext;
        std::string m_name;

        std::vector<std::shared_ptr<Shader>> m_shaders; // either one compute shader or vertex/framgment shader pair or raygen/closesthit/miss/... shaders touple

        vk::PipelineLayout m_pipelineLayout{};
        vk::Pipeline m_pipeline{};

        vk::DescriptorPool m_descriptorPool{};
        std::map<uint32_t, uint32_t> m_descriptorSetToIndex;                    // m_descriptorSetToIndex[setId] - mapping of set number to index in the following vectors
        std::vector<Shader::DescriptorSetLayoutData> m_descriptorSetLayoutData; // m_descriptorSetLayoutData[m_descriptorSetToIndex[setId]] - reflected bindings
        std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts{};          // m_descriptorSetLayouts[m_descriptorSetToIndex[setId]]
        std::vector<std::vector<vk::DescriptorSet>> m_descriptorSets;           // m_descriptorSets[multibufferedId][m_descriptorSetToIndex[setId]] - destroyed implicitly with the descriptor pool

        std::map<uint32_t, std::map<uint32_t, std::shared_ptr<Uniform>>> m_uniforms; // m_uniforms[setId][bindingId]

        vk::QueryPool m_queryPoolTimestamps;
        std::array<uint64_t, 2> m_timestamps = {0, 0};
        bool m_timestampsAvailable = false;
        float m_timeInMs = 0;
        float m_timeInMsAveraged = 0;
        bool m_timeValid = false;

        virtual std::vector<std::shared_ptr<Shader>> createShaders() = 0;

        virtual void createPipeline() = 0;

        virtual uint32_t getPushConstantRangeSize() {
            return 0;
        }

        virtual vk::ShaderStageFlagBits getPushConstantStage() = 0;

        virtual bool getPushConstantRange(vk::PushConstantRange &pushConstantRange) = 0;

        void getDescriptorSets(std::vector<vk::DescriptorSet> &sets) const {
            sets.resize(m_descriptorSets[m_gpuContext->getActiveIndex()].size());
            for (uint32_t i = 0; i < m_descriptorSets[m_gpuContext->getActiveIndex()].size(); i++) {
                sets[i] = m_descriptorSets[m_gpuContext->getActiveIndex()][i];
            }
        }

        void queryTimeFromGPU() {
            if (m_timestampsAvailable) {
                if (m_gpuContext->m_device.getQueryPoolResults(m_queryPoolTimestamps, 0, 2, sizeof(uint64_t) * 2, m_timestamps.data(), sizeof(uint64_t),
                                                               vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait) == vk::Result::eSuccess) {
                    const float timeInMs = static_cast<float>(m_timestamps[1] - m_timestamps[0]) * m_gpuContext->getDeviceLimitsTimestampPeriod() / 1000000.0f;
                    m_timeInMs = timeInMs;
                    m_timeInMsAveraged = m_timeValid ? 0.9f * m_timeInMsAveraged + 0.1f * timeInMs : timeInMs;
                    m_timeValid = true;
                } else {
                    m_timeValid = false;
                }
            } else {
                m_timeValid = false;
            }
            m_timestampsAvailable = true;
        }

    private:
        void createPipelineLayout() {
            vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.setLayoutCount = m_descriptorSetLayouts.size(); // optional
            pipelineLayoutInfo.pSetLayouts = m_descriptorSetLayouts.data();    // optional

            vk::PushConstantRange pushConstantRange{};
            if (getPushConstantRange(pushConstantRange)) {
                pipelineLayoutInfo.pushConstantRangeCount = 1;               // optional
                pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange; // optional
            } else {
                pipelineLayoutInfo.pushConstantRangeCount = 0;    // optional
                pipelineLayoutInfo.pPushConstantRanges = nullptr; // optional
            }

            if (m_gpuContext->m_device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_pipelineLayout) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create pipeline layout!");
            }
        }

        void createDescriptorSetLayout() {
            // retrieve information using SPIRV-reflect
            for (const auto &shader: m_shaders) {
                // for every shader
                for (const auto &[fst, snd]: *shader->getDescriptorSetLayoutData()) {
                    // retrieve all sets
                    uint32_t setId = fst;
                    const auto &[set_number, create_info, bindings, bindingToIndex] = snd;

                    if (m_descriptorSetToIndex.contains(setId)) {
                        // merge with existing set
                        const uint32_t index = m_descriptorSetToIndex[setId];
                        auto &[set_number, create_info, bindings, bindingToIndex] = m_descriptorSetLayoutData[index];
                        assert(setId == set_number);
                        bindings.insert(bindings.end(), bindings.begin(), bindings.end());
                        //                        for (auto &binding : layout.bindings) {
                        //
                        //                        }
                    } else {
                        // insert new set
                        const uint32_t index = m_descriptorSetLayoutData.size();
                        m_descriptorSetToIndex[setId] = index;
                        Shader::DescriptorSetLayoutData newLayoutData{.set_number = setId, .bindings = bindings};
                        m_descriptorSetLayoutData.push_back(newLayoutData);
                    }
                }

                for (const auto &uniforms: std::ranges::views::values(*shader->getUniforms())) {
                    for (const auto &uniform: uniforms) {
                        m_uniforms[uniform->getSet()][uniform->getBinding()] = uniform;
                    }
                }
            }

            for (auto &layoutData: m_descriptorSetLayoutData) {
                layoutData.create_info.bindingCount = layoutData.bindings.size();
                layoutData.create_info.pBindings = layoutData.bindings.data();

                layoutData.generateBindingToIndexMap();
            }

            m_descriptorSetLayouts.resize(m_descriptorSetLayoutData.size());
            for (auto &[set_number, create_info, bindings, bindingToIndex]: m_descriptorSetLayoutData) {
                if (const uint32_t index = m_descriptorSetToIndex[set_number]; m_gpuContext->m_device.createDescriptorSetLayout(&create_info, nullptr, &m_descriptorSetLayouts[index]) != vk::Result::eSuccess) {
                    throw std::runtime_error("Failed to create descriptor set layout!");
                }
            }
        }

        void createDescriptorPool() {
            // retrieve information using SPIRV-reflect
            std::map<vk::DescriptorType, uint32_t> descriptorCounts;
            for (const auto &[set_number, create_info, bindings, bindingToIndex]: m_descriptorSetLayoutData) {
                for (const auto &binding: bindings) {
                    if (descriptorCounts.contains(binding.descriptorType)) {
                        descriptorCounts[binding.descriptorType] += binding.descriptorCount;
                    } else {
                        descriptorCounts[binding.descriptorType] = binding.descriptorCount;
                    }
                }
            }

            std::vector<vk::DescriptorPoolSize> poolSizes;
            for (const auto &[descriptorType, count]: descriptorCounts) {
                poolSizes.emplace_back(descriptorType, count * m_gpuContext->getMultiBufferedCount());
            }

            vk::DescriptorPoolCreateInfo poolInfo{};
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = m_gpuContext->getMultiBufferedCount() * m_descriptorSetLayouts.size();

            if (m_gpuContext->m_device.createDescriptorPool(&poolInfo, nullptr, &m_descriptorPool) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create descriptor pool!");
            }
        }

        void createDescriptorSets() {
            // allocate descriptor sets (valid values will be set later)
            m_descriptorSets.resize(m_gpuContext->getMultiBufferedCount());
            for (uint32_t i = 0; i < m_gpuContext->getMultiBufferedCount(); i++) {
                vk::DescriptorSetAllocateInfo allocInfo{};
                allocInfo.descriptorPool = m_descriptorPool;
                allocInfo.descriptorSetCount = m_descriptorSetLayouts.size();
                allocInfo.pSetLayouts = m_descriptorSetLayouts.data();

                m_descriptorSets[i].resize(m_descriptorSetLayouts.size());
                if (m_gpuContext->m_device.allocateDescriptorSets(&allocInfo, m_descriptorSets[i].data()) != vk::Result::eSuccess) {
                    throw std::runtime_error("Failed to allocate descriptor sets!");
                }
            }
        }

        void createUniforms() {
            for (const auto &uniforms: std::ranges::views::values(m_uniforms)) {
                for (const auto &uniform: std::ranges::views::values(uniforms)) {
                    uniform->create();
                    setUniform(uniform.get());
                }
            }
        }

        void releaseUniforms() {
            for (const auto &uniforms: std::ranges::views::values(m_uniforms)) {
                for (const auto &uniform: std::ranges::views::values(uniforms)) {
                    uniform->release();
                }
            }
        }

        void setUniform(Uniform *uniform) {
            const uint32_t setIdx = m_descriptorSetToIndex[uniform->getSet()];

            auto &[set_number, create_info, bindings, bindingToIndex] = m_descriptorSetLayoutData[setIdx];
            const auto &bindingLayout = bindings[bindingToIndex[uniform->getBinding()]];

            assert(uniform->getSet() == set_number);
            assert(uniform->getBinding() == bindingLayout.binding);

            vk::WriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.dstBinding = uniform->getBinding();
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.descriptorType = bindingLayout.descriptorType;
            writeDescriptorSet.descriptorCount = 1;

            for (uint32_t i = 0; i < m_gpuContext->getMultiBufferedCount(); i++) {
                vk::DescriptorBufferInfo bufferInfo{};
                uniform->generateBufferInfo(&bufferInfo, i);

                writeDescriptorSet.dstSet = m_descriptorSets[i][setIdx];
                writeDescriptorSet.pBufferInfo = &bufferInfo;
                m_gpuContext->m_device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
            }
        }

        void createTimestampQueryPool() {
            vk::QueryPoolCreateInfo queryPoolCreateInfo{};
            queryPoolCreateInfo.sType = vk::StructureType::eQueryPoolCreateInfo;
            queryPoolCreateInfo.queryType = vk::QueryType::eTimestamp;
            queryPoolCreateInfo.queryCount = static_cast<uint32_t>(m_timestamps.size());
            if (m_gpuContext->m_device.createQueryPool(&queryPoolCreateInfo, nullptr, &m_queryPoolTimestamps) != vk::Result::eSuccess) {
                throw std::runtime_error("Cannot create query pool!");
            }
        }
    };
} // namespace raven