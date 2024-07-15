#pragma once

#include "raven/passes/PassShaderCompute.h"
#include "raven/util/Paths.h"

namespace raven {
    class PassPathtrace final : public PassShaderCompute {
    public:
        explicit PassPathtrace(GPUContext *gpuContext) : PassShaderCompute(gpuContext, "PassPathtrace") {
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

    protected:
        std::vector<std::shared_ptr<Shader>> createShaders() override {
            constexpr auto workGroupSize = vk::Extent3D{8, 8, 1};
            auto specializationMapEntries = std::vector{vk::SpecializationMapEntry{0, 0, sizeof(uint32_t)},
                                                        vk::SpecializationMapEntry{1, 4, sizeof(uint32_t)},
                                                        vk::SpecializationMapEntry{2, 8, sizeof(uint32_t)}};
            auto specializationData = std::vector{workGroupSize.width, workGroupSize.height, workGroupSize.depth};
            return {std::make_shared<Shader>(m_gpuContext, Paths::m_resourceDirectoryPath + "/shaders/main/passes", "pass_pathtrace.comp", vk::ShaderStageFlagBits::eCompute, workGroupSize, specializationMapEntries, specializationData)};
        }

        uint32_t getPushConstantRangeSize() override {
            return sizeof(PushConstants);
        }
    };
} // namespace raven