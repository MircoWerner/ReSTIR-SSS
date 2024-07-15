#pragma once

#include "raven/passes/PassShaderCompute.h"
#include "raven/util/Paths.h"

namespace raven {
    class ReSTIRSSSPassSpatialReuse final : public PassShaderCompute {
    public:
        explicit ReSTIRSSSPassSpatialReuse(GPUContext *gpuContext, const uint32_t spatialCount) : PassShaderCompute(gpuContext, "ReSTIRSSSPassSpatialReuse"), m_spatialCount(spatialCount) {
        }

        struct PushConstants {
            glm::ivec2 g_pixels;

            glm::vec3 g_ray_origin;
            glm::vec3 g_ray_left_bottom;
            glm::vec3 g_ray_left_top;
            glm::vec3 g_ray_right_bottom;
            glm::vec3 g_ray_right_top;

            uint32_t g_frame = 0;

            uint32_t g_rng_init;
            uint32_t g_rng_init_offset;
        };

        PushConstants m_pushConstants = {};

        void updateSpecializationConstant3(const uint32_t value) {
            m_spatialCount = value;
            auto specializationMapEntries = std::vector{vk::SpecializationMapEntry{0, 0, sizeof(uint32_t)},
                                                        vk::SpecializationMapEntry{1, 4, sizeof(uint32_t)},
                                                        vk::SpecializationMapEntry{2, 8, sizeof(uint32_t)},
                                                        vk::SpecializationMapEntry{3, 12, sizeof(uint32_t)}};
            auto specializationData = std::vector{workGroupSize.width, workGroupSize.height, workGroupSize.depth, m_spatialCount};
            updateSpecializationConstants(0, specializationMapEntries, specializationData);
        }

    protected:
        const vk::Extent3D workGroupSize = vk::Extent3D{8, 8, 1};
        uint32_t m_spatialCount = 0;

        std::vector<std::shared_ptr<Shader>> createShaders() override {
            auto specializationMapEntries = std::vector{vk::SpecializationMapEntry{0, 0, sizeof(uint32_t)},
                                                        vk::SpecializationMapEntry{1, 4, sizeof(uint32_t)},
                                                        vk::SpecializationMapEntry{2, 8, sizeof(uint32_t)},
                                                        vk::SpecializationMapEntry{3, 12, sizeof(uint32_t)}};
            auto specializationData = std::vector<uint32_t>{workGroupSize.width, workGroupSize.height, workGroupSize.depth, m_spatialCount};
            return {std::make_shared<Shader>(m_gpuContext, Paths::m_resourceDirectoryPath + "/shaders/main/passes", "restirsss_pass_spatial_reuse.comp", vk::ShaderStageFlagBits::eCompute, workGroupSize, specializationMapEntries, specializationData)};
        }

        uint32_t getPushConstantRangeSize() override {
            return sizeof(PushConstants);
        }
    };
} // namespace raven