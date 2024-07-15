#include "restirsss/ReSTIRSSS.h"

#include "glm/gtx/common.inl"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "pugixml/src/pugixml.hpp"
#include "raven/util/ImagePFM.h"
#include "stb/stb_image_write.h"
#include "tinygltf/tiny_gltf.h"

#include <complex>
#include <random>
#include <utility>

#include "raven/core/Application.h"

namespace raven {
    ReSTIRSSS::ReSTIRSSS(ReSTIRSSSSettings settings, std::optional<std::string> sceneName, std::optional<std::string> preset) : m_settings(std::move(settings)), m_sceneName(std::move(sceneName)), m_preset(std::move(preset)) {
        if (m_settings.m_maxFrames.has_value())
            m_renderOptions.m_maxFrames = m_settings.m_maxFrames.value();
        if (m_settings.m_renderer.has_value())
            m_renderOptions.m_renderer = m_settings.m_renderer.value();
        if (m_settings.m_debug.has_value())
            m_renderOptions.m_debug = m_settings.m_debug.value();
        if (m_settings.m_tonemapper.has_value())
            m_renderOptions.m_tonemapper = m_settings.m_tonemapper.value();
        if (m_settings.m_nonSSObjects.has_value())
            m_renderOptions.m_nonSSObjects = m_settings.m_nonSSObjects.value();
        if (m_settings.m_spp.has_value())
            m_renderOptions.m_spp = m_settings.m_spp.value();
        if (m_settings.m_denoiser.has_value())
            m_renderOptions.m_denoiser = m_settings.m_denoiser.value();
        if (m_settings.m_candidate_samples_light.has_value())
            m_reSTIROptions.m_candidate_samples_light = m_settings.m_candidate_samples_light.value();
        if (m_settings.m_candidate_samples_bsdf.has_value())
            m_reSTIROptions.m_candidate_samples_bsdf = m_settings.m_candidate_samples_bsdf.value();
        if (m_settings.m_candidate_sss_all_intersections.has_value())
            m_reSTIROptions.m_candidate_sss_all_intersections = m_settings.m_candidate_sss_all_intersections.value();
        if (m_settings.m_shift.has_value())
            m_reSTIROptions.m_shift = m_settings.m_shift.value();
        if (m_settings.m_temporal.has_value())
            m_reSTIROptions.m_temporal = m_settings.m_temporal.value();
        if (m_settings.m_temporal_shift_sequential.has_value())
            m_reSTIROptions.m_temporal_shift_sequential = m_settings.m_temporal_shift_sequential.value();
        if (m_settings.m_spatial.has_value())
            m_reSTIROptions.m_spatial = m_settings.m_spatial.value();
        if (m_settings.m_spatial_count.has_value())
            m_reSTIROptions.m_spatial_count = m_settings.m_spatial_count.value();
        if (m_settings.m_spatial_shift_sequential.has_value())
            m_reSTIROptions.m_spatial_shift_sequential = m_settings.m_spatial_shift_sequential.value();
        if (m_settings.m_spatial2.has_value())
            m_reSTIROptions.m_spatial2 = m_settings.m_spatial2.value();
        if (m_settings.m_spatial2_count.has_value())
            m_reSTIROptions.m_spatial2_count = m_settings.m_spatial2_count.value();
        if (m_settings.m_spatial2_shift_sequential.has_value())
            m_reSTIROptions.m_spatial2_shift_sequential = m_settings.m_spatial2_shift_sequential.value();
        if (m_settings.m_cameraBSplinePosition.has_value())
            m_cameraBSplinePosition = m_settings.m_cameraBSplinePosition.value();
        if (m_settings.m_cameraBSplineOrientation.has_value())
            m_cameraBSplineOrientation = m_settings.m_cameraBSplineOrientation.value();
        if (m_settings.m_cameraBSplineU.has_value())
            m_cameraBSplineU = m_settings.m_cameraBSplineU.value();
        if (m_settings.m_cameraBSplineSpeed.has_value())
            m_cameraBSplineSpeed = m_settings.m_cameraBSplineSpeed.value();
    }

    void ReSTIRSSS::initResources(GPUContext *gpuContext) {
#ifdef WIN32
        ImGui::SetCurrentContext(reinterpret_cast<Application *>(gpuContext)->getImGuiContext());
#endif

        gpuContext->getCamera()->registerCameraUpdateCallback([this, gpuContext] {
            m_renderOptions.m_frame = 0;

            const glm::mat4 view_to_clip_space = gpuContext->getCamera()->getViewToClipSpace();
            const glm::mat4 world_to_view_space = gpuContext->getCamera()->getWorldToViewSpace();
            const glm::mat4 vp = view_to_clip_space * world_to_view_space;
            m_renderOptions.m_vp_prev = m_renderOptions.m_vp;
            m_renderOptions.m_vp = vp;
        });

        if (m_sceneName.has_value()) {
            const std::string scenePath = Paths::m_resourceDirectoryPath + "/scenes/" + m_sceneName.value() + ".xml";

            pugi::xml_document doc;
            if (const pugi::xml_parse_result result = doc.load_file(scenePath.c_str()); !result) {
                std::cerr << "XML [" << scenePath << "] parsed with errors." << std::endl;
                std::cerr << "Error description: " << result.description() << std::endl;
                std::cerr << "Error offset: " << result.offset << std::endl;
                throw std::runtime_error("Cannot parse scene file.");
            }

            if (const auto &node = doc.child("scene").child("scene"); !node.empty()) {
                if (!node.attribute("name").empty()) {
                    m_settings.m_scene = Rayscenes::sceneFromString(node.attribute("name").as_string());
                }
                if (!node.attribute("mode").empty()) {
                    m_settings.m_sceneMode = node.attribute("mode").as_int();
                }
            }

            if (const auto &node = doc.child("scene").child("camera"); !node.empty()) {
                if (!node.attribute("position").empty()) {
                    glm::vec3 cameraPosition;
                    if (!vec3FromString(node.attribute("position").value(), &cameraPosition)) {
                        throw std::runtime_error("Cannot parse coordinate: " + std::string(node.attribute("position").value()));
                    }
                    m_settings.m_position = cameraPosition;
                }
                if (!node.attribute("orientation").empty()) {
                    glm::vec3 cameraRotation;
                    if (!vec3FromString(node.attribute("orientation").value(), &cameraRotation)) {
                        throw std::runtime_error("Cannot parse coordinate: " + std::string(node.attribute("orientation").value()));
                    }
                    m_settings.m_orientation = cameraRotation;
                }
                if (!node.attribute("speed").empty()) {
                    m_settings.m_speed = node.attribute("speed").as_float();
                }
                if (const auto &nodeSpline = doc.child("scene").child("camera").child("spline"); !nodeSpline.empty()) {
                    for (const auto &point: nodeSpline.children("point")) {
                        glm::vec3 position;
                        if (!vec3FromString(point.attribute("position").value(), &position)) {
                            throw std::runtime_error("Cannot parse coordinate: " + std::string(point.attribute("position").value()));
                        }
                        glm::vec3 orientation;
                        if (!vec3FromString(point.attribute("orientation").value(), &orientation)) {
                            throw std::runtime_error("Cannot parse coordinate: " + std::string(point.attribute("orientation").value()));
                        }
                        m_cameraBSplinePosition.m_controlPoints.push_back(position);
                        m_cameraBSplineOrientation.m_controlPoints.push_back(orientation);
                    }
                }
            }

            if (const auto &node = doc.child("scene").child("window"); !node.empty()) {
                if (!node.attribute("width").empty() && !node.attribute("height").empty()) {
                    const int width = node.attribute("width").as_int();
                    const int height = node.attribute("height").as_int();
                    if (width > 0 && height > 0) {
                        gpuContext->resizeWindow(width, height);
                    }
                }
            }

            if (const auto &node = doc.child("scene").child("rendering"); !node.empty()) {
                if (!node.attribute("maxFrames").empty()) {
                    m_renderOptions.m_maxFrames = node.attribute("maxFrames").as_uint();
                }
                if (!node.attribute("renderer").empty()) {
                    m_renderOptions.m_renderer = node.attribute("renderer").as_uint();
                }
                if (!node.attribute("debug").empty()) {
                    m_renderOptions.m_debug = node.attribute("debug").as_uint();
                }
                if (!node.attribute("tonemapper").empty()) {
                    m_renderOptions.m_tonemapper = node.attribute("tonemapper").as_uint();
                }
                if (!node.attribute("normalMapping").empty()) {
                    m_renderOptions.m_normalMapping = node.attribute("normalMapping").as_bool();
                }
                if (!node.attribute("sss").empty()) {
                    m_renderOptions.m_sss = node.attribute("sss").as_bool();
                }
                if (!node.attribute("nonSSSObjects").empty()) {
                    m_renderOptions.m_nonSSObjects = node.attribute("nonSSSObjects").as_bool();
                }
                if (!node.attribute("spp").empty()) {
                    m_renderOptions.m_spp = node.attribute("spp").as_uint();
                }
                if (!node.attribute("denoiser").empty()) {
                    m_renderOptions.m_denoiser = node.attribute("denoiser").as_bool();
                }
            }

            if (const auto &node = doc.child("scene").child("restirsss"); !node.empty()) {
                if (!node.attribute("nonSSSSurfaces").empty()) {
                    m_reSTIROptions.m_restir_non_sss_surfaces = node.attribute("nonSSSSurfaces").as_bool();
                }
                if (!node.attribute("candidateSamplesLight").empty()) {
                    m_reSTIROptions.m_candidate_samples_light = node.attribute("candidateSamplesLight").as_uint();
                }
                if (!node.attribute("candidateSamplesBSDF").empty()) {
                    m_reSTIROptions.m_candidate_samples_bsdf = node.attribute("candidateSamplesBSDF").as_uint();
                }
                if (!node.attribute("candidateSSSAllIntersections").empty()) {
                    m_reSTIROptions.m_candidate_sss_all_intersections = node.attribute("candidateSSSAllIntersections").as_bool();
                }
                if (!node.attribute("shift").empty()) {
                    m_reSTIROptions.m_shift = node.attribute("shift").as_uint();
                }
                if (!node.attribute("temporal").empty()) {
                    m_reSTIROptions.m_temporal = node.attribute("temporal").as_bool();
                }
                if (!node.attribute("temporalShiftSequential").empty()) {
                    m_reSTIROptions.m_temporal_shift_sequential = node.attribute("temporalShiftSequential").as_uint();
                }
                if (!node.attribute("spatial").empty()) {
                    m_reSTIROptions.m_spatial = node.attribute("spatial").as_bool();
                }
                if (!node.attribute("spatialCount").empty()) {
                    m_reSTIROptions.m_spatial_count = node.attribute("spatialCount").as_uint();
                }
                if (!node.attribute("spatialShiftSequential").empty()) {
                    m_reSTIROptions.m_spatial_shift_sequential = node.attribute("spatialShiftSequential").as_uint();
                }
                if (!node.attribute("spatial2").empty()) {
                    m_reSTIROptions.m_spatial2 = node.attribute("spatial2").as_bool();
                }
                if (!node.attribute("spatial2Count").empty()) {
                    m_reSTIROptions.m_spatial2_count = node.attribute("spatial2Count").as_uint();
                }
                if (!node.attribute("spatial2ShiftSequential").empty()) {
                    m_reSTIROptions.m_spatial2_shift_sequential = node.attribute("spatial2ShiftSequential").as_uint();
                }
            }
        }

        switch (m_settings.m_scene) {
            case Rayscenes::AJAX:
                m_rayscenes.sceneAjax(gpuContext, m_raydata.m_scene);
                break;
            case Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT:
                m_rayscenes.sceneAjaxManyLights(gpuContext, m_raydata.m_scene, false);
                break;
            case Rayscenes::AJAX_MANY_LIGHTS_W_LARGE_LIGHT:
                m_rayscenes.sceneAjaxManyLights(gpuContext, m_raydata.m_scene, true);
                break;
            case Rayscenes::LTE_ORB:
                m_rayscenes.sceneLTEOrb(gpuContext, m_raydata.m_scene, m_settings.m_sceneMode.value_or(0));
                break;
            case Rayscenes::ASIAN_DRAGON:
                m_rayscenes.sceneAsianDragon(gpuContext, m_raydata.m_scene, m_settings.m_sceneMode.value_or(0));
                break;
            case Rayscenes::FLAT_TEST:
                m_rayscenes.sceneFlatTest(gpuContext, m_raydata.m_scene);
                break;
            default:
                throw std::runtime_error("Undefined scene.");
        }

        uint32_t numLights;
        m_raydata.initResources(gpuContext, &numLights, [&](const RavenSceneObject::GPUSceneObject &sceneObject, ObjectDescriptor &objectDescriptor) {
        });
        m_renderOptions.m_numLights = numLights;

        if (m_settings.m_position.has_value()) {
            gpuContext->getCamera()->moveCenter(m_settings.m_position.value());
        }
        if (m_settings.m_orientation.has_value()) {
            gpuContext->getCamera()->setRotation(m_settings.m_orientation.value().x, m_settings.m_orientation.value().y, m_settings.m_orientation.value().z);
        }
        if (m_settings.m_speed.has_value()) {
            gpuContext->getCamera()->setSpeedMultiplier(m_settings.m_speed.value());
        }

        if (m_preset.has_value()) {
            loadPreset(m_preset.value());
        }
    }

    void ReSTIRSSS::initShaderResources(GPUContext *gpuContext) {
        std::vector<Image *> images;
        for (const auto &image: m_raydata.m_textures) {
            images.push_back(image.get());
        }
        for (uint32_t i = images.size(); i < 8; i++) {
            images.push_back(m_raydata.m_textureEnvironment.get());
        }

        // pre processing passes
        m_preProcessingPass = std::make_shared<PassCompute>(gpuContext, Pass::PassSettings{.m_name = "preProcessingPass"});
        m_preProcessingPass->create();

        m_passGBuffer = std::make_shared<PassGBuffer>(gpuContext);
        m_passGBuffer->create();
        m_uniformPassGBuffer = m_passGBuffer->getUniform(0, 3);
        m_passGBuffer->setAccelerationStructure(0, 0, m_raydata.m_tlas->getAccelerationStructure());
        m_passGBuffer->setStorageBuffer(0, 10, m_raydata.m_objectDescriptorBuffer.get());
        m_passGBuffer->setSamplerImages(0, 20, images, vk::ImageLayout::eReadOnlyOptimal);
        m_passGBuffer->setSamplerImage(0, 21, m_raydata.m_textureEnvironment.get(), vk::ImageLayout::eReadOnlyOptimal);

        m_passSurface = std::make_shared<PassSurface>(gpuContext);
        m_passSurface->create();
        m_uniformPassSurface = m_passSurface->getUniform(0, 3);
        m_passSurface->setAccelerationStructure(0, 0, m_raydata.m_tlas->getAccelerationStructure());
        m_passSurface->setStorageBuffer(0, 10, m_raydata.m_objectDescriptorBuffer.get());
        m_passSurface->setSamplerImages(0, 20, images, vk::ImageLayout::eReadOnlyOptimal);
        m_passSurface->setSamplerImage(0, 21, m_raydata.m_textureEnvironment.get(), vk::ImageLayout::eReadOnlyOptimal);

        m_passDebug = std::make_shared<PassDebug>(gpuContext);
        m_passDebug->create();
        m_uniformPassDebug = m_passDebug->getUniform(0, 3);
        m_passDebug->setAccelerationStructure(0, 0, m_raydata.m_tlas->getAccelerationStructure());
        m_passDebug->setStorageBuffer(0, 10, m_raydata.m_objectDescriptorBuffer.get());
        m_passDebug->setSamplerImages(0, 20, images, vk::ImageLayout::eReadOnlyOptimal);
        m_passDebug->setSamplerImage(0, 21, m_raydata.m_textureEnvironment.get(), vk::ImageLayout::eReadOnlyOptimal);

        // ReSTIRSSS passes
        m_ReSTIRSSSPass = std::make_shared<PassCompute>(gpuContext, Pass::PassSettings{.m_name = "ReSTIRSSSPass", .m_semaphoreExportFlag = true});
        m_ReSTIRSSSPass->create();

        m_ReSTIRSSSPassCandidateGeneration = std::make_shared<ReSTIRSSSPassCandidateGeneration>(gpuContext);
        m_ReSTIRSSSPassCandidateGeneration->create();
        m_uniformReSTIRSSSPassCandidateGeneration = m_ReSTIRSSSPassCandidateGeneration->getUniform(0, 3);
        m_ReSTIRSSSPassCandidateGeneration->setAccelerationStructure(0, 0, m_raydata.m_tlas->getAccelerationStructure());
        m_ReSTIRSSSPassCandidateGeneration->setStorageBuffer(0, 10, m_raydata.m_objectDescriptorBuffer.get());
        m_ReSTIRSSSPassCandidateGeneration->setSamplerImages(0, 20, images, vk::ImageLayout::eReadOnlyOptimal);
        m_ReSTIRSSSPassCandidateGeneration->setSamplerImage(0, 21, m_raydata.m_textureEnvironment.get(), vk::ImageLayout::eReadOnlyOptimal);

        m_ReSTIRSSSPassTemporalReuse = std::make_shared<ReSTIRSSSPassTemporalReuse>(gpuContext);
        m_ReSTIRSSSPassTemporalReuse->create();
        m_uniformReSTIRSSSPassTemporalReuse = m_ReSTIRSSSPassTemporalReuse->getUniform(0, 3);
        m_ReSTIRSSSPassTemporalReuse->setAccelerationStructure(0, 0, m_raydata.m_tlas->getAccelerationStructure());
        m_ReSTIRSSSPassTemporalReuse->setStorageBuffer(0, 10, m_raydata.m_objectDescriptorBuffer.get());
        m_ReSTIRSSSPassTemporalReuse->setSamplerImages(0, 20, images, vk::ImageLayout::eReadOnlyOptimal);
        m_ReSTIRSSSPassTemporalReuse->setSamplerImage(0, 21, m_raydata.m_textureEnvironment.get(), vk::ImageLayout::eReadOnlyOptimal);

        m_ReSTIRSSSPassSpatialReuse = std::make_shared<ReSTIRSSSPassSpatialReuse>(gpuContext, m_reSTIROptions.m_spatial_count);
        m_ReSTIRSSSPassSpatialReuse->create();
        m_uniformReSTIRSSSPassSpatialReuse = m_ReSTIRSSSPassSpatialReuse->getUniform(0, 3);
        m_ReSTIRSSSPassSpatialReuse->setAccelerationStructure(0, 0, m_raydata.m_tlas->getAccelerationStructure());
        m_ReSTIRSSSPassSpatialReuse->setStorageBuffer(0, 10, m_raydata.m_objectDescriptorBuffer.get());
        m_ReSTIRSSSPassSpatialReuse->setSamplerImages(0, 20, images, vk::ImageLayout::eReadOnlyOptimal);
        m_ReSTIRSSSPassSpatialReuse->setSamplerImage(0, 21, m_raydata.m_textureEnvironment.get(), vk::ImageLayout::eReadOnlyOptimal);

        m_ReSTIRSSSPassSpatialReuse2 = std::make_shared<ReSTIRSSSPassSpatialReuse>(gpuContext, m_reSTIROptions.m_spatial2_count);
        m_ReSTIRSSSPassSpatialReuse2->create();
        m_uniformReSTIRSSSPassSpatialReuse2 = m_ReSTIRSSSPassSpatialReuse2->getUniform(0, 3);
        m_ReSTIRSSSPassSpatialReuse2->setAccelerationStructure(0, 0, m_raydata.m_tlas->getAccelerationStructure());
        m_ReSTIRSSSPassSpatialReuse2->setStorageBuffer(0, 10, m_raydata.m_objectDescriptorBuffer.get());
        m_ReSTIRSSSPassSpatialReuse2->setSamplerImages(0, 20, images, vk::ImageLayout::eReadOnlyOptimal);
        m_ReSTIRSSSPassSpatialReuse2->setSamplerImage(0, 21, m_raydata.m_textureEnvironment.get(), vk::ImageLayout::eReadOnlyOptimal);

        m_ReSTIRSSSPassShade = std::make_shared<ReSTIRSSSPassShade>(gpuContext);
        m_ReSTIRSSSPassShade->create();
        m_uniformReSTIRSSSPassShade = m_ReSTIRSSSPassShade->getUniform(0, 3);
        m_ReSTIRSSSPassShade->setAccelerationStructure(0, 0, m_raydata.m_tlas->getAccelerationStructure());
        m_ReSTIRSSSPassShade->setStorageBuffer(0, 10, m_raydata.m_objectDescriptorBuffer.get());
        m_ReSTIRSSSPassShade->setSamplerImages(0, 20, images, vk::ImageLayout::eReadOnlyOptimal);
        m_ReSTIRSSSPassShade->setSamplerImage(0, 21, m_raydata.m_textureEnvironment.get(), vk::ImageLayout::eReadOnlyOptimal);

        m_passPathtrace = std::make_shared<PassPathtrace>(gpuContext);
        m_passPathtrace->create();
        m_uniformPassPathtrace = m_passPathtrace->getUniform(0, 3);
        m_passPathtrace->setAccelerationStructure(0, 0, m_raydata.m_tlas->getAccelerationStructure());
        m_passPathtrace->setStorageBuffer(0, 10, m_raydata.m_objectDescriptorBuffer.get());
        m_passPathtrace->setSamplerImages(0, 20, images, vk::ImageLayout::eReadOnlyOptimal);
        m_passPathtrace->setSamplerImage(0, 21, m_raydata.m_textureEnvironment.get(), vk::ImageLayout::eReadOnlyOptimal);

        // post processing passes
        m_postProcessingPass = std::make_shared<PassCompute>(gpuContext, Pass::PassSettings{.m_name = "postProcessingPass"});
        m_postProcessingPass->create();

        m_passTonemapper = std::make_shared<PassTonemapper>(gpuContext);
        m_passTonemapper->create();

        passesSetMaterialBuffer();
        passesSetLightBuffer();
    }

    void ReSTIRSSS::initSwapchainResources(GPUContext *gpuContext, vk::Extent2D extent) {
        m_renderOptions.m_pixels = {extent.width, extent.height};

        m_passGBuffer->setGlobalInvocationSize(extent.width, extent.height, 1);
        m_passSurface->setGlobalInvocationSize(extent.width, extent.height, 1);
        m_passDebug->setGlobalInvocationSize(extent.width, extent.height, 1);

        m_ReSTIRSSSPassCandidateGeneration->setGlobalInvocationSize(extent.width, extent.height, 1);
        m_ReSTIRSSSPassTemporalReuse->setGlobalInvocationSize(extent.width, extent.height, 1);
        m_ReSTIRSSSPassSpatialReuse->setGlobalInvocationSize(extent.width, extent.height, 1);
        m_ReSTIRSSSPassSpatialReuse2->setGlobalInvocationSize(extent.width, extent.height, 1);
        m_ReSTIRSSSPassShade->setGlobalInvocationSize(extent.width, extent.height, 1);
        m_passPathtrace->setGlobalInvocationSize(extent.width, extent.height, 1);

        m_passTonemapper->setGlobalInvocationSize(extent.width, extent.height, 1);

        {
            auto settings = Image::ImageSettings{.m_width = extent.width, .m_height = extent.height, .m_format = vk::Format::eR32G32B32A32Sfloat, .m_usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst, .m_name = "frameBuffer"};
            m_imageFramebuffer = std::make_shared<Image>(gpuContext, settings);
            m_imageFramebuffer->create();
            m_imageFramebuffer->setImageLayout(vk::ImageLayout::eGeneral);
            m_passGBuffer->setStorageImage(0, 1, m_imageFramebuffer.get());
            m_passSurface->setStorageImage(0, 1, m_imageFramebuffer.get());
            m_passDebug->setStorageImage(0, 1, m_imageFramebuffer.get());
            m_ReSTIRSSSPassShade->setStorageImage(0, 1, m_imageFramebuffer.get());
            m_passPathtrace->setStorageImage(0, 1, m_imageFramebuffer.get());
            m_passTonemapper->setStorageImage(0, 0, m_imageFramebuffer.get());
        }
        {
            auto settings = Image::ImageSettings{.m_width = extent.width, .m_height = extent.height, .m_format = vk::Format::eR32G32B32A32Sfloat, .m_usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc, .m_name = "accumulationBuffer"};
            m_imageAccumulationBuffer = std::make_shared<Image>(gpuContext, settings);
            m_imageAccumulationBuffer->create();
            m_imageAccumulationBuffer->setImageLayout(vk::ImageLayout::eGeneral);
            m_passGBuffer->setStorageImage(0, 2, m_imageAccumulationBuffer.get());
            m_passSurface->setStorageImage(0, 2, m_imageAccumulationBuffer.get());
            m_passDebug->setStorageImage(0, 2, m_imageAccumulationBuffer.get());
            m_ReSTIRSSSPassShade->setStorageImage(0, 2, m_imageAccumulationBuffer.get());
            m_passPathtrace->setStorageImage(0, 2, m_imageAccumulationBuffer.get());
        }
        {
            auto settings = Image::ImageSettings{.m_width = extent.width, .m_height = extent.height, .m_format = vk::Format::eR32G32B32A32Sfloat, .m_usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc, .m_name = "albedoBuffer"};
            m_imageAlbedoBuffer = std::make_shared<Image>(gpuContext, settings);
            m_imageAlbedoBuffer->create();
            m_imageAlbedoBuffer->setImageLayout(vk::ImageLayout::eGeneral);
            m_passGBuffer->setStorageImage(0, 31, m_imageAlbedoBuffer.get());
        }
        {
            auto settings = Image::ImageSettings{.m_width = extent.width, .m_height = extent.height, .m_format = vk::Format::eR32G32B32A32Sfloat, .m_usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc, .m_name = "normalBuffer"};
            m_imageNormalBuffer = std::make_shared<Image>(gpuContext, settings);
            m_imageNormalBuffer->create();
            m_imageNormalBuffer->setImageLayout(vk::ImageLayout::eGeneral);
            m_passGBuffer->setStorageImage(0, 32, m_imageNormalBuffer.get());
        }
#ifdef RESTIRSSS_WRITE_DEBUG_IMAGES
        {
            for (uint32_t i = 0; i < 3; i++) {
                std::stringstream ss;
                ss << "ImageReSTIRSSSDebug[" << i << "]";
                auto settings = Image::ImageSettings{.m_width = extent.width, .m_height = extent.height, .m_format = vk::Format::eR32G32B32A32Sfloat, .m_usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage, .m_name = ss.str()};
                m_imageReSTIRDebug[i] = std::make_shared<Image>(gpuContext, settings);
                m_imageReSTIRDebug[i]->create();
                m_imageReSTIRDebug[i]->setImageLayout(vk::ImageLayout::eGeneral);
                m_ReSTIRSSSPassShade->setStorageImage(0, 50 + i, m_imageReSTIRDebug[i].get());
            }
            m_ReSTIRSSSPassTemporalReuse->setStorageImage(0, 50, m_imageReSTIRDebug[0].get());
            m_ReSTIRSSSPassSpatialReuse->setStorageImage(0, 50, m_imageReSTIRDebug[1].get());
            m_ReSTIRSSSPassSpatialReuse2->setStorageImage(0, 50, m_imageReSTIRDebug[2].get());
        }
#endif
        {
            for (uint32_t i = 0; i < 2; i++) {
                std::stringstream ss;
                ss << "ImageGBuffer[" << i << "]";
                auto settings = Image::ImageSettings{.m_width = extent.width, .m_height = extent.height, .m_format = vk::Format::eR32G32B32A32Sfloat, .m_usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage, .m_name = ss.str()};
                m_imageGBuffer[i] = std::make_shared<Image>(gpuContext, settings);
                m_imageGBuffer[i]->create();
                m_imageGBuffer[i]->setImageLayout(vk::ImageLayout::eGeneral);
            }
        }
        {
            for (uint32_t i = 0; i < 8; i++) {
                std::stringstream ss;
                ss << "ImageReSTIRReservoirBuffer[" << i << "]";
                auto settings = Image::ImageSettings{.m_width = extent.width, .m_height = extent.height, .m_format = vk::Format::eR32G32B32A32Sfloat, .m_usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage, .m_name = ss.str()};
                m_imageReSTIRReservoirBuffer[i] = std::make_shared<Image>(gpuContext, settings);
                m_imageReSTIRReservoirBuffer[i]->create();
                m_imageReSTIRReservoirBuffer[i]->setImageLayout(vk::ImageLayout::eGeneral);
            }
        }

        createDenoiserEngine(gpuContext, extent);

        resetFrame(gpuContext);
    }

    void ReSTIRSSS::recordImGui(GPUContext *gpuContext, Camera *camera) {
        ImGui::Begin("Settings");

        const glm::vec3 position = camera->getPosition();
        const glm::vec3 rotation = camera->getSphericalRotation();
        const glm::ivec3 voxel = {static_cast<int>(glm::floor(position.x)), static_cast<int>(glm::floor(position.y)), static_cast<int>(glm::floor(position.z))};
        ImGui::Text("Voxel (X,Y,Z) = (%i,%i,%i)", voxel.x, voxel.y, voxel.z);
        ImGui::Text("Position (X,Y,Z) = (%.3f,%.3f,%.3f)", position.x, position.y, position.z);
        ImGui::Text("Rotation (r,theta,phi) = (%.3f,%.3f,%.3f)", rotation.x, rotation.y, rotation.z);

        ImGui::Text("Frames: %u/%u", m_renderOptions.m_frame, m_renderOptions.m_maxFrames);
        ImGui::InputInt("Max Frames", reinterpret_cast<int *>(&m_renderOptions.m_maxFrames), 1);

        if (ImGui::CollapsingHeader("Presets", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button("Path Tracing")) {
                loadPreset("pathtracing");
                resetFrame(gpuContext);
            }
            if (ImGui::Button("ReSTIR SSS (Hybrid Shift)")) {
                loadPreset("hybrid");
                resetFrame(gpuContext);
            }
            if (ImGui::Button("ReSTIR SSS (Sequential Shift)")) {
                loadPreset("sequential");
                resetFrame(gpuContext);
            }
        }

        if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (const char *rendererItems[] = {"OFF", "GAMMA CORRECTION"}; ImGui::Combo("Tone-Mapping", reinterpret_cast<int32_t *>(&m_renderOptions.m_tonemapper), rendererItems, IM_ARRAYSIZE(rendererItems))) {
                resetFrame(gpuContext);
            }

            ImGui::Separator();

#ifdef OPTIX_SUPPORT
            if (ImGui::Checkbox("Denoiser OptiX", &m_renderOptions.m_denoiser)) {
                resetFrame(gpuContext);
            }

            if (m_renderOptions.m_denoiser) {
                if (ImGui::Checkbox("OptiX Guide Albedo", &m_renderOptions.m_denoiserGuideAlbedo)) {
                    gpuContext->m_device.waitIdle();
                    releaseDenoiserEngine();
                    createDenoiserEngine(gpuContext, {static_cast<uint32_t>(m_renderOptions.m_pixels.x), static_cast<uint32_t>(m_renderOptions.m_pixels.y)});
                    resetFrame(gpuContext);
                }
                if (ImGui::Checkbox("OptiX Guide Normal", &m_renderOptions.m_denoiserGuideNormal)) {
                    gpuContext->m_device.waitIdle();
                    releaseDenoiserEngine();
                    createDenoiserEngine(gpuContext, {static_cast<uint32_t>(m_renderOptions.m_pixels.x), static_cast<uint32_t>(m_renderOptions.m_pixels.y)});
                    resetFrame(gpuContext);
                }
            }

            ImGui::Separator();
#endif

            if (ImGui::Checkbox("Normal Mapping", &m_renderOptions.m_normalMapping)) {
                resetFrame(gpuContext);
            }
            if (ImGui::Checkbox("Subsurface Scattering", &m_renderOptions.m_sss)) {
                resetFrame(gpuContext);
            }
            if (ImGui::Checkbox("Render non Subsurface Scattering Objects", &m_renderOptions.m_nonSSObjects)) {
                resetFrame(gpuContext);
            }

            ImGui::Separator();

            if (const char *rendererItems[] = {"RESTIRSSS", "DEBUG", "PATHTRACE"}; ImGui::Combo("Renderer", reinterpret_cast<int32_t *>(&m_renderOptions.m_renderer), rendererItems, IM_ARRAYSIZE(rendererItems))) {
                resetFrame(gpuContext);
            }
            if (m_renderOptions.m_renderer == RENDERER_RESTIRSSS) {
                if (ImGui::Checkbox("Candidate Generation SSS All Intersections", &m_reSTIROptions.m_candidate_sss_all_intersections)) {
                    resetFrame(gpuContext);
                }
                if (ImGui::InputInt("Candidate Generation Light Source Sampling (NEE)", reinterpret_cast<int *>(&m_reSTIROptions.m_candidate_samples_light), 1)) {
                    m_reSTIROptions.m_candidate_samples_light = glm::clamp(m_reSTIROptions.m_candidate_samples_light, 0u, 128u);
                    resetFrame(gpuContext);
                }
                if (ImGui::InputInt("Candidate Generation BSDF Sampling", reinterpret_cast<int *>(&m_reSTIROptions.m_candidate_samples_bsdf), 1)) {
                    m_reSTIROptions.m_candidate_samples_bsdf = glm::clamp(m_reSTIROptions.m_candidate_samples_bsdf, 0u, 128u);
                    resetFrame(gpuContext);
                }
                ImGui::Separator();
                if (const char *shiftMappingItems[] = {"RECONNECTION", "DELAYED_RECONNECTION", "HYBRID", "SEQUENTIAL"}; ImGui::Combo("Shift Mapping", reinterpret_cast<int32_t *>(&m_reSTIROptions.m_shift), shiftMappingItems, IM_ARRAYSIZE(shiftMappingItems))) {
                    resetFrame(gpuContext);
                }
                ImGui::Separator();
                if (ImGui::Checkbox("Temporal Reuse", &m_reSTIROptions.m_temporal)) {
                    resetFrame(gpuContext);
                }
                if (m_reSTIROptions.m_shift == RESTIRSSS_SHIFT_SEQUENTIAL) {
                    if (const char *temporalShiftMappingItems[] = {"RECONNECTION", "DELAYED_RECONNECTION"}; ImGui::Combo("Temporal Reuse Shift (SEQUENTIAL)", reinterpret_cast<int32_t *>(&m_reSTIROptions.m_temporal_shift_sequential), temporalShiftMappingItems, IM_ARRAYSIZE(temporalShiftMappingItems))) {
                        resetFrame(gpuContext);
                    }
                }
                ImGui::Separator();
                if (ImGui::Checkbox("Spatial Reuse", &m_reSTIROptions.m_spatial)) {
                    resetFrame(gpuContext);
                }
                if (ImGui::InputInt("Spatial Reuse Neighbor Count", reinterpret_cast<int *>(&m_reSTIROptions.m_spatial_count), 1)) {
                    m_reSTIROptions.m_spatial_count = glm::clamp(m_reSTIROptions.m_spatial_count, 0u, 64u);
                    gpuContext->m_device.waitIdle();
                    m_ReSTIRSSSPassSpatialReuse->updateSpecializationConstant3(m_reSTIROptions.m_spatial_count);
                    resetFrame(gpuContext);
                }
                if (m_reSTIROptions.m_shift == RESTIRSSS_SHIFT_SEQUENTIAL) {
                    if (const char *spatialShiftMappingItems[] = {"RECONNECTION", "DELAYED_RECONNECTION"}; ImGui::Combo("Spatial Reuse Shift (SEQUENTIAL)", reinterpret_cast<int32_t *>(&m_reSTIROptions.m_spatial_shift_sequential), spatialShiftMappingItems, IM_ARRAYSIZE(spatialShiftMappingItems))) {
                        resetFrame(gpuContext);
                    }
                }
                ImGui::Separator();
                if (ImGui::Checkbox("Spatial2 Reuse", &m_reSTIROptions.m_spatial2)) {
                    resetFrame(gpuContext);
                }
                if (ImGui::InputInt("Spatial2 Reuse Neighbor Count", reinterpret_cast<int *>(&m_reSTIROptions.m_spatial2_count), 1)) {
                    m_reSTIROptions.m_spatial2_count = glm::clamp(m_reSTIROptions.m_spatial2_count, 0u, 64u);
                    gpuContext->m_device.waitIdle();
                    m_ReSTIRSSSPassSpatialReuse2->updateSpecializationConstant3(m_reSTIROptions.m_spatial2_count);
                    resetFrame(gpuContext);
                }
                if (m_reSTIROptions.m_shift == RESTIRSSS_SHIFT_SEQUENTIAL) {
                    if (const char *spatial2ShiftMappingItems[] = {"RECONNECTION", "DELAYED_RECONNECTION"}; ImGui::Combo("Spatial2 Reuse Shift (SEQUENTIAL)", reinterpret_cast<int32_t *>(&m_reSTIROptions.m_spatial2_shift_sequential), spatial2ShiftMappingItems, IM_ARRAYSIZE(spatial2ShiftMappingItems))) {
                        resetFrame(gpuContext);
                    }
                }
                ImGui::Separator();
                if (ImGui::Checkbox("ReSTIR for non-SSS Surfaces", &m_reSTIROptions.m_restir_non_sss_surfaces)) {
                    resetFrame(gpuContext);
                }
#ifdef RESTIRSSS_WRITE_DEBUG_IMAGES
                ImGui::Separator();
                if (const char *items[] = {"DISABLED", "TEMPORAL_REUSE", "SPATIAL_REUSE", "SPATIAL2_REUSE"}; ImGui::Combo("ReSTIR Debug", reinterpret_cast<int32_t *>(&m_reSTIROptions.m_restir_debug), items, IM_ARRAYSIZE(items))) {
                    resetFrame(gpuContext);
                }
#endif
            }
            if (m_renderOptions.m_renderer == RENDERER_DEBUG) {
                if (const char *debugItems[] = {"POSITION", "NORMAL", "GEOMETRIC_NORMAL", "UV", "BASE_COLOR", "NORMAL_MAP", "TANGENT", "BI_TANGENT", "OBJECT_DESCRIPTOR", "TRIANGLE", "MOTION", "FACE", "COLORMAPS", "DISK_BASED_SAMPLING_INTERSECTIONS", "RESTIRSSS_SEARCH_RADIUS"}; ImGui::Combo("Debug Mode", reinterpret_cast<int32_t *>(&m_renderOptions.m_debug), debugItems, IM_ARRAYSIZE(debugItems))) {
                    resetFrame(gpuContext);
                }
            }
            if (m_renderOptions.m_renderer == RENDERER_PATHTRACE) {
                if (ImGui::InputInt("Samples per Pixel per Frame", reinterpret_cast<int *>(&m_renderOptions.m_spp), 1)) {
                    m_renderOptions.m_spp = glm::clamp(m_renderOptions.m_spp, 1u, 128u);
                    resetFrame(gpuContext);
                }
            }
        }
        if (ImGui::CollapsingHeader("Environment")) {
            if (ImGui::Checkbox("Environment Map", &m_rayscenes.m_sceneOptions.m_environmentMap)) {
                resetFrame(gpuContext);
            }
            if (ImGui::SliderFloat2("Environment Map Rotation", reinterpret_cast<float *>(&m_rayscenes.m_sceneOptions.m_environmentMapRotation), 0.f, 1.f)) {
                resetFrame(gpuContext);
            }
            if (ImGui::SliderFloat3("Sky Color", reinterpret_cast<float *>(&m_rayscenes.m_sceneOptions.m_skyColor), 0.f, 1.f)) {
                resetFrame(gpuContext);
            }
        }
        bool updateMaterial = false;
        bool updateLight = false;
        uint32_t numLights = 0;
        m_raydata.guiScene(gpuContext, &updateMaterial, &updateLight, &numLights);
        if (updateMaterial) {
            passesSetMaterialBuffer();
            resetFrame(gpuContext);
        }
        if (updateLight) {
            passesSetLightBuffer();
            m_renderOptions.m_numLights = numLights;
            resetFrame(gpuContext);
        }
        if (ImGui::CollapsingHeader("Camera")) {
            if (ImGui::Button("B-Spline")) {
                m_cameraBSplineU = 0.f;
            }
            ImGui::InputFloat("B-Spline Speed", &m_cameraBSplineSpeed);
            ImGui::Text("%f", m_cameraBSplineU);
            if (ImGui::Button("Control Point +")) {
                m_cameraBSplinePosition.m_controlPoints.push_back(camera->getPosition());
                glm::vec3 orientation = camera->getSphericalRotation();
                if (!m_cameraBSplineOrientation.m_controlPoints.empty()) {
                    glm::vec3 lastOrientation = m_cameraBSplineOrientation.m_controlPoints.back();
                    float floored = glm::floor(lastOrientation.y / glm::two_pi<float>()) * glm::two_pi<float>();
                    orientation.y += floored;
                    if (orientation.y < lastOrientation.y) {
                        float ceiled = glm::ceil(lastOrientation.y / glm::two_pi<float>()) * glm::two_pi<float>();
                        orientation.y += ceiled;
                        std::cout << ceiled << " " << orientation.y << std::endl;
                    }
                }
                m_cameraBSplineOrientation.m_controlPoints.push_back(orientation);
            }
            static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;
            if (ImGui::BeginTable("table1", 7, flags)) {
                ImGui::TableSetupColumn("Index");
                ImGui::TableSetupColumn("Position");
                ImGui::TableSetupColumn("Orientation");
                ImGui::TableSetupColumn("Teleport");
                ImGui::TableSetupColumn("Up");
                ImGui::TableSetupColumn("Down");
                ImGui::TableSetupColumn("Delete");
                ImGui::TableHeadersRow();
                for (int row = 0; row < m_cameraBSplinePosition.m_controlPoints.size(); row++) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%d", row);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%f,%f,%f", m_cameraBSplinePosition.m_controlPoints[row].x, m_cameraBSplinePosition.m_controlPoints[row].y, m_cameraBSplinePosition.m_controlPoints[row].z);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%f,%f,%f", m_cameraBSplineOrientation.m_controlPoints[row].x, m_cameraBSplineOrientation.m_controlPoints[row].y, m_cameraBSplineOrientation.m_controlPoints[row].z);
                    ImGui::TableSetColumnIndex(3);
                    std::string abc = "controlpoint" + std::to_string(row);
                    ImGui::PushID(abc.c_str());
                    if (ImGui::SmallButton("##teleport")) {
                        gpuContext->getCamera()->fromSphericalCoordinates(m_cameraBSplinePosition.m_controlPoints[row], m_cameraBSplineOrientation.m_controlPoints[row].x, m_cameraBSplineOrientation.m_controlPoints[row].y, m_cameraBSplineOrientation.m_controlPoints[row].z);
                        resetFrame(gpuContext);
                    }
                    ImGui::TableSetColumnIndex(4);
                    if (ImGui::SmallButton("##up")) {
                        if (row > 0) {
                            std::swap(m_cameraBSplinePosition.m_controlPoints[row], m_cameraBSplinePosition.m_controlPoints[row - 1]);
                            std::swap(m_cameraBSplineOrientation.m_controlPoints[row], m_cameraBSplineOrientation.m_controlPoints[row - 1]);
                        }
                    }
                    ImGui::TableSetColumnIndex(5);
                    if (ImGui::SmallButton("##down")) {
                        if (row < m_cameraBSplinePosition.m_controlPoints.size() - 1) {
                            std::swap(m_cameraBSplinePosition.m_controlPoints[row], m_cameraBSplinePosition.m_controlPoints[row + 1]);
                            std::swap(m_cameraBSplineOrientation.m_controlPoints[row], m_cameraBSplineOrientation.m_controlPoints[row + 1]);
                        }
                    }
                    ImGui::TableSetColumnIndex(6);
                    if (ImGui::SmallButton("##delete")) {
                        m_cameraBSplinePosition.m_controlPoints.erase(m_cameraBSplinePosition.m_controlPoints.begin() + row);
                        m_cameraBSplineOrientation.m_controlPoints.erase(m_cameraBSplineOrientation.m_controlPoints.begin() + row);
                    }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            if (ImGui::Button("Print Control Points")) {
                for (const auto &point: m_cameraBSplinePosition.m_controlPoints) {
                    std::cout << "{" << point.x << "," << point.y << "," << point.z << "},";
                }
                std::cout << std::endl;
                for (const auto &point: m_cameraBSplineOrientation.m_controlPoints) {
                    std::cout << "{" << point.x << "," << point.y << "," << point.z << "},";
                }
                std::cout << std::endl;
            }

            ImGui::Separator();

            if (ImGui::Button("Statue far")) {
                gpuContext->getCamera()->fromSphericalCoordinates({AJAX_POSITION}, AJAX_ORIENTATION);
                resetFrame(gpuContext);
            }
            if (ImGui::Button("Statue near")) {
                gpuContext->getCamera()->fromSphericalCoordinates({15.914f, 10.581f, -0.313f}, 0.f, 1.816f, 1.658f);
                resetFrame(gpuContext);
            }
            if (ImGui::Button("Statue back")) {
                gpuContext->getCamera()->fromSphericalCoordinates({7.498f, -7.052f, 17.030f}, 0.f, 0.422f, 1.655f);
                resetFrame(gpuContext);
            }
            if (ImGui::Button("Statue left")) {
                gpuContext->getCamera()->fromSphericalCoordinates({15.739f, -13.075f, -2.773f}, 0.f, 1.886f, 1.8f);
                resetFrame(gpuContext);
            }
            if (ImGui::Button("Statue Shadow Edge")) {
                gpuContext->getCamera()->fromSphericalCoordinates({12.448f, -11.063f, -2.089f}, 0.f, 1.826f, 2.174f);
                resetFrame(gpuContext);
            }
            if (ImGui::Button("Execute Trajectory AJAX")) {
                glm::vec3 start = {AJAX_TRAJECTORY_START};
                glm::vec3 end = {AJAX_POSITION};
                gpuContext->getCamera()->fromSphericalCoordinates(start, AJAX_ORIENTATION);
                resetFrame(gpuContext);
                gpuContext->getCamera()->initTrajectory(start, end, 16);
            }
            if (ImGui::Button("Execute Trajectory ASIAN_DRAGON")) {
                glm::vec3 start = {ASIAN_DRAGON_TRAJECTORY_START};
                glm::vec3 end = {ASIAN_DRAGON_POSITION};
                gpuContext->getCamera()->fromSphericalCoordinates(start, ASIAN_DRAGON_ORIENTATION);
                resetFrame(gpuContext);
                gpuContext->getCamera()->initTrajectory(start, end, 16);
            }
            if (ImGui::Button("Execute Trajectory LTE_ORB")) {
                glm::vec3 start = {LTE_ORB_TRAJECTORY_START};
                glm::vec3 end = {LTE_ORB_POSITION};
                gpuContext->getCamera()->fromSphericalCoordinates(start, LTE_ORB_ORIENTATION);
                resetFrame(gpuContext);
                gpuContext->getCamera()->initTrajectory(start, end, 16);
            }
            if (ImGui::Button("Execute Trajectory SHADOW EDGE")) {
                glm::vec3 start = {12.654f, -11.063f, -1.579f};
                glm::vec3 end = {12.448f, -11.063f, -2.089f};
                gpuContext->getCamera()->fromSphericalCoordinates(start, 0.f, 1.826f, 2.174f);
                resetFrame(gpuContext);
                gpuContext->getCamera()->initTrajectory(start, end, 16);
            }
        }
        if (ImGui::CollapsingHeader("Utility")) {
            if (ImGui::Button("Screenshot")) {
                const auto t = std::time(nullptr);
                const auto tm = *std::localtime(&t);
                std::ostringstream oss;
                oss << std::put_time(&tm, "%Y-%m-%d-%H-%M-%S");
                const std::string path = "screenshot-" + oss.str() + ".png";
                screenshot(gpuContext, m_imageFramebuffer, path);
            }
        }
        if (ImGui::CollapsingHeader("Timings", ImGuiTreeNodeFlags_DefaultOpen)) {
            {
                if (m_renderOptions.m_renderer == RENDERER_DEBUG) {
                    ImGui::Text("PassDebug                        %.3f", m_passDebug->getTimeAveraged());
                } else {
                    ImGui::Text("PassGBuffer                      %.3f", m_passGBuffer->getTimeAveraged());
                    if (!m_reSTIROptions.m_restir_non_sss_surfaces) {
                        ImGui::Text("PassSurface                      %.3f", m_passSurface->getTimeAveraged());
                    }
                }
                ImGui::Separator();
            }
            if (m_renderOptions.m_renderer == RENDERER_RESTIRSSS) {
                ImGui::Text("ReSTIRSSSPassCandidateGeneration %.3f", m_ReSTIRSSSPassCandidateGeneration->getTimeAveraged());
                ImGui::Text("ReSTIRSSSPassTemporalReuse       %.3f", m_ReSTIRSSSPassTemporalReuse->getTimeAveraged());
                ImGui::Text("ReSTIRSSSPassSpatialReuse        %.3f", m_ReSTIRSSSPassSpatialReuse->getTimeAveraged());
                ImGui::Text("ReSTIRSSSPassSpatialReuse2       %.3f", m_ReSTIRSSSPassSpatialReuse2->getTimeAveraged());
                ImGui::Text("ReSTIRSSSPassShade               %.3f", m_ReSTIRSSSPassShade->getTimeAveraged());
                const auto candidateGeneration = std::isnan(m_ReSTIRSSSPassCandidateGeneration->getTimeAveraged()) ? 0 : m_ReSTIRSSSPassCandidateGeneration->getTimeAveraged();
                const auto temporalReuse = std::isnan(m_ReSTIRSSSPassTemporalReuse->getTimeAveraged()) ? 0 : m_ReSTIRSSSPassTemporalReuse->getTimeAveraged();
                const auto spatialReuse = std::isnan(m_ReSTIRSSSPassSpatialReuse->getTimeAveraged()) ? 0 : m_ReSTIRSSSPassSpatialReuse->getTimeAveraged();
                const auto spatialReuse2 = std::isnan(m_ReSTIRSSSPassSpatialReuse2->getTimeAveraged()) ? 0 : m_ReSTIRSSSPassSpatialReuse2->getTimeAveraged();
                const auto shade = std::isnan(m_ReSTIRSSSPassShade->getTimeAveraged()) ? 0 : m_ReSTIRSSSPassShade->getTimeAveraged();
                const auto total = candidateGeneration + temporalReuse + spatialReuse + spatialReuse2 + shade;
                ImGui::Text("=                                %.3f", total);
            } else if (m_renderOptions.m_renderer == RENDERER_PATHTRACE) {
                ImGui::Text("ReSTIRSSSPassPathtrace           %.3f", m_passPathtrace->getTimeAveraged());
            }
            {
                ImGui::Separator();
                ImGui::Text("PassTonemapper                   %.3f", m_passTonemapper->getTimeAveraged());
            }
        }

        ImGui::End();
    }

    void ReSTIRSSS::update(GPUContext *gpuContext, Camera *camera, const float deltaTime) {
        if (m_cameraBSplineU < FLT_MAX && m_cameraBSplineU + deltaTime * m_cameraBSplineSpeed < m_cameraBSplinePosition.getMaxU()) {
            m_cameraBSplineU += deltaTime * m_cameraBSplineSpeed;
            camera->moveCenter(m_cameraBSplinePosition.evaluate(m_cameraBSplineU));
            const auto orientation = m_cameraBSplineOrientation.evaluate(m_cameraBSplineU);
            camera->setRotation(0.f, orientation.y, orientation.z);
            // resetFrame(gpuContext);
            camera->onCameraUpdate();
        } else {
            m_cameraBSplineU = FLT_MAX;
        }
    }

    void ReSTIRSSS::render(GPUContext *gpuContext, Camera *camera, const uint32_t activeIndex, RendererResult *rendererResult) {
        if (m_renderOptions.m_frame >= m_renderOptions.m_maxFrames) {
            rendererResult->m_image = m_imageFramebuffer.get();
            return;
        }

        const glm::mat4 view_to_clip_space = camera->getViewToClipSpace();
        const glm::mat4 clip_to_view_space = inverse(view_to_clip_space);
        const glm::mat4 world_to_view_space = camera->getWorldToViewSpace();
        const glm::mat4 view_to_world_space = inverse(world_to_view_space);

        const glm::vec3 rayOrigin = camera->getPosition();
        const glm::vec3 rayLeftBottom = Camera::ndcToWorldSpace(-1.f, -1.f, clip_to_view_space, view_to_world_space);
        const glm::vec3 rayLeftTop = Camera::ndcToWorldSpace(-1.f, 1.f, clip_to_view_space, view_to_world_space);
        const glm::vec3 rayRightBottom = Camera::ndcToWorldSpace(1.f, -1.f, clip_to_view_space, view_to_world_space);
        const glm::vec3 rayRightTop = Camera::ndcToWorldSpace(1.f, 1.f, clip_to_view_space, view_to_world_space);

        const bool executeDebug = m_renderOptions.m_renderer == RENDERER_DEBUG;
        const bool executeReSTIRSSSPass = !executeDebug && (m_renderOptions.m_renderer == RENDERER_RESTIRSSS || m_renderOptions.m_renderer == RENDERER_PATHTRACE);
#ifdef OPTIX_SUPPORT
        const bool executeDenoiser = executeReSTIRSSSPass && m_renderOptions.m_denoiser;
#endif

        // pre processing passes
        // 0 <= rng offset < 10
        {
            if (executeDebug) {
                m_preProcessingPass->execute(
                        [&] {
                        },
                        [&] {
                            m_uniformPassDebug->setVariable<glm::mat4>("g_view_to_clip_space", view_to_clip_space);
                            m_uniformPassDebug->setVariable<glm::mat4>("g_world_to_view_space", world_to_view_space);
                            m_uniformPassDebug->setVariable<uint32_t>("g_debug_mode", m_renderOptions.m_debug);
                            m_uniformPassDebug->setVariable<uint32_t>("g_num_lights", m_renderOptions.m_numLights);
                            m_uniformPassDebug->setVariable<uint32_t>("g_environment_map", m_rayscenes.m_sceneOptions.m_environmentMap);
                            m_uniformPassDebug->setVariable<glm::vec2>("g_environment_map_rotation", m_rayscenes.m_sceneOptions.m_environmentMapRotation);
                            m_uniformPassDebug->setVariable<glm::vec3>("g_sky_color", m_rayscenes.m_sceneOptions.m_skyColor);
                            m_uniformPassDebug->setVariable<uint32_t>("g_normal_mapping", m_renderOptions.m_normalMapping ? 1 : 0);
                            m_uniformPassDebug->setVariable<uint32_t>("g_sss", m_renderOptions.m_sss ? 1 : 0);
                            m_uniformPassDebug->setVariable<glm::mat4>("g_vp", m_renderOptions.m_vp);
                            m_uniformPassDebug->setVariable<glm::mat4>("g_vp_prev", m_renderOptions.m_vp_prev);
                            m_uniformPassDebug->upload(activeIndex);
                        },
                        [&](vk::CommandBuffer &commandBuffer) {
                            // execute shaders
                            {
                                m_passDebug->m_pushConstants.g_pixels = m_renderOptions.m_pixels;
                                m_passDebug->m_pushConstants.g_ray_origin = rayOrigin;
                                m_passDebug->m_pushConstants.g_ray_left_bottom = rayLeftBottom;
                                m_passDebug->m_pushConstants.g_ray_left_top = rayLeftTop;
                                m_passDebug->m_pushConstants.g_ray_right_bottom = rayRightBottom;
                                m_passDebug->m_pushConstants.g_ray_right_top = rayRightTop;
                                m_passDebug->m_pushConstants.g_frame = m_renderOptions.m_frame;
                                m_passDebug->m_pushConstants.g_rng_init = m_renderOptions.m_time;
                                m_passDebug->m_pushConstants.g_rng_init_offset = 0;

                                m_passDebug->execute(
                                        [&](const vk::CommandBuffer &cbf, const vk::PipelineLayout &pipelineLayout) {
                                            cbf.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(PassDebug::PushConstants), &m_passDebug->m_pushConstants);
                                        },
                                        [](const vk::CommandBuffer &) {
                                        },
                                        commandBuffer);
                            }
                        },
                        0, nullptr, nullptr, nullptr);
            } else {
                m_preProcessingPass->execute(
                        [&] {
                            // m_passGBuffer->setStorageBuffer(0, 30, m_gBuffer[m_renderOptions.getGBufferThisFrameIndex()].get());
                            m_passGBuffer->setStorageImage(0, 30, m_imageGBuffer[m_renderOptions.getGBufferThisFrameIndex()].get());

                            if (!m_reSTIROptions.m_restir_non_sss_surfaces) {
                                // m_passSurface->setStorageBuffer(0, 30, m_gBuffer[m_renderOptions.getGBufferThisFrameIndex()].get());
                                m_passSurface->setStorageImage(0, 30, m_imageGBuffer[m_renderOptions.getGBufferThisFrameIndex()].get());
                            }
                        },
                        [&] {
                            m_uniformPassGBuffer->setVariable<uint32_t>("g_num_lights", m_renderOptions.m_numLights);
                            m_uniformPassGBuffer->setVariable<uint32_t>("g_environment_map", m_rayscenes.m_sceneOptions.m_environmentMap);
                            m_uniformPassGBuffer->setVariable<glm::vec2>("g_environment_map_rotation", m_rayscenes.m_sceneOptions.m_environmentMapRotation);
                            m_uniformPassGBuffer->setVariable<glm::vec3>("g_sky_color", m_rayscenes.m_sceneOptions.m_skyColor);
                            m_uniformPassGBuffer->setVariable<uint32_t>("g_normal_mapping", m_renderOptions.m_normalMapping ? 1 : 0);
                            m_uniformPassGBuffer->setVariable<uint32_t>("g_sss", m_renderOptions.m_sss ? 1 : 0);
                            m_uniformPassGBuffer->setVariable<uint32_t>("g_non_sss_objects", m_renderOptions.m_nonSSObjects ? 1 : 0);
                            m_uniformPassGBuffer->setVariable<glm::mat4>("g_vp", m_renderOptions.m_vp);
                            m_uniformPassGBuffer->setVariable<glm::mat4>("g_vp_prev", m_renderOptions.m_vp_prev);
                            m_uniformPassGBuffer->upload(activeIndex);

                            if (!m_reSTIROptions.m_restir_non_sss_surfaces) {
                                m_uniformPassSurface->setVariable<uint32_t>("g_num_lights", m_renderOptions.m_numLights);
                                m_uniformPassSurface->setVariable<uint32_t>("g_normal_mapping", m_renderOptions.m_normalMapping ? 1 : 0);
                                m_uniformPassSurface->setVariable<uint32_t>("g_sss", m_renderOptions.m_sss ? 1 : 0);
                                m_uniformPassSurface->upload(activeIndex);
                            }
                        },
                        [&](vk::CommandBuffer &commandBuffer) {
                            // execute shaders
                            {
                                m_passGBuffer->m_pushConstants.g_pixels = m_renderOptions.m_pixels;
                                m_passGBuffer->m_pushConstants.g_ray_origin = rayOrigin;
                                m_passGBuffer->m_pushConstants.g_ray_left_bottom = rayLeftBottom;
                                m_passGBuffer->m_pushConstants.g_ray_left_top = rayLeftTop;
                                m_passGBuffer->m_pushConstants.g_ray_right_bottom = rayRightBottom;
                                m_passGBuffer->m_pushConstants.g_ray_right_top = rayRightTop;
                                m_passGBuffer->m_pushConstants.g_frame = m_renderOptions.m_frame;
                                m_passGBuffer->m_pushConstants.g_rng_init = m_renderOptions.m_time;
                                m_passGBuffer->m_pushConstants.g_rng_init_offset = 0;

                                m_passGBuffer->execute(
                                        [&](const vk::CommandBuffer &cbf, const vk::PipelineLayout &pipelineLayout) {
                                            cbf.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(PassGBuffer::PushConstants), &m_passGBuffer->m_pushConstants);
                                        },
                                        [](const vk::CommandBuffer &cbf) {
                                            constexpr vk::MemoryBarrier memoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
                                            cbf.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags{}, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                                        },
                                        commandBuffer);
                            }

                            if (!m_reSTIROptions.m_restir_non_sss_surfaces) {
                                m_passSurface->m_pushConstants.g_pixels = m_renderOptions.m_pixels;
                                m_passSurface->m_pushConstants.g_ray_origin = rayOrigin;
                                m_passSurface->m_pushConstants.g_ray_left_bottom = rayLeftBottom;
                                m_passSurface->m_pushConstants.g_ray_left_top = rayLeftTop;
                                m_passSurface->m_pushConstants.g_ray_right_bottom = rayRightBottom;
                                m_passSurface->m_pushConstants.g_ray_right_top = rayRightTop;
                                m_passSurface->m_pushConstants.g_frame = m_renderOptions.m_frame;
                                m_passSurface->m_pushConstants.g_rng_init = m_renderOptions.m_time;
                                m_passSurface->m_pushConstants.g_rng_init_offset = 1;

                                m_passSurface->execute(
                                        [&](const vk::CommandBuffer &cbf, const vk::PipelineLayout &pipelineLayout) {
                                            cbf.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(PassGBuffer::PushConstants), &m_passSurface->m_pushConstants);
                                        },
                                        [](const vk::CommandBuffer &cbf) {
                                            constexpr vk::MemoryBarrier memoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
                                            cbf.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags{}, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                                        },
                                        commandBuffer);
                            }
                        },
                        0, nullptr, nullptr, nullptr);
            }
        }

        // ReSTIR SSS passes
        // 10 <= rng offset < 20
        if (executeReSTIRSSSPass) {
            const vk::Semaphore waitBeforeReSTIRSSSimelineSemaphore = m_preProcessingPass->getTimelineSemaphore();
            const uint64_t waitBeforeReSTIRSSSTimelineSemaphoreValue = m_preProcessingPass->getTimelineSemaphoreValue();
            vk::PipelineStageFlags waitBeforeReSTIRSSSDstStageMask = vk::PipelineStageFlagBits::eComputeShader;

            if (m_renderOptions.m_renderer == RENDERER_RESTIRSSS) {
                const uint32_t temporalShift = m_reSTIROptions.m_shift == RESTIRSSS_SHIFT_SEQUENTIAL ? RESTIRSSS_SHIFT_SEQUENTIAL + m_reSTIROptions.m_temporal_shift_sequential : m_reSTIROptions.m_shift;
                const uint32_t spatialShift = m_reSTIROptions.m_shift == RESTIRSSS_SHIFT_SEQUENTIAL ? RESTIRSSS_SHIFT_SEQUENTIAL + m_reSTIROptions.m_spatial_shift_sequential : m_reSTIROptions.m_shift;
                const uint32_t spatial2Shift = m_reSTIROptions.m_shift == RESTIRSSS_SHIFT_SEQUENTIAL ? RESTIRSSS_SHIFT_SEQUENTIAL + m_reSTIROptions.m_spatial2_shift_sequential : m_reSTIROptions.m_shift;

                m_ReSTIRSSSPass->execute(
                        [&] {
                            // m_ReSTIRSSSPassCandidateGeneration->setStorageBuffer(0, 30, m_gBuffer[m_renderOptions.getGBufferThisFrameIndex()].get());
                            m_ReSTIRSSSPassCandidateGeneration->setStorageImage(0, 30, m_imageGBuffer[m_renderOptions.getGBufferThisFrameIndex()].get());
                            // m_ReSTIRSSSPassCandidateGeneration->setStorageBuffer(0, 41, m_ReSTIRReservoirBuffer[m_reSTIROptions.getReservoirWriteIndex()].get()); // write
                            m_ReSTIRSSSPassCandidateGeneration->setStorageImage(0, 45 + 0, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirWriteIndex() + 0].get()); // write
                            m_ReSTIRSSSPassCandidateGeneration->setStorageImage(0, 45 + 1, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirWriteIndex() + 1].get()); // write
                            m_ReSTIRSSSPassCandidateGeneration->setStorageImage(0, 45 + 2, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirWriteIndex() + 2].get()); // write
                            m_ReSTIRSSSPassCandidateGeneration->setStorageImage(0, 45 + 3, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirWriteIndex() + 3].get()); // write
                            m_reSTIROptions.swapReservoirIndex();

                            if (m_reSTIROptions.m_temporal && m_renderOptions.m_framePreviousValid) {
                                // m_ReSTIRSSSPassTemporalReuse->setStorageBuffer(0, 30, m_gBuffer[m_renderOptions.getGBufferThisFrameIndex()].get());
                                m_ReSTIRSSSPassTemporalReuse->setStorageImage(0, 30, m_imageGBuffer[m_renderOptions.getGBufferThisFrameIndex()].get());
                                // m_ReSTIRSSSPassTemporalReuse->setStorageBuffer(0, 31, m_gBuffer[m_renderOptions.getGBufferLastFrameIndex()].get());
                                m_ReSTIRSSSPassTemporalReuse->setStorageImage(0, 31, m_imageGBuffer[m_renderOptions.getGBufferLastFrameIndex()].get());
                                // m_ReSTIRSSSPassTemporalReuse->setStorageBuffer(0, 40, m_ReSTIRReservoirBuffer[m_reSTIROptions.getReservoirWriteIndex()].get()); // last frame
                                m_ReSTIRSSSPassTemporalReuse->setStorageImage(0, 40 + 0, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirWriteIndex() + 0].get()); // last frame
                                m_ReSTIRSSSPassTemporalReuse->setStorageImage(0, 40 + 1, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirWriteIndex() + 1].get()); // last frame
                                m_ReSTIRSSSPassTemporalReuse->setStorageImage(0, 40 + 2, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirWriteIndex() + 2].get()); // last frame
                                m_ReSTIRSSSPassTemporalReuse->setStorageImage(0, 40 + 3, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirWriteIndex() + 3].get()); // last frame
                                // m_ReSTIRSSSPassTemporalReuse->setStorageBuffer(0, 41, m_ReSTIRReservoirBuffer[m_reSTIROptions.getReservoirReadIndex()].get()); // this frame
                                m_ReSTIRSSSPassTemporalReuse->setStorageImage(0, 45 + 0, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirReadIndex() + 0].get()); // this frame
                                m_ReSTIRSSSPassTemporalReuse->setStorageImage(0, 45 + 1, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirReadIndex() + 1].get()); // this frame
                                m_ReSTIRSSSPassTemporalReuse->setStorageImage(0, 45 + 2, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirReadIndex() + 2].get()); // this frame
                                m_ReSTIRSSSPassTemporalReuse->setStorageImage(0, 45 + 3, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirReadIndex() + 3].get()); // this frame
                            }

                            if (m_reSTIROptions.m_spatial) {
                                // m_ReSTIRSSSPassSpatialReuse->setStorageBuffer(0, 30, m_gBuffer[m_renderOptions.getGBufferThisFrameIndex()].get());
                                m_ReSTIRSSSPassSpatialReuse->setStorageImage(0, 30, m_imageGBuffer[m_renderOptions.getGBufferThisFrameIndex()].get());
                                // m_ReSTIRSSSPassSpatialReuse->setStorageBuffer(0, 40, m_ReSTIRReservoirBuffer[m_reSTIROptions.getReservoirReadIndex()].get()); // read
                                m_ReSTIRSSSPassSpatialReuse->setStorageImage(0, 40 + 0, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirReadIndex() + 0].get()); // read
                                m_ReSTIRSSSPassSpatialReuse->setStorageImage(0, 40 + 1, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirReadIndex() + 1].get()); // read
                                m_ReSTIRSSSPassSpatialReuse->setStorageImage(0, 40 + 2, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirReadIndex() + 2].get()); // read
                                m_ReSTIRSSSPassSpatialReuse->setStorageImage(0, 40 + 3, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirReadIndex() + 3].get()); // read
                                // m_ReSTIRSSSPassSpatialReuse->setStorageBuffer(0, 41, m_ReSTIRReservoirBuffer[m_reSTIROptions.getReservoirWriteIndex()].get()); // write
                                m_ReSTIRSSSPassSpatialReuse->setStorageImage(0, 45 + 0, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirWriteIndex() + 0].get()); // write
                                m_ReSTIRSSSPassSpatialReuse->setStorageImage(0, 45 + 1, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirWriteIndex() + 1].get()); // write
                                m_ReSTIRSSSPassSpatialReuse->setStorageImage(0, 45 + 2, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirWriteIndex() + 2].get()); // write
                                m_ReSTIRSSSPassSpatialReuse->setStorageImage(0, 45 + 3, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirWriteIndex() + 3].get()); // write
                                m_reSTIROptions.swapReservoirIndex();
                            }

                            if (m_reSTIROptions.m_spatial2) {
                                // m_ReSTIRSSSPassSpatialReuse2->setStorageBuffer(0, 30, m_gBuffer[m_renderOptions.getGBufferThisFrameIndex()].get());
                                m_ReSTIRSSSPassSpatialReuse2->setStorageImage(0, 30, m_imageGBuffer[m_renderOptions.getGBufferThisFrameIndex()].get());
                                // m_ReSTIRSSSPassSpatialReuse2->setStorageBuffer(0, 40, m_ReSTIRReservoirBuffer[m_reSTIROptions.getReservoirReadIndex()].get()); // read
                                m_ReSTIRSSSPassSpatialReuse2->setStorageImage(0, 40 + 0, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirReadIndex() + 0].get()); // read
                                m_ReSTIRSSSPassSpatialReuse2->setStorageImage(0, 40 + 1, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirReadIndex() + 1].get()); // read
                                m_ReSTIRSSSPassSpatialReuse2->setStorageImage(0, 40 + 2, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirReadIndex() + 2].get()); // read
                                m_ReSTIRSSSPassSpatialReuse2->setStorageImage(0, 40 + 3, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirReadIndex() + 3].get()); // read
                                // m_ReSTIRSSSPassSpatialReuse2->setStorageBuffer(0, 41, m_ReSTIRReservoirBuffer[m_reSTIROptions.getReservoirWriteIndex()].get()); // write
                                m_ReSTIRSSSPassSpatialReuse2->setStorageImage(0, 45 + 0, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirWriteIndex() + 0].get()); // write
                                m_ReSTIRSSSPassSpatialReuse2->setStorageImage(0, 45 + 1, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirWriteIndex() + 1].get()); // write
                                m_ReSTIRSSSPassSpatialReuse2->setStorageImage(0, 45 + 2, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirWriteIndex() + 2].get()); // write
                                m_ReSTIRSSSPassSpatialReuse2->setStorageImage(0, 45 + 3, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirWriteIndex() + 3].get()); // write
                                m_reSTIROptions.swapReservoirIndex();
                            }

                            // m_ReSTIRSSSPassShade->setStorageBuffer(0, 30, m_gBuffer[m_renderOptions.getGBufferThisFrameIndex()].get());
                            m_ReSTIRSSSPassShade->setStorageImage(0, 30, m_imageGBuffer[m_renderOptions.getGBufferThisFrameIndex()].get());
                            // m_ReSTIRSSSPassShade->setStorageBuffer(0, 40, m_ReSTIRReservoirBuffer[m_reSTIROptions.getReservoirReadIndex()].get()); // read
                            m_ReSTIRSSSPassShade->setStorageImage(0, 40 + 0, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirReadIndex() + 0].get()); // read
                            m_ReSTIRSSSPassShade->setStorageImage(0, 40 + 1, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirReadIndex() + 1].get()); // read
                            m_ReSTIRSSSPassShade->setStorageImage(0, 40 + 2, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirReadIndex() + 2].get()); // read
                            m_ReSTIRSSSPassShade->setStorageImage(0, 40 + 3, m_imageReSTIRReservoirBuffer[4 * m_reSTIROptions.getReservoirReadIndex() + 3].get()); // read

                            m_renderOptions.swapGBufferIndex();
                        },
                        [&] {
                            m_uniformReSTIRSSSPassCandidateGeneration->setVariable<uint32_t>("g_num_lights", m_renderOptions.m_numLights);
                            m_uniformReSTIRSSSPassCandidateGeneration->setVariable<uint32_t>("g_environment_map", m_rayscenes.m_sceneOptions.m_environmentMap);
                            m_uniformReSTIRSSSPassCandidateGeneration->setVariable<glm::vec2>("g_environment_map_rotation", m_rayscenes.m_sceneOptions.m_environmentMapRotation);
                            m_uniformReSTIRSSSPassCandidateGeneration->setVariable<glm::vec3>("g_sky_color", m_rayscenes.m_sceneOptions.m_skyColor);
                            m_uniformReSTIRSSSPassCandidateGeneration->setVariable<uint32_t>("g_normal_mapping", m_renderOptions.m_normalMapping ? 1 : 0);
                            m_uniformReSTIRSSSPassCandidateGeneration->setVariable<uint32_t>("g_sss", m_renderOptions.m_sss ? 1 : 0);
                            m_uniformReSTIRSSSPassCandidateGeneration->setVariable<glm::mat4>("g_vp", m_renderOptions.m_vp);
                            m_uniformReSTIRSSSPassCandidateGeneration->setVariable<glm::mat4>("g_vp_prev", m_renderOptions.m_vp_prev);
                            m_uniformReSTIRSSSPassCandidateGeneration->setVariable<uint32_t>("g_restir_candidate_sss_all_intersections", m_reSTIROptions.m_candidate_sss_all_intersections ? 1 : 0);
                            m_uniformReSTIRSSSPassCandidateGeneration->setVariable<uint32_t>("g_restir_candidate_samples_light", m_reSTIROptions.m_candidate_samples_light);
                            m_uniformReSTIRSSSPassCandidateGeneration->setVariable<uint32_t>("g_restir_candidate_samples_bsdf", m_reSTIROptions.m_candidate_samples_bsdf);
                            m_uniformReSTIRSSSPassCandidateGeneration->setVariable<uint32_t>("g_restir_shift", spatialShift);
                            m_uniformReSTIRSSSPassCandidateGeneration->setVariable<uint32_t>("g_restir_non_sss_surfaces", m_reSTIROptions.m_restir_non_sss_surfaces ? 1 : 0);
                            m_uniformReSTIRSSSPassCandidateGeneration->upload(activeIndex);

                            if (m_reSTIROptions.m_temporal && m_renderOptions.m_framePreviousValid) {
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<glm::vec3>("g_ray_origin_prev", m_renderOptions.m_rayOrigin);
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<glm::vec3>("g_ray_left_bottom_prev", m_renderOptions.m_rayLeftBottom);
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<glm::vec3>("g_ray_left_top_prev", m_renderOptions.m_rayLeftTop);
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<glm::vec3>("g_ray_right_bottom_prev", m_renderOptions.m_rayRightBottom);
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<glm::vec3>("g_ray_right_top_prev", m_renderOptions.m_rayRightTop);
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<glm::mat4>("g_view_to_clip_space", view_to_clip_space);
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<glm::mat4>("g_world_to_view_space", world_to_view_space);
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<uint32_t>("g_num_lights", m_renderOptions.m_numLights);
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<uint32_t>("g_environment_map", m_rayscenes.m_sceneOptions.m_environmentMap);
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<glm::vec2>("g_environment_map_rotation", m_rayscenes.m_sceneOptions.m_environmentMapRotation);
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<glm::vec3>("g_sky_color", m_rayscenes.m_sceneOptions.m_skyColor);
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<uint32_t>("g_normal_mapping", m_renderOptions.m_normalMapping ? 1 : 0);
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<uint32_t>("g_sss", m_renderOptions.m_sss ? 1 : 0);
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<glm::mat4>("g_vp", m_renderOptions.m_vp);
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<glm::mat4>("g_vp_prev", m_renderOptions.m_vp_prev);
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<uint32_t>("g_restir_candidate_samples", m_reSTIROptions.m_candidate_samples_light + m_reSTIROptions.m_candidate_samples_bsdf);
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<uint32_t>("g_restir_shift", temporalShift);
                                m_uniformReSTIRSSSPassTemporalReuse->setVariable<uint32_t>("g_restir_non_sss_surfaces", m_reSTIROptions.m_restir_non_sss_surfaces ? 1 : 0);
                                m_uniformReSTIRSSSPassTemporalReuse->upload(activeIndex);
                            }

                            if (m_reSTIROptions.m_spatial) {
                                m_uniformReSTIRSSSPassSpatialReuse->setVariable<glm::mat4>("g_view_to_clip_space", view_to_clip_space);
                                m_uniformReSTIRSSSPassSpatialReuse->setVariable<glm::mat4>("g_world_to_view_space", world_to_view_space);
                                m_uniformReSTIRSSSPassSpatialReuse->setVariable<uint32_t>("g_num_lights", m_renderOptions.m_numLights);
                                m_uniformReSTIRSSSPassSpatialReuse->setVariable<uint32_t>("g_environment_map", m_rayscenes.m_sceneOptions.m_environmentMap);
                                m_uniformReSTIRSSSPassSpatialReuse->setVariable<glm::vec2>("g_environment_map_rotation", m_rayscenes.m_sceneOptions.m_environmentMapRotation);
                                m_uniformReSTIRSSSPassSpatialReuse->setVariable<glm::vec3>("g_sky_color", m_rayscenes.m_sceneOptions.m_skyColor);
                                m_uniformReSTIRSSSPassSpatialReuse->setVariable<uint32_t>("g_normal_mapping", m_renderOptions.m_normalMapping ? 1 : 0);
                                m_uniformReSTIRSSSPassSpatialReuse->setVariable<uint32_t>("g_sss", m_renderOptions.m_sss ? 1 : 0);
                                m_uniformReSTIRSSSPassSpatialReuse->setVariable<glm::mat4>("g_vp", m_renderOptions.m_vp);
                                m_uniformReSTIRSSSPassSpatialReuse->setVariable<glm::mat4>("g_vp_prev", m_renderOptions.m_vp_prev);
                                m_uniformReSTIRSSSPassSpatialReuse->setVariable<uint32_t>("g_restir_shift", spatialShift);
                                m_uniformReSTIRSSSPassSpatialReuse->setVariable<uint32_t>("g_restir_non_sss_surfaces", m_reSTIROptions.m_restir_non_sss_surfaces ? 1 : 0);
                                m_uniformReSTIRSSSPassSpatialReuse->upload(activeIndex);
                            }

                            if (m_reSTIROptions.m_spatial2) {
                                m_uniformReSTIRSSSPassSpatialReuse2->setVariable<glm::mat4>("g_view_to_clip_space", view_to_clip_space);
                                m_uniformReSTIRSSSPassSpatialReuse2->setVariable<glm::mat4>("g_world_to_view_space", world_to_view_space);
                                m_uniformReSTIRSSSPassSpatialReuse2->setVariable<uint32_t>("g_num_lights", m_renderOptions.m_numLights);
                                m_uniformReSTIRSSSPassSpatialReuse2->setVariable<uint32_t>("g_environment_map", m_rayscenes.m_sceneOptions.m_environmentMap);
                                m_uniformReSTIRSSSPassSpatialReuse2->setVariable<glm::vec2>("g_environment_map_rotation", m_rayscenes.m_sceneOptions.m_environmentMapRotation);
                                m_uniformReSTIRSSSPassSpatialReuse2->setVariable<glm::vec3>("g_sky_color", m_rayscenes.m_sceneOptions.m_skyColor);
                                m_uniformReSTIRSSSPassSpatialReuse2->setVariable<uint32_t>("g_normal_mapping", m_renderOptions.m_normalMapping ? 1 : 0);
                                m_uniformReSTIRSSSPassSpatialReuse2->setVariable<uint32_t>("g_sss", m_renderOptions.m_sss ? 1 : 0);
                                m_uniformReSTIRSSSPassSpatialReuse2->setVariable<glm::mat4>("g_vp", m_renderOptions.m_vp);
                                m_uniformReSTIRSSSPassSpatialReuse2->setVariable<glm::mat4>("g_vp_prev", m_renderOptions.m_vp_prev);
                                m_uniformReSTIRSSSPassSpatialReuse2->setVariable<uint32_t>("g_restir_shift", spatial2Shift);
                                m_uniformReSTIRSSSPassSpatialReuse2->setVariable<uint32_t>("g_restir_non_sss_surfaces", m_reSTIROptions.m_restir_non_sss_surfaces ? 1 : 0);
                                m_uniformReSTIRSSSPassSpatialReuse2->upload(activeIndex);
                            }

                            m_uniformReSTIRSSSPassShade->setVariable<glm::mat4>("g_view_to_clip_space", view_to_clip_space);
                            m_uniformReSTIRSSSPassShade->setVariable<glm::mat4>("g_world_to_view_space", world_to_view_space);
                            m_uniformReSTIRSSSPassShade->setVariable<uint32_t>("g_num_lights", m_renderOptions.m_numLights);
                            m_uniformReSTIRSSSPassShade->setVariable<uint32_t>("g_environment_map", m_rayscenes.m_sceneOptions.m_environmentMap);
                            m_uniformReSTIRSSSPassShade->setVariable<glm::vec2>("g_environment_map_rotation", m_rayscenes.m_sceneOptions.m_environmentMapRotation);
                            m_uniformReSTIRSSSPassShade->setVariable<glm::vec3>("g_sky_color", m_rayscenes.m_sceneOptions.m_skyColor);
                            m_uniformReSTIRSSSPassShade->setVariable<uint32_t>("g_normal_mapping", m_renderOptions.m_normalMapping ? 1 : 0);
                            m_uniformReSTIRSSSPassShade->setVariable<uint32_t>("g_sss", m_renderOptions.m_sss ? 1 : 0);
                            m_uniformReSTIRSSSPassShade->setVariable<glm::mat4>("g_vp", m_renderOptions.m_vp);
                            m_uniformReSTIRSSSPassShade->setVariable<glm::mat4>("g_vp_prev", m_renderOptions.m_vp_prev);
                            m_uniformReSTIRSSSPassShade->setVariable<uint32_t>("g_restir_shift", spatialShift);
                            m_uniformReSTIRSSSPassShade->setVariable<uint32_t>("g_restir_non_sss_surfaces", m_reSTIROptions.m_restir_non_sss_surfaces ? 1 : 0);
                            m_uniformReSTIRSSSPassShade->setVariable<uint32_t>("g_restir_debug", m_reSTIROptions.m_restir_debug);
                            m_uniformReSTIRSSSPassShade->upload(activeIndex);
                        },
                        [&](vk::CommandBuffer &commandBuffer) {
                            // execute shaders
                            {
                                m_ReSTIRSSSPassCandidateGeneration->m_pushConstants.g_pixels = m_renderOptions.m_pixels;
                                m_ReSTIRSSSPassCandidateGeneration->m_pushConstants.g_ray_origin = rayOrigin;
                                m_ReSTIRSSSPassCandidateGeneration->m_pushConstants.g_ray_left_bottom = rayLeftBottom;
                                m_ReSTIRSSSPassCandidateGeneration->m_pushConstants.g_ray_left_top = rayLeftTop;
                                m_ReSTIRSSSPassCandidateGeneration->m_pushConstants.g_ray_right_bottom = rayRightBottom;
                                m_ReSTIRSSSPassCandidateGeneration->m_pushConstants.g_ray_right_top = rayRightTop;
                                m_ReSTIRSSSPassCandidateGeneration->m_pushConstants.g_frame = m_renderOptions.m_frame;
                                m_ReSTIRSSSPassCandidateGeneration->m_pushConstants.g_rng_init = m_renderOptions.m_time;
                                m_ReSTIRSSSPassCandidateGeneration->m_pushConstants.g_rng_init_offset = 10;

                                m_ReSTIRSSSPassCandidateGeneration->execute(
                                        [&](const vk::CommandBuffer &cbf, const vk::PipelineLayout &pipelineLayout) {
                                            cbf.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(ReSTIRSSSPassCandidateGeneration::PushConstants), &m_ReSTIRSSSPassCandidateGeneration->m_pushConstants);
                                        },
                                        [](const vk::CommandBuffer &cbf) {
                                            constexpr vk::MemoryBarrier memoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
                                            cbf.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags{}, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                                        },
                                        commandBuffer);
                            }

                            if (m_reSTIROptions.m_temporal && m_renderOptions.m_framePreviousValid) {
                                m_ReSTIRSSSPassTemporalReuse->m_pushConstants.g_pixels = m_renderOptions.m_pixels;
                                m_ReSTIRSSSPassTemporalReuse->m_pushConstants.g_ray_origin = rayOrigin;
                                m_ReSTIRSSSPassTemporalReuse->m_pushConstants.g_ray_left_bottom = rayLeftBottom;
                                m_ReSTIRSSSPassTemporalReuse->m_pushConstants.g_ray_left_top = rayLeftTop;
                                m_ReSTIRSSSPassTemporalReuse->m_pushConstants.g_ray_right_bottom = rayRightBottom;
                                m_ReSTIRSSSPassTemporalReuse->m_pushConstants.g_ray_right_top = rayRightTop;
                                m_ReSTIRSSSPassTemporalReuse->m_pushConstants.g_frame = m_renderOptions.m_frame;
                                m_ReSTIRSSSPassTemporalReuse->m_pushConstants.g_rng_init = m_renderOptions.m_time;
                                m_ReSTIRSSSPassTemporalReuse->m_pushConstants.g_rng_init_offset = 11;

                                m_ReSTIRSSSPassTemporalReuse->execute(
                                        [&](const vk::CommandBuffer &cbf, const vk::PipelineLayout &pipelineLayout) {
                                            cbf.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(ReSTIRSSSPassTemporalReuse::PushConstants), &m_ReSTIRSSSPassTemporalReuse->m_pushConstants);
                                        },
                                        [](const vk::CommandBuffer &cbf) {
                                            constexpr vk::MemoryBarrier memoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
                                            cbf.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags{}, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                                        },
                                        commandBuffer);
                            }

                            if (m_reSTIROptions.m_spatial) {
                                m_ReSTIRSSSPassSpatialReuse->m_pushConstants.g_pixels = m_renderOptions.m_pixels;
                                m_ReSTIRSSSPassSpatialReuse->m_pushConstants.g_ray_origin = rayOrigin;
                                m_ReSTIRSSSPassSpatialReuse->m_pushConstants.g_ray_left_bottom = rayLeftBottom;
                                m_ReSTIRSSSPassSpatialReuse->m_pushConstants.g_ray_left_top = rayLeftTop;
                                m_ReSTIRSSSPassSpatialReuse->m_pushConstants.g_ray_right_bottom = rayRightBottom;
                                m_ReSTIRSSSPassSpatialReuse->m_pushConstants.g_ray_right_top = rayRightTop;
                                m_ReSTIRSSSPassSpatialReuse->m_pushConstants.g_frame = m_renderOptions.m_frame;
                                m_ReSTIRSSSPassSpatialReuse->m_pushConstants.g_rng_init = m_renderOptions.m_time;
                                m_ReSTIRSSSPassSpatialReuse->m_pushConstants.g_rng_init_offset = 12;

                                m_ReSTIRSSSPassSpatialReuse->execute(
                                        [&](const vk::CommandBuffer &cbf, const vk::PipelineLayout &pipelineLayout) {
                                            cbf.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(ReSTIRSSSPassSpatialReuse::PushConstants), &m_ReSTIRSSSPassSpatialReuse->m_pushConstants);
                                        },
                                        [](const vk::CommandBuffer &cbf) {
                                            constexpr vk::MemoryBarrier memoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
                                            cbf.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags{}, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                                        },
                                        commandBuffer);
                            }

                            if (m_reSTIROptions.m_spatial2) {
                                m_ReSTIRSSSPassSpatialReuse2->m_pushConstants.g_pixels = m_renderOptions.m_pixels;
                                m_ReSTIRSSSPassSpatialReuse2->m_pushConstants.g_ray_origin = rayOrigin;
                                m_ReSTIRSSSPassSpatialReuse2->m_pushConstants.g_ray_left_bottom = rayLeftBottom;
                                m_ReSTIRSSSPassSpatialReuse2->m_pushConstants.g_ray_left_top = rayLeftTop;
                                m_ReSTIRSSSPassSpatialReuse2->m_pushConstants.g_ray_right_bottom = rayRightBottom;
                                m_ReSTIRSSSPassSpatialReuse2->m_pushConstants.g_ray_right_top = rayRightTop;
                                m_ReSTIRSSSPassSpatialReuse2->m_pushConstants.g_frame = m_renderOptions.m_frame;
                                m_ReSTIRSSSPassSpatialReuse2->m_pushConstants.g_rng_init = m_renderOptions.m_time;
                                m_ReSTIRSSSPassSpatialReuse2->m_pushConstants.g_rng_init_offset = 13;

                                m_ReSTIRSSSPassSpatialReuse2->execute(
                                        [&](const vk::CommandBuffer &cbf, const vk::PipelineLayout &pipelineLayout) {
                                            cbf.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(ReSTIRSSSPassSpatialReuse::PushConstants), &m_ReSTIRSSSPassSpatialReuse2->m_pushConstants);
                                        },
                                        [](const vk::CommandBuffer &cbf) {
                                            constexpr vk::MemoryBarrier memoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
                                            cbf.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags{}, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                                        },
                                        commandBuffer);
                            }

                            {
                                m_ReSTIRSSSPassShade->m_pushConstants.g_pixels = m_renderOptions.m_pixels;
                                m_ReSTIRSSSPassShade->m_pushConstants.g_ray_origin = rayOrigin;
                                m_ReSTIRSSSPassShade->m_pushConstants.g_ray_left_bottom = rayLeftBottom;
                                m_ReSTIRSSSPassShade->m_pushConstants.g_ray_left_top = rayLeftTop;
                                m_ReSTIRSSSPassShade->m_pushConstants.g_ray_right_bottom = rayRightBottom;
                                m_ReSTIRSSSPassShade->m_pushConstants.g_ray_right_top = rayRightTop;
                                m_ReSTIRSSSPassShade->m_pushConstants.g_frame = m_renderOptions.m_frame;
                                m_ReSTIRSSSPassShade->m_pushConstants.g_rng_init = m_renderOptions.m_time;
                                m_ReSTIRSSSPassShade->m_pushConstants.g_rng_init_offset = 14;

                                m_ReSTIRSSSPassShade->execute(
                                        [&](const vk::CommandBuffer &cbf, const vk::PipelineLayout &pipelineLayout) {
                                            cbf.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(ReSTIRSSSPassShade::PushConstants), &m_ReSTIRSSSPassShade->m_pushConstants);
                                        },
                                        [](const vk::CommandBuffer &cbf) {
                                            constexpr vk::MemoryBarrier memoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
                                            cbf.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags{}, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                                        },
                                        commandBuffer);
                            }

#ifdef OPTIX_SUPPORT
                            if (executeDenoiser) {
                                m_denoiser->imageToBuffer(commandBuffer, {m_imageAccumulationBuffer, m_imageAlbedoBuffer, m_imageNormalBuffer});
                            }
#endif
                        },
                        1, &waitBeforeReSTIRSSSimelineSemaphore, &waitBeforeReSTIRSSSTimelineSemaphoreValue, &waitBeforeReSTIRSSSDstStageMask);
            } else {
                m_ReSTIRSSSPass->execute(
                        [&] {
                            // m_ReSTIRSSSPassPathtrace->setStorageBuffer(0, 30, m_gBuffer[m_renderOptions.getGBufferThisFrameIndex()].get());
                            m_passPathtrace->setStorageImage(0, 30, m_imageGBuffer[m_renderOptions.getGBufferThisFrameIndex()].get());

                            m_renderOptions.swapGBufferIndex();
                        },
                        [&] {
                            m_uniformPassPathtrace->setVariable<glm::mat4>("g_view_to_clip_space", view_to_clip_space);
                            m_uniformPassPathtrace->setVariable<glm::mat4>("g_world_to_view_space", world_to_view_space);
                            m_uniformPassPathtrace->setVariable<uint32_t>("g_num_lights", m_renderOptions.m_numLights);
                            m_uniformPassPathtrace->setVariable<uint32_t>("g_environment_map", m_rayscenes.m_sceneOptions.m_environmentMap);
                            m_uniformPassPathtrace->setVariable<glm::vec2>("g_environment_map_rotation", m_rayscenes.m_sceneOptions.m_environmentMapRotation);
                            m_uniformPassPathtrace->setVariable<glm::vec3>("g_sky_color", m_rayscenes.m_sceneOptions.m_skyColor);
                            m_uniformPassPathtrace->setVariable<uint32_t>("g_normal_mapping", m_renderOptions.m_normalMapping ? 1 : 0);
                            m_uniformPassPathtrace->setVariable<uint32_t>("g_sss", m_renderOptions.m_sss ? 1 : 0);
                            m_uniformPassPathtrace->setVariable<glm::mat4>("g_vp", m_renderOptions.m_vp);
                            m_uniformPassPathtrace->setVariable<glm::mat4>("g_vp_prev", m_renderOptions.m_vp_prev);
                            m_uniformPassPathtrace->setVariable<uint32_t>("g_spp", m_renderOptions.m_spp);
                            m_uniformPassPathtrace->setVariable<uint32_t>("g_restir_non_sss_surfaces", m_reSTIROptions.m_restir_non_sss_surfaces ? 1 : 0);
                            m_uniformPassPathtrace->upload(activeIndex);
                        },
                        [&](vk::CommandBuffer &commandBuffer) {
                            // execute shaders
                            {
                                m_passPathtrace->m_pushConstants.g_pixels = m_renderOptions.m_pixels;
                                m_passPathtrace->m_pushConstants.g_ray_origin = rayOrigin;
                                m_passPathtrace->m_pushConstants.g_ray_left_bottom = rayLeftBottom;
                                m_passPathtrace->m_pushConstants.g_ray_left_top = rayLeftTop;
                                m_passPathtrace->m_pushConstants.g_ray_right_bottom = rayRightBottom;
                                m_passPathtrace->m_pushConstants.g_ray_right_top = rayRightTop;
                                m_passPathtrace->m_pushConstants.g_frame = m_renderOptions.m_frame;
                                m_passPathtrace->m_pushConstants.g_rng_init = m_renderOptions.m_time;
                                m_passPathtrace->m_pushConstants.g_rng_init_offset = 10;

                                m_passPathtrace->execute(
                                        [&](const vk::CommandBuffer &cbf, const vk::PipelineLayout &pipelineLayout) {
                                            cbf.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(ReSTIRSSSPassShade::PushConstants), &m_passPathtrace->m_pushConstants);
                                        },
                                        [](const vk::CommandBuffer &cbf) {
                                            constexpr vk::MemoryBarrier memoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
                                            cbf.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags{}, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
                                        },
                                        commandBuffer);
                            }

#ifdef OPTIX_SUPPORT
                            if (m_renderOptions.m_denoiser) {
                                m_denoiser->imageToBuffer(commandBuffer, {m_imageAccumulationBuffer, m_imageAlbedoBuffer, m_imageNormalBuffer});
                            }
#endif
                        },
                        1, &waitBeforeReSTIRSSSimelineSemaphore, &waitBeforeReSTIRSSSTimelineSemaphoreValue, &waitBeforeReSTIRSSSDstStageMask);
            }
        }

        // post processing passes
        {
            vk::Semaphore waitBeforePostProcessingTimelineSemaphore;
            uint64_t waitBeforePostProcessingTimelineSemaphoreValue;
            vk::PipelineStageFlags waitBeforePostProcessingDstStageMask = vk::PipelineStageFlagBits::eComputeShader;
            if (executeReSTIRSSSPass) {
#ifdef OPTIX_SUPPORT
                if (executeDenoiser) {
                    m_denoiser->denoiseImageBuffer(m_ReSTIRSSSPass->getTimelineSemaphoreValue());
                    waitBeforePostProcessingTimelineSemaphore = m_denoiser->getTimelineSemaphore();
                    waitBeforePostProcessingTimelineSemaphoreValue = m_denoiser->getTimelineSemaphoreValue();
                } else {
                    waitBeforePostProcessingTimelineSemaphore = m_ReSTIRSSSPass->getTimelineSemaphore();
                    waitBeforePostProcessingTimelineSemaphoreValue = m_ReSTIRSSSPass->getTimelineSemaphoreValue();
                }
#else
                waitBeforePostProcessingTimelineSemaphore = m_ReSTIRSSSPass->getTimelineSemaphore();
                waitBeforePostProcessingTimelineSemaphoreValue = m_ReSTIRSSSPass->getTimelineSemaphoreValue();
#endif
            } else {
                waitBeforePostProcessingTimelineSemaphore = m_preProcessingPass->getTimelineSemaphore();
                waitBeforePostProcessingTimelineSemaphoreValue = m_preProcessingPass->getTimelineSemaphoreValue();
            }

            m_postProcessingPass->execute(
                    [] {
                    },
                    [&] {
                    },
                    [&](vk::CommandBuffer &commandBuffer) {
#ifdef OPTIX_SUPPORT
                        if (executeDenoiser) {
                            m_denoiser->bufferToImage(commandBuffer, m_imageFramebuffer);
                        }
#endif

                        {
                            m_passTonemapper->m_pushConstants.g_pixels = m_renderOptions.m_pixels;
                            m_passTonemapper->m_pushConstants.g_tonemapper = m_renderOptions.m_tonemapper;

                            m_passTonemapper->execute(
                                    [&](const vk::CommandBuffer &cbf, const vk::PipelineLayout &pipelineLayout) {
                                        cbf.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(PassTonemapper::PushConstants), &m_passTonemapper->m_pushConstants);
                                    },
                                    [](const vk::CommandBuffer &cbf) {
                                    },
                                    commandBuffer);
                        }
                    },
                    1, &waitBeforePostProcessingTimelineSemaphore, &waitBeforePostProcessingTimelineSemaphoreValue, &waitBeforePostProcessingDstStageMask);

            rendererResult->m_waitTimelineSemaphore = m_postProcessingPass->getTimelineSemaphore();
            rendererResult->m_waitTimelineSemaphoreValue = m_postProcessingPass->getTimelineSemaphoreValue();
        }

        m_renderOptions.m_time++;
        m_renderOptions.m_vp_prev = m_renderOptions.m_vp;
        m_renderOptions.m_rayOrigin = rayOrigin;
        m_renderOptions.m_rayLeftBottom = rayLeftBottom;
        m_renderOptions.m_rayLeftTop = rayLeftTop;
        m_renderOptions.m_rayRightBottom = rayRightBottom;
        m_renderOptions.m_rayRightTop = rayRightTop;
        m_renderOptions.m_frame++;
        m_renderOptions.m_framePreviousValid = true;
        m_renderOptions.m_validActiveIndex = activeIndex;

        rendererResult->m_image = m_imageFramebuffer.get();
    }

    void ReSTIRSSS::releaseResources() {
        m_raydata.releaseResources();
    }

    void ReSTIRSSS::releaseShaderResources() {
        RAVEN_PASS_RELEASE(m_preProcessingPass);
        RAVEN_PASS_RELEASE(m_ReSTIRSSSPass);
        RAVEN_PASS_RELEASE(m_postProcessingPass);

        RAVEN_PASS_SHADER_RELEASE(m_passGBuffer);
        RAVEN_PASS_SHADER_RELEASE(m_passSurface);
        RAVEN_PASS_SHADER_RELEASE(m_passDebug);

        RAVEN_PASS_SHADER_RELEASE(m_ReSTIRSSSPassCandidateGeneration);
        RAVEN_PASS_SHADER_RELEASE(m_ReSTIRSSSPassTemporalReuse);
        RAVEN_PASS_SHADER_RELEASE(m_ReSTIRSSSPassSpatialReuse);
        RAVEN_PASS_SHADER_RELEASE(m_ReSTIRSSSPassSpatialReuse2);
        RAVEN_PASS_SHADER_RELEASE(m_ReSTIRSSSPassShade);
        RAVEN_PASS_SHADER_RELEASE(m_passPathtrace);

        RAVEN_PASS_SHADER_RELEASE(m_passTonemapper);
    }

    void ReSTIRSSS::releaseSwapchainResources() {
        RAVEN_IMAGE_RELEASE(m_imageFramebuffer);
        RAVEN_IMAGE_RELEASE(m_imageAccumulationBuffer);
        RAVEN_IMAGE_RELEASE(m_imageAlbedoBuffer);
        RAVEN_IMAGE_RELEASE(m_imageNormalBuffer);
        for (const auto &img: m_imageGBuffer) {
            RAVEN_IMAGE_RELEASE(img);
        }
#ifdef RESTIRSSS_WRITE_DEBUG_IMAGES
        for (const auto &img: m_imageReSTIRDebug) {
            RAVEN_IMAGE_RELEASE(img);
        }
#endif
        for (const auto &img: m_imageReSTIRReservoirBuffer) {
            RAVEN_IMAGE_RELEASE(img);
        }
        releaseDenoiserEngine();
    }

    void ReSTIRSSS::resetFrame(GPUContext *gpuContext) {
        m_renderOptions.m_frame = 0;
        m_renderOptions.m_framePreviousValid = false;
        m_renderOptions.m_time = 0;

        const glm::mat4 view_to_clip_space = gpuContext->getCamera()->getViewToClipSpace();
        const glm::mat4 world_to_view_space = gpuContext->getCamera()->getWorldToViewSpace();
        const glm::mat4 vp = view_to_clip_space * world_to_view_space;
        m_renderOptions.m_vp_prev = vp;
        m_renderOptions.m_vp = vp;

        m_passGBuffer->invalidateTime();
        m_passSurface->invalidateTime();
        m_passDebug->invalidateTime();

        m_ReSTIRSSSPassCandidateGeneration->invalidateTime();
        m_ReSTIRSSSPassTemporalReuse->invalidateTime();
        m_ReSTIRSSSPassSpatialReuse->invalidateTime();
        m_ReSTIRSSSPassSpatialReuse2->invalidateTime();
        m_ReSTIRSSSPassShade->invalidateTime();
        m_passPathtrace->invalidateTime();
    }

    void ReSTIRSSS::screenshot(GPUContext *gpuContext, std::shared_ptr<Image> &image, const std::string &path) const {
        gpuContext->m_device.waitIdle();

        if (m_renderOptions.m_tonemapper == 1) {
            // create target image
            auto settings = Image::ImageSettings{.m_width = image->getWidth(), .m_height = image->getHeight(), .m_format = vk::Format::eR8G8B8A8Unorm, .m_tiling = vk::ImageTiling::eLinear, .m_usageFlags = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage, .m_name = "screenshotBuffer"};
            auto screenshotImage = std::make_shared<Image>(gpuContext, settings);
            screenshotImage->create();

            const auto priorLayout = image->getLayout();

            gpuContext->executeCommands([image, screenshotImage, priorLayout](vk::CommandBuffer commandBuffer) {
                {
                    vk::ImageMemoryBarrier barrier{};
                    barrier.oldLayout = priorLayout;
                    barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
                    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.image = image->getImage();
                    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                    barrier.subresourceRange.baseMipLevel = 0;
                    barrier.subresourceRange.levelCount = 1;
                    barrier.subresourceRange.baseArrayLayer = 0;
                    barrier.subresourceRange.layerCount = 1;
                    barrier.srcAccessMask = vk::AccessFlagBits::eNone;
                    barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
                    auto sourceStage = vk::PipelineStageFlagBits::eHost;
                    auto destinationStage = vk::PipelineStageFlagBits::eTransfer;
                    commandBuffer.pipelineBarrier(
                            sourceStage, destinationStage,
                            {},
                            nullptr, nullptr, {barrier});
                }
                {
                    vk::ImageMemoryBarrier barrier{};
                    barrier.oldLayout = screenshotImage->getLayout();
                    barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
                    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.image = screenshotImage->getImage();
                    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                    barrier.subresourceRange.baseMipLevel = 0;
                    barrier.subresourceRange.levelCount = 1;
                    barrier.subresourceRange.baseArrayLayer = 0;
                    barrier.subresourceRange.layerCount = 1;
                    barrier.srcAccessMask = vk::AccessFlagBits::eNone;
                    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
                    auto sourceStage = vk::PipelineStageFlagBits::eHost;
                    auto destinationStage = vk::PipelineStageFlagBits::eTransfer;
                    commandBuffer.pipelineBarrier(
                            sourceStage, destinationStage,
                            {},
                            nullptr, nullptr, {barrier});
                }

                {
                    vk::ImageBlit copyRegion{};
                    copyRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                    copyRegion.srcSubresource.layerCount = 1;
                    copyRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                    copyRegion.dstSubresource.layerCount = 1;
                    copyRegion.srcOffsets[0] = vk::Offset3D{0, 0, 0};
                    copyRegion.srcOffsets[1] = vk::Offset3D{static_cast<int32_t>(image->getWidth()), static_cast<int32_t>(image->getHeight()), 1};
                    copyRegion.dstOffsets[0] = vk::Offset3D{0, 0, 0};
                    copyRegion.dstOffsets[1] = vk::Offset3D{static_cast<int32_t>(image->getWidth()), static_cast<int32_t>(image->getHeight()), 1};
                    auto filter = vk::Filter::eNearest;
                    commandBuffer.blitImage(image->getImage(), vk::ImageLayout::eTransferSrcOptimal, screenshotImage->getImage(), vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion, filter);
                }

                {
                    vk::ImageMemoryBarrier barrier{};
                    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
                    barrier.newLayout = vk::ImageLayout::eGeneral;
                    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.image = screenshotImage->getImage();
                    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                    barrier.subresourceRange.baseMipLevel = 0;
                    barrier.subresourceRange.levelCount = 1;
                    barrier.subresourceRange.baseArrayLayer = 0;
                    barrier.subresourceRange.layerCount = 1;
                    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                    barrier.dstAccessMask = vk::AccessFlagBits::eNone;
                    auto sourceStage = vk::PipelineStageFlagBits::eTransfer;
                    auto destinationStage = vk::PipelineStageFlagBits::eHost;
                    commandBuffer.pipelineBarrier(
                            sourceStage, destinationStage,
                            {},
                            nullptr, nullptr, {barrier});
                    screenshotImage->setLayout(vk::ImageLayout::eGeneral);
                }
                {
                    vk::ImageMemoryBarrier barrier{};
                    barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
                    barrier.newLayout = priorLayout;
                    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.image = image->getImage();
                    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                    barrier.subresourceRange.baseMipLevel = 0;
                    barrier.subresourceRange.levelCount = 1;
                    barrier.subresourceRange.baseArrayLayer = 0;
                    barrier.subresourceRange.layerCount = 1;
                    barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
                    barrier.dstAccessMask = vk::AccessFlagBits::eNone;
                    auto sourceStage = vk::PipelineStageFlagBits::eTransfer;
                    auto destinationStage = vk::PipelineStageFlagBits::eHost;
                    commandBuffer.pipelineBarrier(
                            sourceStage, destinationStage,
                            {},
                            nullptr, nullptr, {barrier});
                }
            });

            constexpr vk::ImageSubresource subResource{vk::ImageAspectFlagBits::eColor, 0, 0};
            vk::SubresourceLayout subResourceLayout;
            gpuContext->m_device.getImageSubresourceLayout(screenshotImage->getImage(), &subResource, &subResourceLayout);

            std::vector<uint8_t> bytes(screenshotImage->getSizeBytes());
            screenshotImage->download(bytes.data());

            const uint8_t *data = bytes.data();
            data += subResourceLayout.offset;

            stbi_write_png(path.c_str(), static_cast<int>(image->getWidth()), static_cast<int>(image->getHeight()), 4, data, static_cast<int>(image->getWidth()) * 4);
        } else {
            std::vector<float> pixels(image->getSizeBytes());
            image->download(pixels.data());
            // ImagePFM::writeFilePFM(pixels, ImagePFM::COLOR, static_cast<int>(image->getWidth()), static_cast<int>(image->getHeight()), path + ".pfm");
            const std::string hdr = path + ".hdr";
            stbi_write_hdr(hdr.c_str(), static_cast<int>(image->getWidth()), static_cast<int>(image->getHeight()), 4, pixels.data());
        }

        gpuContext->m_device.waitIdle();
    }

    void ReSTIRSSS::passesSetMaterialBuffer() const {
        m_passGBuffer->setStorageBuffer(0, 11, m_raydata.m_materialBuffer.get());
        m_passSurface->setStorageBuffer(0, 11, m_raydata.m_materialBuffer.get());
        m_passDebug->setStorageBuffer(0, 11, m_raydata.m_materialBuffer.get());

        m_ReSTIRSSSPassCandidateGeneration->setStorageBuffer(0, 11, m_raydata.m_materialBuffer.get());
        m_ReSTIRSSSPassTemporalReuse->setStorageBuffer(0, 11, m_raydata.m_materialBuffer.get());
        m_ReSTIRSSSPassSpatialReuse->setStorageBuffer(0, 11, m_raydata.m_materialBuffer.get());
        m_ReSTIRSSSPassSpatialReuse2->setStorageBuffer(0, 11, m_raydata.m_materialBuffer.get());
        m_ReSTIRSSSPassShade->setStorageBuffer(0, 11, m_raydata.m_materialBuffer.get());
        m_passPathtrace->setStorageBuffer(0, 11, m_raydata.m_materialBuffer.get());
    }

    void ReSTIRSSS::passesSetLightBuffer() const {
        m_passGBuffer->setStorageBuffer(0, 12, m_raydata.m_lightBuffer.get());
        m_passGBuffer->setStorageBuffer(0, 12, m_raydata.m_lightBuffer.get());
        m_passSurface->setStorageBuffer(0, 12, m_raydata.m_lightBuffer.get());

        m_ReSTIRSSSPassCandidateGeneration->setStorageBuffer(0, 12, m_raydata.m_lightBuffer.get());
        m_ReSTIRSSSPassTemporalReuse->setStorageBuffer(0, 12, m_raydata.m_lightBuffer.get());
        m_ReSTIRSSSPassSpatialReuse->setStorageBuffer(0, 12, m_raydata.m_lightBuffer.get());
        m_ReSTIRSSSPassSpatialReuse2->setStorageBuffer(0, 12, m_raydata.m_lightBuffer.get());
        m_ReSTIRSSSPassShade->setStorageBuffer(0, 12, m_raydata.m_lightBuffer.get());
        m_passPathtrace->setStorageBuffer(0, 12, m_raydata.m_lightBuffer.get());
    }

    void ReSTIRSSS::createDenoiserEngine(GPUContext *gpuContext, const vk::Extent2D extent) {
#ifdef OPTIX_SUPPORT
        if (m_denoiser) {
            // recreate timeline semaphore
            m_ReSTIRSSSPass->recreateSyncObjects();
        }

        // create denoiser
        m_denoiser = std::make_unique<DenoiserOptiX>(gpuContext);
        OptixDenoiserOptions d_options;
        d_options.guideAlbedo = m_renderOptions.m_denoiserGuideAlbedo ? 1u : 0u;
        d_options.guideNormal = m_renderOptions.m_denoiserGuideNormal ? 1u : 0u;
        m_denoiser->create(d_options, OPTIX_PIXEL_FORMAT_FLOAT4, extent, m_ReSTIRSSSPass->getTimelineSemaphore());
#endif
    }

    void ReSTIRSSS::releaseDenoiserEngine() const {
#ifdef OPTIX_SUPPORT
        if (m_denoiser) {
            // destroy denoiser
            m_denoiser->release();
        }
#endif
    }

    void ReSTIRSSS::loadPreset(const std::string &preset) {
        if (preset == "pathtracing") {
            m_renderOptions.m_sss = true;
            m_renderOptions.m_renderer = RENDERER_PATHTRACE;
            m_renderOptions.m_spp = 1;
        } else if (preset == "hybrid") {
            m_renderOptions.m_sss = true;
            m_renderOptions.m_renderer = RENDERER_RESTIRSSS;
            m_reSTIROptions.m_candidate_sss_all_intersections = true;
            m_reSTIROptions.m_candidate_samples_light = 1;
            m_reSTIROptions.m_candidate_samples_bsdf = 0;
            m_reSTIROptions.m_shift = RESTIRSSS_SHIFT_HYBRID;
            m_reSTIROptions.m_temporal = true;
            m_reSTIROptions.m_spatial = true;
            m_reSTIROptions.m_spatial_count = 4;
            m_reSTIROptions.m_spatial2 = false;
        } else if (preset == "sequential") {
            m_renderOptions.m_sss = true;
            m_renderOptions.m_renderer = RENDERER_RESTIRSSS;
            m_reSTIROptions.m_candidate_sss_all_intersections = true;
            m_reSTIROptions.m_candidate_samples_light = 1;
            m_reSTIROptions.m_candidate_samples_bsdf = 0;
            m_reSTIROptions.m_shift = RESTIRSSS_SHIFT_SEQUENTIAL;
            m_reSTIROptions.m_temporal = true;
            m_reSTIROptions.m_spatial = true;
            m_reSTIROptions.m_spatial_count = 2;
            m_reSTIROptions.m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION;
            m_reSTIROptions.m_spatial2 = true;
            m_reSTIROptions.m_spatial2_count = 2;
            m_reSTIROptions.m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION;
        }
    }
} // namespace raven