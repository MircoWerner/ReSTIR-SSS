#pragma once

#include "Raydata.h"
#include "Rayscenes.h"
#include "passes/PassDebug.h"
#include "passes/PassGBuffer.h"
#include "passes/PassPathtrace.h"
#include "passes/PassSurface.h"
#include "passes/PassTonemapper.h"
#include "passes/ReSTIRSSSPassCandidateGeneration.h"
#include "passes/ReSTIRSSSPassShade.h"
#include "passes/ReSTIRSSSPassSpatialReuse.h"
#include "passes/ReSTIRSSSPassTemporalReuse.h"
#include "raven/core/AccelerationStructure.h"
#include "raven/core/Image.h"
#include "raven/core/Renderer.h"
#include "raven/core/Uniform.h"
#include "raven/passes/PassCompute.h"
#include "raven/util/animation/BSpline.h"

#ifdef OPTIX_SUPPORT
#include "denoiser/DenoiserOptiX.h"
#endif

// #define RESTIRSSS_WRITE_DEBUG_IMAGES

namespace raven {
    class ReSTIRSSS final : public Renderer {
    public:
#define RENDERER_RESTIRSSS 0
#define RENDERER_DEBUG 1
#define RENDERER_PATHTRACE 2

#define RESTIRSSS_SHIFT_RECONNECTION 0
#define RESTIRSSS_SHIFT_DELAYED_RECONNECTION 1
#define RESTIRSSS_SHIFT_HYBRID 2
#define RESTIRSSS_SHIFT_SEQUENTIAL 3

        struct ReSTIRSSSSettings {
            // SCENE
            Rayscenes::EScene m_scene = Rayscenes::UNDEFINED;
            std::optional<uint32_t> m_sceneMode;
            std::optional<glm::vec3> m_position;
            std::optional<glm::vec3> m_orientation;
            std::optional<float> m_speed;

            // RENDERING
            std::optional<uint32_t> m_maxFrames;
            std::optional<uint32_t> m_renderer;
            std::optional<uint32_t> m_debug;
            std::optional<uint32_t> m_tonemapper;
            std::optional<uint32_t> m_nonSSObjects;
            std::optional<uint32_t> m_spp;
            std::optional<bool> m_denoiser;

            // ReSTIR SSS
            std::optional<bool> m_restir_non_sss_surfaces;

            std::optional<uint32_t> m_candidate_samples_light;
            std::optional<uint32_t> m_candidate_samples_bsdf;
            std::optional<bool> m_candidate_sss_all_intersections;

            std::optional<uint32_t> m_shift;

            std::optional<bool> m_temporal;
            std::optional<uint32_t> m_temporal_shift_sequential; // only relevant when sequential shifts selected

            std::optional<bool> m_spatial;
            std::optional<uint32_t> m_spatial_count;
            std::optional<uint32_t> m_spatial_shift_sequential; // only relevant when sequential shifts selected

            std::optional<bool> m_spatial2;
            std::optional<uint32_t> m_spatial2_count;
            std::optional<uint32_t> m_spatial2_shift_sequential; // only relevant when sequential shifts selected

            // CAMERA
            std::optional<BSpline> m_cameraBSplinePosition;
            std::optional<BSpline> m_cameraBSplineOrientation;
            std::optional<float> m_cameraBSplineU;
            std::optional<float> m_cameraBSplineSpeed;
        };

        ReSTIRSSS(ReSTIRSSSSettings settings, std::optional<std::string> sceneName = std::nullopt, std::optional<std::string> preset = std::nullopt);

        void initResources(GPUContext *gpuContext) override;
        void initShaderResources(GPUContext *gpuContext) override;
        void initSwapchainResources(GPUContext *gpuContext, vk::Extent2D extent) override;
        void recordImGui(GPUContext *gpuContext, Camera *camera) override;
        void update(GPUContext *gpuContext, Camera *camera, float deltaTime) override;
        void render(GPUContext *gpuContext, Camera *camera, uint32_t activeIndex, RendererResult *rendererResult) override;
        void releaseResources() override;
        void releaseShaderResources() override;
        void releaseSwapchainResources() override;

        [[nodiscard]] float getGBufferTimeAveraged() const {
            return m_passGBuffer->getTimeAveraged();
        }
        [[nodiscard]] float getCandidateGenerationTimeAveraged() const {
            return m_ReSTIRSSSPassCandidateGeneration->getTimeAveraged();
        }
        [[nodiscard]] float getTemporalReuseTimeAveraged() const {
            return m_ReSTIRSSSPassTemporalReuse->getTimeAveraged();
        }
        [[nodiscard]] float getSpatialReuseTimeAveraged() const {
            return m_ReSTIRSSSPassSpatialReuse->getTimeAveraged();
        }
        [[nodiscard]] float getSpatialReuse2TimeAveraged() const {
            return m_ReSTIRSSSPassSpatialReuse2->getTimeAveraged();
        }
        [[nodiscard]] float getShadeTimeAveraged() const {
            return m_ReSTIRSSSPassShade->getTimeAveraged();
        }
        [[nodiscard]] float getPathtraceTimeAveraged() const {
            return m_passPathtrace->getTimeAveraged();
        }

    private:
        ReSTIRSSSSettings m_settings;
        std::optional<std::string> m_sceneName;
        std::optional<std::string> m_preset;

        std::shared_ptr<Image> m_imageFramebuffer;
        std::shared_ptr<Image> m_imageAccumulationBuffer;
        std::shared_ptr<Image> m_imageAlbedoBuffer;
        std::shared_ptr<Image> m_imageNormalBuffer;
#ifdef RESTIRSSS_WRITE_DEBUG_IMAGES
        std::array<std::shared_ptr<Image>, 3> m_imageReSTIRDebug;
#endif

        std::shared_ptr<PassCompute> m_preProcessingPass;
        std::shared_ptr<PassGBuffer> m_passGBuffer;
        std::shared_ptr<PassSurface> m_passSurface;
        std::shared_ptr<Uniform> m_uniformPassGBuffer;
        std::shared_ptr<Uniform> m_uniformPassSurface;
        std::shared_ptr<PassDebug> m_passDebug;
        std::shared_ptr<Uniform> m_uniformPassDebug;

        std::shared_ptr<PassCompute> m_ReSTIRSSSPass;
        std::shared_ptr<ReSTIRSSSPassCandidateGeneration> m_ReSTIRSSSPassCandidateGeneration;
        std::shared_ptr<ReSTIRSSSPassTemporalReuse> m_ReSTIRSSSPassTemporalReuse;
        std::shared_ptr<ReSTIRSSSPassSpatialReuse> m_ReSTIRSSSPassSpatialReuse;
        std::shared_ptr<ReSTIRSSSPassSpatialReuse> m_ReSTIRSSSPassSpatialReuse2;
        std::shared_ptr<ReSTIRSSSPassShade> m_ReSTIRSSSPassShade;
        std::shared_ptr<Uniform> m_uniformReSTIRSSSPassCandidateGeneration;
        std::shared_ptr<Uniform> m_uniformReSTIRSSSPassTemporalReuse;
        std::shared_ptr<Uniform> m_uniformReSTIRSSSPassSpatialReuse;
        std::shared_ptr<Uniform> m_uniformReSTIRSSSPassSpatialReuse2;
        std::shared_ptr<Uniform> m_uniformReSTIRSSSPassShade;
        std::shared_ptr<PassPathtrace> m_passPathtrace;
        std::shared_ptr<Uniform> m_uniformPassPathtrace;

        std::shared_ptr<PassCompute> m_postProcessingPass;
        std::shared_ptr<PassTonemapper> m_passTonemapper;

#ifdef OPTIX_SUPPORT
        std::unique_ptr<DenoiserOptiX> m_denoiser;
#endif

        std::array<std::shared_ptr<Image>, 2> m_imageGBuffer;

#define RESTIR_SAMPLE_INVALID 0xFFFFFFFF
#define INVALID_UINT_VALUE 0xFFFFFFFFu

        std::array<std::shared_ptr<Image>, 8> m_imageReSTIRReservoirBuffer;

        struct RenderOptions {
            uint32_t m_time = 0; // total number of executions/frames since application start

            glm::ivec2 m_pixels;

            uint32_t m_frame = 0;
            uint32_t m_maxFrames = 1024;
            uint32_t m_validActiveIndex = 0;

            bool m_framePreviousValid = false;

            uint32_t m_tonemapper = 1;

            bool m_normalMapping = false;
            bool m_sss = true;

            bool m_nonSSObjects = true;

            uint32_t m_renderer = RENDERER_RESTIRSSS; // 0 ReSTIRSSS, 1 ReSTIRSSSDebug, 2 ReSTIRSSSPathtrace

            uint32_t m_debug = 0; // for ReSTIRSSSDebug
            uint32_t m_spp = 1;   // for ReSTIRSSSPathtrace

            glm::mat4 m_vp = glm::mat4(1.f);
            glm::mat4 m_vp_prev = glm::mat4(1.f);

            bool m_denoiser = false;
            bool m_denoiserGuideAlbedo = true;
            bool m_denoiserGuideNormal = true;

            glm::vec3 m_rayOrigin;
            glm::vec3 m_rayLeftBottom;
            glm::vec3 m_rayLeftTop;
            glm::vec3 m_rayRightBottom;
            glm::vec3 m_rayRightTop;

            uint32_t m_numLights = 0;

            uint32_t m_gBufferIndex = 0;

            void swapGBufferIndex() {
                m_gBufferIndex = (m_gBufferIndex + 1) % 2;
            }

            [[nodiscard]] uint32_t getGBufferThisFrameIndex() const {
                return m_gBufferIndex;
            }
            [[nodiscard]] uint32_t getGBufferLastFrameIndex() const {
                return 1 - m_gBufferIndex;
            }
        };
        RenderOptions m_renderOptions{};

        struct ReSTIROptions {
            bool m_restir_non_sss_surfaces = false;

            uint32_t m_candidate_samples_light = 1;
            uint32_t m_candidate_samples_bsdf = 0;
            bool m_candidate_sss_all_intersections = true;

            uint32_t m_shift = RESTIRSSS_SHIFT_HYBRID;

            bool m_temporal = false;
            uint32_t m_temporal_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION; // only relevant when sequential shifts selected

            bool m_spatial = false;
            uint32_t m_spatial_count = 4;
            uint32_t m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION; // only relevant when sequential shifts selected

            bool m_spatial2 = false;
            uint32_t m_spatial2_count = 2;
            uint32_t m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION; // only relevant when sequential shifts selected

            uint32_t m_restir_debug = 0;

            uint32_t m_reservoirsInIndex = 0;

            void swapReservoirIndex() {
                m_reservoirsInIndex = (m_reservoirsInIndex + 1) % 2;
            }

            [[nodiscard]] uint32_t getReservoirReadIndex() const {
                return m_reservoirsInIndex;
            }
            [[nodiscard]] uint32_t getReservoirWriteIndex() const {
                return 1 - m_reservoirsInIndex;
            }
        };
        ReSTIROptions m_reSTIROptions{};

        Raydata m_raydata;
        Rayscenes m_rayscenes;

        void resetFrame(GPUContext *gpuContext);

        void screenshot(GPUContext *gpuContext, std::shared_ptr<Image> &image, const std::string &path) const;

        void passesSetMaterialBuffer() const;
        void passesSetLightBuffer() const;

        void createDenoiserEngine(GPUContext *gpuContext, vk::Extent2D extent);
        void releaseDenoiserEngine() const;

        void loadPreset(const std::string &preset);

        static bool vec3FromString(const std::string &str, glm::vec3 *position) {
            const std::regex rgx("([+-]?[0-9]*[.]?[0-9]+) ([+-]?[0-9]*[.]?[0-9]+) ([+-]?[0-9]*[.]?[0-9]+)");
            std::smatch matches;
            std::regex_search(str, matches, rgx);
            if (matches.size() == 4) {
                position->x = std::stof(matches[1]);
                position->y = std::stof(matches[2]);
                position->z = std::stof(matches[3]);
                return true;
            }
            return false;
        }

        BSpline m_cameraBSplinePosition;
        BSpline m_cameraBSplineOrientation;
        float m_cameraBSplineU = FLT_MAX;
        float m_cameraBSplineSpeed = 1.f;
    };
} // namespace raven