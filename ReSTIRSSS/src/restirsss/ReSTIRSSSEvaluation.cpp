#include "restirsss/ReSTIRSSSEvaluation.h"

namespace raven {
    bool ReSTIRSSSEvaluation::evaluateAjax() const {
        constexpr glm::vec3 position = {AJAX_POSITION};
        constexpr glm::vec3 orientation = {AJAX_ORIENTATION};
        constexpr glm::vec3 trajectoryStart = {AJAX_TRAJECTORY_START};
        constexpr int32_t width = 720;
        constexpr int32_t height = 1080;

        std::string appName = "ReSTIR SSS";
        {
            const std::string subdirectory = m_directory + "/ajax";
            if (!std::filesystem::create_directories(subdirectory)) {
                return false;
            }
            const std::string namePrefix = "ajax_";

            if (m_groundTruth) {
                // GROUND TRUTH
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 8192,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 8,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 8192,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "ground_truth",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 1,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_1spp",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 1,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_1spp_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 6,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_6spp",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 6,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_6spp_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 12,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_12spp",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 12,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_12spp_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIAL - RECONNECTION
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_temporal = false,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_reconnection",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIAL - RECONNECTION
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_temporal = false,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_reconnection_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIOTEMPORAL - RECONNECTION
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_temporal = true,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_reconnection",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIOTEMPORAL - RECONNECTION
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_temporal = true,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_reconnection_denoised",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIAL - DELAYED RECONNECTION
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                        .m_temporal = false,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_delayed_reconnection",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIAL - DELAYED RECONNECTION
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                        .m_temporal = false,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_delayed_reconnection_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIOTEMPORAL - DELAYED RECONNECTION
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                        .m_temporal = true,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_delayed_reconnection",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIOTEMPORAL - DELAYED RECONNECTION
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                        .m_temporal = true,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_delayed_reconnection_denoised",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIAL - HYBRID
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_HYBRID,
                        .m_temporal = false,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_hybrid",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIAL - HYBRID
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_HYBRID,
                        .m_temporal = false,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_hybrid_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIOTEMPORAL - HYBRID
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_HYBRID,
                        .m_temporal = true,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_hybrid",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIOTEMPORAL - HYBRID
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_HYBRID,
                        .m_temporal = true,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_hybrid_denoised",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIAL - SEQUENTIAL
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_SEQUENTIAL,
                        .m_temporal = false,
                        .m_temporal_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial = true,
                        .m_spatial_count = 2,
                        .m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial2 = true,
                        .m_spatial2_count = 2,
                        .m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_sequential",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIAL - SEQUENTIAL
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_SEQUENTIAL,
                        .m_temporal = false,
                        .m_temporal_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial = true,
                        .m_spatial_count = 2,
                        .m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial2 = true,
                        .m_spatial2_count = 2,
                        .m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_sequential_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIOTEMPORAL - SEQUENTIAL
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_SEQUENTIAL,
                        .m_temporal = true,
                        .m_temporal_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial = true,
                        .m_spatial_count = 2,
                        .m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial2 = true,
                        .m_spatial2_count = 2,
                        .m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_sequential",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIOTEMPORAL - SEQUENTIAL
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_SEQUENTIAL,
                        .m_temporal = true,
                        .m_temporal_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial = true,
                        .m_spatial_count = 2,
                        .m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial2 = true,
                        .m_spatial2_count = 2,
                        .m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_sequential_denoised",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            if (m_png) {
                {
                    std::ofstream compareScript;
                    compareScript.open(subdirectory + "/compare_rmse.sh");
                    compareScript << "#!/bin/bash\n";
                    compareScript << "printf \"RMSE\\n\"\n";
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_1spp");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_6spp");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_12spp");
                    writeRMSECompare(compareScript, namePrefix, "spatial_reconnection");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    writeRMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    writeRMSECompare(compareScript, namePrefix, "spatial_hybrid");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid");
                    writeRMSECompare(compareScript, namePrefix, "spatial_sequential");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_sequential");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_1spp_denoised");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_6spp_denoised");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_12spp_denoised");
                    writeRMSECompare(compareScript, namePrefix, "spatial_reconnection_denoised");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection_denoised");
                    writeRMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection_denoised");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection_denoised");
                    writeRMSECompare(compareScript, namePrefix, "spatial_hybrid_denoised");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid_denoised");
                    writeRMSECompare(compareScript, namePrefix, "spatial_sequential_denoised");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_sequential_denoised");
                    compareScript.close();
                }
                {
                    std::ofstream compareScript;
                    compareScript.open(subdirectory + "/compare_mse.sh");
                    compareScript << "#!/bin/bash\n";
                    compareScript << "printf \"MSE\\n\"\n";
                    writeMSECompare(compareScript, namePrefix, "pathtracer_1spp");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_6spp");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_12spp");
                    writeMSECompare(compareScript, namePrefix, "spatial_reconnection");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    writeMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    writeMSECompare(compareScript, namePrefix, "spatial_hybrid");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid");
                    writeMSECompare(compareScript, namePrefix, "spatial_sequential");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_sequential");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_1spp_denoised");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_6spp_denoised");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_12spp_denoised");
                    writeMSECompare(compareScript, namePrefix, "spatial_reconnection_denoised");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection_denoised");
                    writeMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection_denoised");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection_denoised");
                    writeMSECompare(compareScript, namePrefix, "spatial_hybrid_denoised");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid_denoised");
                    writeMSECompare(compareScript, namePrefix, "spatial_sequential_denoised");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_sequential_denoised");
                    compareScript.close();
                }
                {
                    std::ofstream compareScript;
                    compareScript.open(subdirectory + "/compare_flip.sh");
                    compareScript << "#!/bin/bash\n";
                    compareScript << "printf \"FLIP\\n\"\n";
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_1spp");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_6spp");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_12spp");
                    writeFLIPCompare(compareScript, namePrefix, "spatial_reconnection");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    writeFLIPCompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    writeFLIPCompare(compareScript, namePrefix, "spatial_hybrid");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_hybrid");
                    writeFLIPCompare(compareScript, namePrefix, "spatial_sequential");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_sequential");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_1spp_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_6spp_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_12spp_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "spatial_reconnection_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_reconnection_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "spatial_delayed_reconnection_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "spatial_hybrid_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_hybrid_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "spatial_sequential_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_sequential_denoised");
                    compareScript.close();
                }
            } else {
                std::ofstream compareScript;
                compareScript.open(subdirectory + "/compare_hdr_rmse.sh");
                compareScript << "#!/bin/bash\n";
                compareScript << "printf \"HDR RMSE\\n\"\n";
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_1spp");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_6spp");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_12spp");
                writeHDRRMSECompare(compareScript, namePrefix, "spatial_reconnection");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                writeHDRRMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                writeHDRRMSECompare(compareScript, namePrefix, "spatial_hybrid");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid");
                writeHDRRMSECompare(compareScript, namePrefix, "spatial_sequential");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_sequential");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_1spp_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_6spp_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_12spp_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "spatial_reconnection_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "spatial_hybrid_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "spatial_sequential_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_sequential_denoised");
                compareScript.close();
            }
        }

        return true;
    }

    bool ReSTIRSSSEvaluation::evaluateAjaxManyLights() const {
        constexpr glm::vec3 position = {AJAX_POSITION};
        constexpr glm::vec3 orientation = {AJAX_ORIENTATION};
        constexpr glm::vec3 trajectoryStart = {AJAX_TRAJECTORY_START};
        constexpr int32_t width = 720;
        constexpr int32_t height = 1080;

        std::string appName = "ReSTIR SSS";
        {
            const std::string subdirectory = m_directory + "/ajax_manylights";
            if (!std::filesystem::create_directories(subdirectory)) {
                return false;
            }
            const std::string namePrefix = "ajax_";

            if (m_groundTruth) {
                // GROUND TRUTH
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 8192,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 8,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 8192,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "ground_truth",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 1,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_1spp",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 1,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_1spp_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 6,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_6spp",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 6,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_6spp_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 12,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_12spp",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 12,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_12spp_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIAL - RECONNECTION
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_temporal = false,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_reconnection",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // SPATIAL - RECONNECTION
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_temporal = false,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_reconnection_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIOTEMPORAL - RECONNECTION
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_temporal = true,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_reconnection",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // SPATIOTEMPORAL - RECONNECTION
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_temporal = true,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_reconnection_denoised",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIAL - DELAYED RECONNECTION
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                        .m_temporal = false,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_delayed_reconnection",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // SPATIAL - DELAYED RECONNECTION
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                        .m_temporal = false,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_delayed_reconnection_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIOTEMPORAL - DELAYED RECONNECTION
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                        .m_temporal = true,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_delayed_reconnection",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // SPATIOTEMPORAL - DELAYED RECONNECTION
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                        .m_temporal = true,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_delayed_reconnection_denoised",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIAL - HYBRID
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_HYBRID,
                        .m_temporal = false,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_hybrid",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // SPATIAL - HYBRID
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_HYBRID,
                        .m_temporal = false,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_hybrid_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIOTEMPORAL - HYBRID
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_HYBRID,
                        .m_temporal = true,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_hybrid",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // SPATIOTEMPORAL - HYBRID
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_HYBRID,
                        .m_temporal = true,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_hybrid_denoised",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIAL - SEQUENTIAL
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_SEQUENTIAL,
                        .m_temporal = false,
                        .m_temporal_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial = true,
                        .m_spatial_count = 2,
                        .m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial2 = true,
                        .m_spatial2_count = 2,
                        .m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_sequential",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // SPATIAL - SEQUENTIAL
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_SEQUENTIAL,
                        .m_temporal = false,
                        .m_temporal_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial = true,
                        .m_spatial_count = 2,
                        .m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial2 = true,
                        .m_spatial2_count = 2,
                        .m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_sequential_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // SPATIOTEMPORAL - SEQUENTIAL
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_SEQUENTIAL,
                        .m_temporal = true,
                        .m_temporal_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial = true,
                        .m_spatial_count = 2,
                        .m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial2 = true,
                        .m_spatial2_count = 2,
                        .m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_sequential",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // SPATIOTEMPORAL - SEQUENTIAL
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_SEQUENTIAL,
                        .m_temporal = true,
                        .m_temporal_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial = true,
                        .m_spatial_count = 2,
                        .m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial2 = true,
                        .m_spatial2_count = 2,
                        .m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_sequential_denoised",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            if (m_png) {
                {
                    std::ofstream compareScript;
                    compareScript.open(subdirectory + "/compare_rmse.sh");
                    compareScript << "#!/bin/bash\n";
                    compareScript << "printf \"RMSE\\n\"\n";
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_1spp");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_6spp");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_12spp");
                    writeRMSECompare(compareScript, namePrefix, "spatial_reconnection");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    writeRMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    writeRMSECompare(compareScript, namePrefix, "spatial_hybrid");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid");
                    writeRMSECompare(compareScript, namePrefix, "spatial_sequential");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_sequential");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_1spp_denoised");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_6spp_denoised");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_12spp_denoised");
                    writeRMSECompare(compareScript, namePrefix, "spatial_reconnection_denoised");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection_denoised");
                    writeRMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection_denoised");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection_denoised");
                    writeRMSECompare(compareScript, namePrefix, "spatial_hybrid_denoised");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid_denoised");
                    writeRMSECompare(compareScript, namePrefix, "spatial_sequential_denoised");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_sequential_denoised");
                    compareScript.close();
                }
                {
                    std::ofstream compareScript;
                    compareScript.open(subdirectory + "/compare_mse.sh");
                    compareScript << "#!/bin/bash\n";
                    compareScript << "printf \"MSE\\n\"\n";
                    writeMSECompare(compareScript, namePrefix, "pathtracer_1spp");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_6spp");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_12spp");
                    writeMSECompare(compareScript, namePrefix, "spatial_reconnection");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    writeMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    writeMSECompare(compareScript, namePrefix, "spatial_hybrid");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid");
                    writeMSECompare(compareScript, namePrefix, "spatial_sequential");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_sequential");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_1spp_denoised");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_6spp_denoised");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_12spp_denoised");
                    writeMSECompare(compareScript, namePrefix, "spatial_reconnection_denoised");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection_denoised");
                    writeMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection_denoised");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection_denoised");
                    writeMSECompare(compareScript, namePrefix, "spatial_hybrid_denoised");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid_denoised");
                    writeMSECompare(compareScript, namePrefix, "spatial_sequential_denoised");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_sequential_denoised");
                    compareScript.close();
                }
                {
                    std::ofstream compareScript;
                    compareScript.open(subdirectory + "/compare_flip.sh");
                    compareScript << "#!/bin/bash\n";
                    compareScript << "printf \"FLIP\\n\"\n";
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_1spp");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_6spp");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_12spp");
                    writeFLIPCompare(compareScript, namePrefix, "spatial_reconnection");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    writeFLIPCompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    writeFLIPCompare(compareScript, namePrefix, "spatial_hybrid");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_hybrid");
                    writeFLIPCompare(compareScript, namePrefix, "spatial_sequential");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_sequential");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_1spp_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_6spp_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_12spp_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "spatial_reconnection_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_reconnection_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "spatial_delayed_reconnection_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "spatial_hybrid_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_hybrid_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "spatial_sequential_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_sequential_denoised");
                    compareScript.close();
                }
            } else {
                std::ofstream compareScript;
                compareScript.open(subdirectory + "/compare_hdr_rmse.sh");
                compareScript << "#!/bin/bash\n";
                compareScript << "printf \"HDR RMSE\\n\"\n";
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_1spp");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_6spp");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_12spp");
                writeHDRRMSECompare(compareScript, namePrefix, "spatial_reconnection");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                writeHDRRMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                writeHDRRMSECompare(compareScript, namePrefix, "spatial_hybrid");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid");
                writeHDRRMSECompare(compareScript, namePrefix, "spatial_sequential");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_sequential");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_1spp_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_6spp_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_12spp_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "spatial_reconnection_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "spatial_hybrid_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "spatial_sequential_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_sequential_denoised");
                compareScript.close();
            }
        }
        return true;
    }

    bool ReSTIRSSSEvaluation::evaluateAjaxIntersectionCount() const {
        constexpr glm::vec3 position = {AJAX_POSITION};
        constexpr glm::vec3 orientation = {AJAX_ORIENTATION};
        constexpr glm::vec3 trajectoryStart = {AJAX_TRAJECTORY_START};
        constexpr int32_t width = 720;
        constexpr int32_t height = 1080;

        std::string appName = "ReSTIR SSS";
        {
            const std::string subdirectory = m_directory + "/ajax_intersectioncount";
            if (!std::filesystem::create_directories(subdirectory)) {
                return false;
            }
            const std::string namePrefix = "ajax_";

            {
                // INTERSECTION COUNT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1024,
                        .m_renderer = RENDERER_DEBUG,
                        .m_debug = 13,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1024,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "intersectioncount",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
        }
        return true;
    }

    bool ReSTIRSSSEvaluation::evaluateAsianDragon(const uint32_t sceneMode) const {
        constexpr glm::vec3 position = {ASIAN_DRAGON_POSITION};
        constexpr glm::vec3 orientation = {ASIAN_DRAGON_ORIENTATION};
        constexpr glm::vec3 trajectoryStart = {ASIAN_DRAGON_TRAJECTORY_START};
        constexpr int32_t width = 1920;
        constexpr int32_t height = 1080;

        std::string appName = "ReSTIR SSS";
        {
            const std::string subdirectory = m_directory + "/asian_dragon_" + std::to_string(sceneMode);
            if (!std::filesystem::create_directories(subdirectory)) {
                return false;
            }
            const std::string namePrefix = "asian_dragon_";

            if (m_groundTruth) {
                // GROUND TRUTH
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::ASIAN_DRAGON,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 8192,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 8,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 8192,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "ground_truth",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::ASIAN_DRAGON,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 1,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_1spp",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::ASIAN_DRAGON,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 1,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_1spp_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::ASIAN_DRAGON,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 6,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_6spp",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::ASIAN_DRAGON,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 6,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_6spp_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::ASIAN_DRAGON,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 8,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_8spp",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::ASIAN_DRAGON,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 8,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_8spp_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::ASIAN_DRAGON,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 9,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_9spp",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::ASIAN_DRAGON,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 9,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_9spp_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::ASIAN_DRAGON,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 12,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_12spp",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::ASIAN_DRAGON,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 12,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_12spp_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            // {
            //     // SPATIAL - RECONNECTION
            //     auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
            //             .m_scene = Rayscenes::ASIAN_DRAGON,
            //             .m_sceneMode = sceneMode,
            //             .m_position = position,
            //             .m_orientation = orientation,
            //             .m_maxFrames = 1,
            //             .m_candidate_samples_light = 1,
            //             .m_candidate_samples_bsdf = 0,
            //             .m_candidate_sss_all_intersections = true,
            //             .m_shift = RESTIRSSS_SHIFT_RECONNECTION,
            //             .m_temporal = false,
            //             .m_spatial = true,
            //             .m_spatial_count = 4,
            //             .m_spatial2 = false};
            //     const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);
            //
            //     const auto settings = HeadlessApplication::ApplicationSettings{
            //             .m_msaaSamples = vk::SampleCountFlagBits::e1,
            //             .m_deviceExtensions = DEVICE_EXTENSIONS,
            //             .m_width = width,
            //             .m_height = height,
            //             .m_rendererExecutions = 1,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatial_reconnection"};
            //     HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
            //
            //     try {
            //         app.run();
            //     } catch (const std::exception &e) {
            //         std::cerr << e.what() << std::endl;
            //         return false;
            //     }
            // }
            //
            // {
            //     // SPATIOTEMPORAL - RECONNECTION
            //     auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
            //             .m_scene = Rayscenes::ASIAN_DRAGON,
            //             .m_sceneMode = sceneMode,
            //             .m_position = position,
            //             .m_orientation = orientation,
            //             .m_maxFrames = 1,
            //             .m_candidate_samples_light = 1,
            //             .m_candidate_samples_bsdf = 0,
            //             .m_candidate_sss_all_intersections = true,
            //             .m_shift = RESTIRSSS_SHIFT_RECONNECTION,
            //             .m_temporal = true,
            //             .m_spatial = true,
            //             .m_spatial_count = 4,
            //             .m_spatial2 = false};
            //     const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);
            //
            //     const auto settings = HeadlessApplication::ApplicationSettings{
            //             .m_msaaSamples = vk::SampleCountFlagBits::e1,
            //             .m_deviceExtensions = DEVICE_EXTENSIONS,
            //             .m_width = width,
            //             .m_height = height,
            //             .m_rendererExecutions = 16,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatiotemporal_reconnection",
            //             .m_executeAfterInit = [&](GPUContext *gpuContext) {
            //                 constexpr glm::vec3 start = trajectoryStart;
            //                 constexpr glm::vec3 end = position;
            //                 gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
            //                 gpuContext->getCamera()->initTrajectory(start, end, 16);
            //             }};
            //     HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
            //
            //     try {
            //         app.run();
            //     } catch (const std::exception &e) {
            //         std::cerr << e.what() << std::endl;
            //         return false;
            //     }
            // }
            //
            // {
            //     // SPATIAL - DELAYED RECONNECTION
            //     auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
            //             .m_scene = Rayscenes::ASIAN_DRAGON,
            //             .m_sceneMode = sceneMode,
            //             .m_position = position,
            //             .m_orientation = orientation,
            //             .m_maxFrames = 1,
            //             .m_candidate_samples_light = 1,
            //             .m_candidate_samples_bsdf = 0,
            //             .m_candidate_sss_all_intersections = true,
            //             .m_shift = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
            //             .m_temporal = false,
            //             .m_spatial = true,
            //             .m_spatial_count = 4,
            //             .m_spatial2 = false};
            //     const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);
            //
            //     const auto settings = HeadlessApplication::ApplicationSettings{
            //             .m_msaaSamples = vk::SampleCountFlagBits::e1,
            //             .m_deviceExtensions = DEVICE_EXTENSIONS,
            //             .m_width = width,
            //             .m_height = height,
            //             .m_rendererExecutions = 1,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatial_delayed_reconnection"};
            //     HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
            //
            //     try {
            //         app.run();
            //     } catch (const std::exception &e) {
            //         std::cerr << e.what() << std::endl;
            //         return false;
            //     }
            // }
            //
            // {
            //     // SPATIOTEMPORAL - DELAYED RECONNECTION
            //     auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
            //             .m_scene = Rayscenes::ASIAN_DRAGON,
            //             .m_sceneMode = sceneMode,
            //             .m_position = position,
            //             .m_orientation = orientation,
            //             .m_maxFrames = 1,
            //             .m_candidate_samples_light = 1,
            //             .m_candidate_samples_bsdf = 0,
            //             .m_candidate_sss_all_intersections = true,
            //             .m_shift = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
            //             .m_temporal = true,
            //             .m_spatial = true,
            //             .m_spatial_count = 4,
            //             .m_spatial2 = false};
            //     const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);
            //
            //     const auto settings = HeadlessApplication::ApplicationSettings{
            //             .m_msaaSamples = vk::SampleCountFlagBits::e1,
            //             .m_deviceExtensions = DEVICE_EXTENSIONS,
            //             .m_width = width,
            //             .m_height = height,
            //             .m_rendererExecutions = 16,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatiotemporal_delayed_reconnection",
            //             .m_executeAfterInit = [&](GPUContext *gpuContext) {
            //                 constexpr glm::vec3 start = trajectoryStart;
            //                 constexpr glm::vec3 end = position;
            //                 gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
            //                 gpuContext->getCamera()->initTrajectory(start, end, 16);
            //             }};
            //     HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
            //
            //     try {
            //         app.run();
            //     } catch (const std::exception &e) {
            //         std::cerr << e.what() << std::endl;
            //         return false;
            //     }
            // }
            //
            // {
            //     // SPATIAL - HYBRID
            //     auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
            //             .m_scene = Rayscenes::ASIAN_DRAGON,
            //             .m_sceneMode = sceneMode,
            //             .m_position = position,
            //             .m_orientation = orientation,
            //             .m_maxFrames = 1,
            //             .m_candidate_samples_light = 1,
            //             .m_candidate_samples_bsdf = 0,
            //             .m_candidate_sss_all_intersections = true,
            //             .m_shift = RESTIRSSS_SHIFT_HYBRID,
            //             .m_temporal = false,
            //             .m_spatial = true,
            //             .m_spatial_count = 4,
            //             .m_spatial2 = false};
            //     const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);
            //
            //     const auto settings = HeadlessApplication::ApplicationSettings{
            //             .m_msaaSamples = vk::SampleCountFlagBits::e1,
            //             .m_deviceExtensions = DEVICE_EXTENSIONS,
            //             .m_width = width,
            //             .m_height = height,
            //             .m_rendererExecutions = 1,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatial_hybrid"};
            //     HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
            //
            //     try {
            //         app.run();
            //     } catch (const std::exception &e) {
            //         std::cerr << e.what() << std::endl;
            //         return false;
            //     }
            // }

            {
                // SPATIOTEMPORAL - HYBRID
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::ASIAN_DRAGON,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_HYBRID,
                        .m_temporal = true,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_hybrid",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // SPATIOTEMPORAL - HYBRID
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::ASIAN_DRAGON,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_HYBRID,
                        .m_temporal = true,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_hybrid_denoised",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            // {
            //     // SPATIAL - SEQUENTIAL
            //     auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
            //             .m_scene = Rayscenes::ASIAN_DRAGON,
            //             .m_sceneMode = sceneMode,
            //             .m_position = position,
            //             .m_orientation = orientation,
            //             .m_maxFrames = 1,
            //             .m_candidate_samples_light = 1,
            //             .m_candidate_samples_bsdf = 0,
            //             .m_candidate_sss_all_intersections = true,
            //             .m_shift = RESTIRSSS_SHIFT_SEQUENTIAL,
            //             .m_temporal = false,
            //             .m_temporal_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
            //             .m_spatial = true,
            //             .m_spatial_count = 2,
            //             .m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
            //             .m_spatial2 = true,
            //             .m_spatial2_count = 2,
            //             .m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
            //     };
            //     const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);
            //
            //     const auto settings = HeadlessApplication::ApplicationSettings{
            //             .m_msaaSamples = vk::SampleCountFlagBits::e1,
            //             .m_deviceExtensions = DEVICE_EXTENSIONS,
            //             .m_width = width,
            //             .m_height = height,
            //             .m_rendererExecutions = 1,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatial_sequential"};
            //     HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
            //
            //     try {
            //         app.run();
            //     } catch (const std::exception &e) {
            //         std::cerr << e.what() << std::endl;
            //         return false;
            //     }
            // }

            {
                // SPATIOTEMPORAL - SEQUENTIAL
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::ASIAN_DRAGON,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_SEQUENTIAL,
                        .m_temporal = true,
                        .m_temporal_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial = true,
                        .m_spatial_count = 2,
                        .m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial2 = true,
                        .m_spatial2_count = 2,
                        .m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_sequential",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // SPATIOTEMPORAL - SEQUENTIAL
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::ASIAN_DRAGON,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_SEQUENTIAL,
                        .m_temporal = true,
                        .m_temporal_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial = true,
                        .m_spatial_count = 2,
                        .m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial2 = true,
                        .m_spatial2_count = 2,
                        .m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_sequential_denoised",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            if (m_png) {
                {
                    std::ofstream compareScript;
                    compareScript.open(subdirectory + "/compare_rmse.sh");
                    compareScript << "#!/bin/bash\n";
                    compareScript << "printf \"RMSE\\n\"\n";
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_1spp");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_6spp");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_8spp");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_9spp");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_12spp");
                    // writeRMSECompare(compareScript, namePrefix, "spatial_reconnection");
                    // writeRMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    // writeRMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    // writeRMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    // writeRMSECompare(compareScript, namePrefix, "spatial_hybrid");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid");
                    // writeRMSECompare(compareScript, namePrefix, "spatial_sequential");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_sequential");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_1spp_denoised");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_6spp_denoised");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_8spp_denoised");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_9spp_denoised");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_12spp_denoised");
                    // writeRMSECompare(compareScript, namePrefix, "spatial_reconnection");
                    // writeRMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    // writeRMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    // writeRMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    // writeRMSECompare(compareScript, namePrefix, "spatial_hybrid");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid_denoised");
                    // writeRMSECompare(compareScript, namePrefix, "spatial_sequential");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_sequential_denoised");
                    compareScript.close();
                }
                {
                    std::ofstream compareScript;
                    compareScript.open(subdirectory + "/compare_mse.sh");
                    compareScript << "#!/bin/bash\n";
                    compareScript << "printf \"MSE\\n\"\n";
                    writeMSECompare(compareScript, namePrefix, "pathtracer_1spp");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_6spp");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_8spp");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_9spp");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_12spp");
                    // writeMSECompare(compareScript, namePrefix, "spatial_reconnection");
                    // writeMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    // writeMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    // writeMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    // writeMSECompare(compareScript, namePrefix, "spatial_hybrid");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid");
                    // writeMSECompare(compareScript, namePrefix, "spatial_sequential");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_sequential");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_1spp_denoised");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_6spp_denoised");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_8spp_denoised");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_9spp_denoised");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_12spp_denoised");
                    // writeMSECompare(compareScript, namePrefix, "spatial_reconnection");
                    // writeMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    // writeMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    // writeMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    // writeMSECompare(compareScript, namePrefix, "spatial_hybrid");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid_denoised");
                    // writeMSECompare(compareScript, namePrefix, "spatial_sequential");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_sequential_denoised");
                    compareScript.close();
                }
                {
                    std::ofstream compareScript;
                    compareScript.open(subdirectory + "/compare_flip.sh");
                    compareScript << "#!/bin/bash\n";
                    compareScript << "printf \"FLIP\\n\"\n";
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_1spp");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_6spp");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_8spp");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_9spp");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_12spp");
                    // writeFLIPCompare(compareScript, namePrefix, "spatial_reconnection");
                    // writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    // writeFLIPCompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    // writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    // writeFLIPCompare(compareScript, namePrefix, "spatial_hybrid");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_hybrid");
                    // writeFLIPCompare(compareScript, namePrefix, "spatial_sequential");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_sequential");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_1spp_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_6spp_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_8spp_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_9spp_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_12spp_denoised");
                    // writeFLIPCompare(compareScript, namePrefix, "spatial_reconnection");
                    // writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    // writeFLIPCompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    // writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    // writeFLIPCompare(compareScript, namePrefix, "spatial_hybrid");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_hybrid_denoised");
                    // writeFLIPCompare(compareScript, namePrefix, "spatial_sequential");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_sequential_denoised");
                    compareScript.close();
                }
            } else {
                std::ofstream compareScript;
                compareScript.open(subdirectory + "/compare_hdr_rmse.sh");
                compareScript << "#!/bin/bash\n";
                compareScript << "printf \"HDR RMSE\\n\"\n";
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_1spp");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_6spp");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_8spp");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_9spp");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_12spp");
                // writeRMSECompare(compareScript, namePrefix, "spatial_reconnection");
                // writeRMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                // writeRMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                // writeRMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                // writeRMSECompare(compareScript, namePrefix, "spatial_hybrid");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid");
                // writeRMSECompare(compareScript, namePrefix, "spatial_sequential");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_sequential");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_1spp_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_6spp_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_8spp_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_9spp_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_12spp_denoised");
                // writeRMSECompare(compareScript, namePrefix, "spatial_reconnection");
                // writeRMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                // writeRMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                // writeRMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                // writeRMSECompare(compareScript, namePrefix, "spatial_hybrid");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid_denoised");
                // writeRMSECompare(compareScript, namePrefix, "spatial_sequential");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_sequential_denoised");
                compareScript.close();
            }
        }

        return true;
    }

    bool ReSTIRSSSEvaluation::evaluateLTEOrb(const uint32_t sceneMode) const {
        constexpr glm::vec3 position = {LTE_ORB_POSITION};
        constexpr glm::vec3 orientation = {LTE_ORB_ORIENTATION};
        constexpr glm::vec3 trajectoryStart = {LTE_ORB_TRAJECTORY_START};
        constexpr int32_t width = 1080;
        constexpr int32_t height = 1080;

        std::string appName = "ReSTIR SSS";
        {
            const std::string subdirectory = m_directory + "/lte_orb_" + std::to_string(sceneMode);
            if (!std::filesystem::create_directories(subdirectory)) {
                return false;
            }
            const std::string namePrefix = "lte_orb_";

            if (m_groundTruth) {
                // GROUND TRUTH
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::LTE_ORB,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 8192,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 8,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 8192,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "ground_truth",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::LTE_ORB,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 1,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_1spp",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::LTE_ORB,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 1,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_1spp_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::LTE_ORB,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 6,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_6spp",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::LTE_ORB,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 6,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_6spp_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::LTE_ORB,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 12,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_12spp",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::LTE_ORB,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 12,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 1,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_12spp_denoised",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            // {
            //     // SPATIAL - RECONNECTION
            //     auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
            //             .m_scene = Rayscenes::LTE_ORB,
            //             .m_sceneMode = sceneMode,
            //             .m_position = position,
            //             .m_orientation = orientation,
            //             .m_maxFrames = 1,
            //             .m_candidate_samples_light = 1,
            //             .m_candidate_samples_bsdf = 0,
            //             .m_candidate_sss_all_intersections = true,
            //             .m_shift = RESTIRSSS_SHIFT_RECONNECTION,
            //             .m_temporal = false,
            //             .m_spatial = true,
            //             .m_spatial_count = 4,
            //             .m_spatial2 = false};
            //     const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);
            //
            //     const auto settings = HeadlessApplication::ApplicationSettings{
            //             .m_msaaSamples = vk::SampleCountFlagBits::e1,
            //             .m_deviceExtensions = DEVICE_EXTENSIONS,
            //             .m_width = width,
            //             .m_height = height,
            //             .m_rendererExecutions = 1,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatial_reconnection"};
            //     HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
            //
            //     try {
            //         app.run();
            //     } catch (const std::exception &e) {
            //         std::cerr << e.what() << std::endl;
            //         return false;
            //     }
            // }
            //
            // {
            //     // SPATIOTEMPORAL - RECONNECTION
            //     auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
            //             .m_scene = Rayscenes::LTE_ORB,
            //             .m_sceneMode = sceneMode,
            //             .m_position = position,
            //             .m_orientation = orientation,
            //             .m_maxFrames = 1,
            //             .m_candidate_samples_light = 1,
            //             .m_candidate_samples_bsdf = 0,
            //             .m_candidate_sss_all_intersections = true,
            //             .m_shift = RESTIRSSS_SHIFT_RECONNECTION,
            //             .m_temporal = true,
            //             .m_spatial = true,
            //             .m_spatial_count = 4,
            //             .m_spatial2 = false};
            //     const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);
            //
            //     const auto settings = HeadlessApplication::ApplicationSettings{
            //             .m_msaaSamples = vk::SampleCountFlagBits::e1,
            //             .m_deviceExtensions = DEVICE_EXTENSIONS,
            //             .m_width = width,
            //             .m_height = height,
            //             .m_rendererExecutions = 16,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatiotemporal_reconnection",
            //             .m_executeAfterInit = [&](GPUContext *gpuContext) {
            //                 constexpr glm::vec3 start = trajectoryStart;
            //                 constexpr glm::vec3 end = position;
            //                 gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
            //                 gpuContext->getCamera()->initTrajectory(start, end, 16);
            //             }};
            //     HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
            //
            //     try {
            //         app.run();
            //     } catch (const std::exception &e) {
            //         std::cerr << e.what() << std::endl;
            //         return false;
            //     }
            // }
            //
            // {
            //     // SPATIAL - DELAYED RECONNECTION
            //     auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
            //             .m_scene = Rayscenes::LTE_ORB,
            //             .m_sceneMode = sceneMode,
            //             .m_position = position,
            //             .m_orientation = orientation,
            //             .m_maxFrames = 1,
            //             .m_candidate_samples_light = 1,
            //             .m_candidate_samples_bsdf = 0,
            //             .m_candidate_sss_all_intersections = true,
            //             .m_shift = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
            //             .m_temporal = false,
            //             .m_spatial = true,
            //             .m_spatial_count = 4,
            //             .m_spatial2 = false};
            //     const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);
            //
            //     const auto settings = HeadlessApplication::ApplicationSettings{
            //             .m_msaaSamples = vk::SampleCountFlagBits::e1,
            //             .m_deviceExtensions = DEVICE_EXTENSIONS,
            //             .m_width = width,
            //             .m_height = height,
            //             .m_rendererExecutions = 1,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatial_delayed_reconnection"};
            //     HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
            //
            //     try {
            //         app.run();
            //     } catch (const std::exception &e) {
            //         std::cerr << e.what() << std::endl;
            //         return false;
            //     }
            // }
            //
            // {
            //     // SPATIOTEMPORAL - DELAYED RECONNECTION
            //     auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
            //             .m_scene = Rayscenes::LTE_ORB,
            //             .m_sceneMode = sceneMode,
            //             .m_position = position,
            //             .m_orientation = orientation,
            //             .m_maxFrames = 1,
            //             .m_candidate_samples_light = 1,
            //             .m_candidate_samples_bsdf = 0,
            //             .m_candidate_sss_all_intersections = true,
            //             .m_shift = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
            //             .m_temporal = true,
            //             .m_spatial = true,
            //             .m_spatial_count = 4,
            //             .m_spatial2 = false};
            //     const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);
            //
            //     const auto settings = HeadlessApplication::ApplicationSettings{
            //             .m_msaaSamples = vk::SampleCountFlagBits::e1,
            //             .m_deviceExtensions = DEVICE_EXTENSIONS,
            //             .m_width = width,
            //             .m_height = height,
            //             .m_rendererExecutions = 16,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatiotemporal_delayed_reconnection",
            //             .m_executeAfterInit = [&](GPUContext *gpuContext) {
            //                 constexpr glm::vec3 start = trajectoryStart;
            //                 constexpr glm::vec3 end = position;
            //                 gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
            //                 gpuContext->getCamera()->initTrajectory(start, end, 16);
            //             }};
            //     HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
            //
            //     try {
            //         app.run();
            //     } catch (const std::exception &e) {
            //         std::cerr << e.what() << std::endl;
            //         return false;
            //     }
            // }

            // {
            //     // SPATIAL - HYBRID
            //     auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
            //             .m_scene = Rayscenes::LTE_ORB,
            //             .m_sceneMode = sceneMode,
            //             .m_position = position,
            //             .m_orientation = orientation,
            //             .m_maxFrames = 1,
            //             .m_candidate_samples_light = 1,
            //             .m_candidate_samples_bsdf = 0,
            //             .m_candidate_sss_all_intersections = true,
            //             .m_shift = RESTIRSSS_SHIFT_HYBRID,
            //             .m_temporal = false,
            //             .m_spatial = true,
            //             .m_spatial_count = 4,
            //             .m_spatial2 = false};
            //     const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);
            //
            //     const auto settings = HeadlessApplication::ApplicationSettings{
            //             .m_msaaSamples = vk::SampleCountFlagBits::e1,
            //             .m_deviceExtensions = DEVICE_EXTENSIONS,
            //             .m_width = width,
            //             .m_height = height,
            //             .m_rendererExecutions = 1,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatial_hybrid"};
            //     HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
            //
            //     try {
            //         app.run();
            //     } catch (const std::exception &e) {
            //         std::cerr << e.what() << std::endl;
            //         return false;
            //     }
            // }

            {
                // SPATIOTEMPORAL - HYBRID
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::LTE_ORB,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_HYBRID,
                        .m_temporal = true,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_hybrid",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // SPATIOTEMPORAL - HYBRID
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::LTE_ORB,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_HYBRID,
                        .m_temporal = true,
                        .m_spatial = true,
                        .m_spatial_count = 4,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_hybrid_denoised",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            // {
            //     // SPATIAL - SEQUENTIAL
            //     auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
            //             .m_scene = Rayscenes::LTE_ORB,
            //             .m_sceneMode = sceneMode,
            //             .m_position = position,
            //             .m_orientation = orientation,
            //             .m_maxFrames = 1,
            //             .m_candidate_samples_light = 1,
            //             .m_candidate_samples_bsdf = 0,
            //             .m_candidate_sss_all_intersections = true,
            //             .m_shift = RESTIRSSS_SHIFT_SEQUENTIAL,
            //             .m_temporal = false,
            //             .m_temporal_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
            //             .m_spatial = true,
            //             .m_spatial_count = 2,
            //             .m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
            //             .m_spatial2 = true,
            //             .m_spatial2_count = 2,
            //             .m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
            //     };
            //     const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);
            //
            //     const auto settings = HeadlessApplication::ApplicationSettings{
            //             .m_msaaSamples = vk::SampleCountFlagBits::e1,
            //             .m_deviceExtensions = DEVICE_EXTENSIONS,
            //             .m_width = width,
            //             .m_height = height,
            //             .m_rendererExecutions = 1,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatial_sequential"};
            //     HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
            //
            //     try {
            //         app.run();
            //     } catch (const std::exception &e) {
            //         std::cerr << e.what() << std::endl;
            //         return false;
            //     }
            // }

            {
                // SPATIOTEMPORAL - SEQUENTIAL
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::LTE_ORB,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_SEQUENTIAL,
                        .m_temporal = true,
                        .m_temporal_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial = true,
                        .m_spatial_count = 2,
                        .m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial2 = true,
                        .m_spatial2_count = 2,
                        .m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_sequential",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            {
                // SPATIOTEMPORAL - SEQUENTIAL
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::LTE_ORB,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_shift = RESTIRSSS_SHIFT_SEQUENTIAL,
                        .m_temporal = true,
                        .m_temporal_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial = true,
                        .m_spatial_count = 2,
                        .m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                        .m_spatial2 = true,
                        .m_spatial2_count = 2,
                        .m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_sequential_denoised",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        },
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            if (m_png) {
                {
                    std::ofstream compareScript;
                    compareScript.open(subdirectory + "/compare_rmse.sh");
                    compareScript << "#!/bin/bash\n";
                    compareScript << "printf \"RMSE\\n\"\n";
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_1spp");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_6spp");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_12spp");
                    // writeRMSECompare(compareScript, namePrefix, "spatial_reconnection");
                    // writeRMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    // writeRMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    // writeRMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    // writeRMSECompare(compareScript, namePrefix, "spatial_hybrid");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid");
                    // writeRMSECompare(compareScript, namePrefix, "spatial_sequential");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_sequential");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_1spp_denoised");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_6spp_denoised");
                    writeRMSECompare(compareScript, namePrefix, "pathtracer_12spp_denoised");
                    // writeRMSECompare(compareScript, namePrefix, "spatial_reconnection");
                    // writeRMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    // writeRMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    // writeRMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    // writeRMSECompare(compareScript, namePrefix, "spatial_hybrid");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid_denoised");
                    // writeRMSECompare(compareScript, namePrefix, "spatial_sequential");
                    writeRMSECompare(compareScript, namePrefix, "spatiotemporal_sequential_denoised");
                    compareScript.close();
                }
                {
                    std::ofstream compareScript;
                    compareScript.open(subdirectory + "/compare_mse.sh");
                    compareScript << "#!/bin/bash\n";
                    compareScript << "printf \"MSE\\n\"\n";
                    writeMSECompare(compareScript, namePrefix, "pathtracer_1spp");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_6spp");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_12spp");
                    // writeMSECompare(compareScript, namePrefix, "spatial_reconnection");
                    // writeMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    // writeMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    // writeMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    // writeMSECompare(compareScript, namePrefix, "spatial_hybrid");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid");
                    // writeMSECompare(compareScript, namePrefix, "spatial_sequential");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_sequential");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_1spp_denoised");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_6spp_denoised");
                    writeMSECompare(compareScript, namePrefix, "pathtracer_12spp_denoised");
                    // writeMSECompare(compareScript, namePrefix, "spatial_reconnection");
                    // writeMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    // writeMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    // writeMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    // writeMSECompare(compareScript, namePrefix, "spatial_hybrid");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid_denoised");
                    // writeMSECompare(compareScript, namePrefix, "spatial_sequential");
                    writeMSECompare(compareScript, namePrefix, "spatiotemporal_sequential_denoised");
                    compareScript.close();
                }
                {
                    std::ofstream compareScript;
                    compareScript.open(subdirectory + "/compare_flip.sh");
                    compareScript << "#!/bin/bash\n";
                    compareScript << "printf \"FLIP\\n\"\n";
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_1spp");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_6spp");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_12spp");
                    // writeFLIPCompare(compareScript, namePrefix, "spatial_reconnection");
                    // writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    // writeFLIPCompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    // writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    // writeFLIPCompare(compareScript, namePrefix, "spatial_hybrid");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_hybrid");
                    // writeFLIPCompare(compareScript, namePrefix, "spatial_sequential");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_sequential");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_1spp_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_6spp_denoised");
                    writeFLIPCompare(compareScript, namePrefix, "pathtracer_12spp_denoised");
                    // writeFLIPCompare(compareScript, namePrefix, "spatial_reconnection");
                    // writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                    // writeFLIPCompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                    // writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                    // writeFLIPCompare(compareScript, namePrefix, "spatial_hybrid");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_hybrid_denoised");
                    // writeFLIPCompare(compareScript, namePrefix, "spatial_sequential");
                    writeFLIPCompare(compareScript, namePrefix, "spatiotemporal_sequential_denoised");
                    compareScript.close();
                }
            } else {
                std::ofstream compareScript;
                compareScript.open(subdirectory + "/compare_hdr_rmse.sh");
                compareScript << "#!/bin/bash\n";
                compareScript << "printf \"HDR RMSE\\n\"\n";
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_1spp");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_6spp");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_12spp");
                // writeRMSECompare(compareScript, namePrefix, "spatial_reconnection");
                // writeRMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                // writeRMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                // writeRMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                // writeRMSECompare(compareScript, namePrefix, "spatial_hybrid");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid");
                // writeRMSECompare(compareScript, namePrefix, "spatial_sequential");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_sequential");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_1spp_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_6spp_denoised");
                writeHDRRMSECompare(compareScript, namePrefix, "pathtracer_12spp_denoised");
                // writeRMSECompare(compareScript, namePrefix, "spatial_reconnection");
                // writeRMSECompare(compareScript, namePrefix, "spatiotemporal_reconnection");
                // writeRMSECompare(compareScript, namePrefix, "spatial_delayed_reconnection");
                // writeRMSECompare(compareScript, namePrefix, "spatiotemporal_delayed_reconnection");
                // writeRMSECompare(compareScript, namePrefix, "spatial_hybrid");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_hybrid_denoised");
                // writeRMSECompare(compareScript, namePrefix, "spatial_sequential");
                writeHDRRMSECompare(compareScript, namePrefix, "spatiotemporal_sequential_denoised");
                compareScript.close();
            }
        }

        return true;
    }

    bool ReSTIRSSSEvaluation::evaluateSpatioTemporalConvergenceAjax() const {
        constexpr glm::vec3 position = {AJAX_POSITION};
        constexpr glm::vec3 orientation = {AJAX_ORIENTATION};
        constexpr glm::vec3 trajectoryStart = {AJAX_TRAJECTORY_START};
        constexpr int32_t width = 720;
        constexpr int32_t height = 1080;

        std::string appName = "ReSTIR SSS";
        {
            const std::string subdirectory = m_directory + "/ajax";
            if (!std::filesystem::create_directories(subdirectory)) {
                return false;
            }
            const std::string namePrefix = "ajax_";

            if (m_groundTruth) {
                // GROUND TRUTH
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 8192,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 8,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 8192,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "ground_truth",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            std::vector<uint32_t> spps = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};

            for (const auto &spp: spps) {
                {
                    // SPATIOTEMPORAL - RECONNECTION
                    auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                            .m_scene = Rayscenes::AJAX,
                            .m_position = position,
                            .m_orientation = orientation,
                            .m_maxFrames = spp,
                            .m_tonemapper = m_png ? 1 : 0,
                            .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                            .m_candidate_samples_light = 1,
                            .m_candidate_samples_bsdf = 0,
                            .m_candidate_sss_all_intersections = true,
                            .m_shift = RESTIRSSS_SHIFT_RECONNECTION,
                            .m_temporal = true,
                            .m_spatial = true,
                            .m_spatial_count = 4,
                            .m_spatial2 = false};
                    const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                    const auto settings = HeadlessApplication::ApplicationSettings{
                            .m_msaaSamples = vk::SampleCountFlagBits::e1,
                            .m_deviceExtensions = DEVICE_EXTENSIONS,
                            .m_width = width,
                            .m_height = height,
                            .m_rendererExecutions = spp + 16,
                            .m_directory = subdirectory,
                            .m_name = namePrefix + "spatiotemporal_reconnection_" + std::to_string(spp),
                            .m_executeAfterInit = [&](GPUContext *gpuContext) {
                                constexpr glm::vec3 start = trajectoryStart;
                                constexpr glm::vec3 end = position;
                                gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                                gpuContext->getCamera()->initTrajectory(start, end, 16);
                            },
                            .m_png = m_png};
                    HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                    try {
                        app.run();
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << std::endl;
                        return false;
                    }
                }

                {
                    // SPATIOTEMPORAL - DELAYED RECONNECTION
                    auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                            .m_scene = Rayscenes::AJAX,
                            .m_position = position,
                            .m_orientation = orientation,
                            .m_maxFrames = spp,
                            .m_tonemapper = m_png ? 1 : 0,
                            .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                            .m_candidate_samples_light = 1,
                            .m_candidate_samples_bsdf = 0,
                            .m_candidate_sss_all_intersections = true,
                            .m_shift = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                            .m_temporal = true,
                            .m_spatial = true,
                            .m_spatial_count = 4,
                            .m_spatial2 = false};
                    const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                    const auto settings = HeadlessApplication::ApplicationSettings{
                            .m_msaaSamples = vk::SampleCountFlagBits::e1,
                            .m_deviceExtensions = DEVICE_EXTENSIONS,
                            .m_width = width,
                            .m_height = height,
                            .m_rendererExecutions = spp + 16,
                            .m_directory = subdirectory,
                            .m_name = namePrefix + "spatiotemporal_delayed_reconnection_" + std::to_string(spp),
                            .m_executeAfterInit = [&](GPUContext *gpuContext) {
                                constexpr glm::vec3 start = trajectoryStart;
                                constexpr glm::vec3 end = position;
                                gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                                gpuContext->getCamera()->initTrajectory(start, end, 16);
                            },
                            .m_png = m_png};
                    HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                    try {
                        app.run();
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << std::endl;
                        return false;
                    }
                }

                {
                    // SPATIOTEMPORAL - HYBRID
                    auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                            .m_scene = Rayscenes::AJAX,
                            .m_position = position,
                            .m_orientation = orientation,
                            .m_maxFrames = spp,
                            .m_tonemapper = m_png ? 1 : 0,
                            .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                            .m_candidate_samples_light = 1,
                            .m_candidate_samples_bsdf = 0,
                            .m_candidate_sss_all_intersections = true,
                            .m_shift = RESTIRSSS_SHIFT_HYBRID,
                            .m_temporal = true,
                            .m_spatial = true,
                            .m_spatial_count = 4,
                            .m_spatial2 = false};
                    const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                    const auto settings = HeadlessApplication::ApplicationSettings{
                            .m_msaaSamples = vk::SampleCountFlagBits::e1,
                            .m_deviceExtensions = DEVICE_EXTENSIONS,
                            .m_width = width,
                            .m_height = height,
                            .m_rendererExecutions = spp + 16,
                            .m_directory = subdirectory,
                            .m_name = namePrefix + "spatiotemporal_hybrid_" + std::to_string(spp),
                            .m_executeAfterInit = [&](GPUContext *gpuContext) {
                                constexpr glm::vec3 start = trajectoryStart;
                                constexpr glm::vec3 end = position;
                                gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                                gpuContext->getCamera()->initTrajectory(start, end, 16);
                            },
                            .m_png = m_png};
                    HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                    try {
                        app.run();
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << std::endl;
                        return false;
                    }
                }

                {
                    // SPATIOTEMPORAL - SEQUENTIAL
                    auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                            .m_scene = Rayscenes::AJAX,
                            .m_position = position,
                            .m_orientation = orientation,
                            .m_maxFrames = spp,
                            .m_tonemapper = m_png ? 1 : 0,
                            .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                            .m_candidate_samples_light = 1,
                            .m_candidate_samples_bsdf = 0,
                            .m_candidate_sss_all_intersections = true,
                            .m_shift = RESTIRSSS_SHIFT_SEQUENTIAL,
                            .m_temporal = true,
                            .m_temporal_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                            .m_spatial = true,
                            .m_spatial_count = 2,
                            .m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                            .m_spatial2 = true,
                            .m_spatial2_count = 2,
                            .m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                    };
                    const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                    const auto settings = HeadlessApplication::ApplicationSettings{
                            .m_msaaSamples = vk::SampleCountFlagBits::e1,
                            .m_deviceExtensions = DEVICE_EXTENSIONS,
                            .m_width = width,
                            .m_height = height,
                            .m_rendererExecutions = spp + 16,
                            .m_directory = subdirectory,
                            .m_name = namePrefix + "spatiotemporal_sequential_" + std::to_string(spp),
                            .m_executeAfterInit = [&](GPUContext *gpuContext) {
                                constexpr glm::vec3 start = trajectoryStart;
                                constexpr glm::vec3 end = position;
                                gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                                gpuContext->getCamera()->initTrajectory(start, end, 16);
                            },
                            .m_png = m_png};
                    HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                    try {
                        app.run();
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << std::endl;
                        return false;
                    }
                }
            }
        }

        return true;
    }

    bool ReSTIRSSSEvaluation::evaluateSpatioTemporalConvergenceLTEOrb(uint32_t sceneMode) const {
        constexpr glm::vec3 position = {LTE_ORB_POSITION};
        constexpr glm::vec3 orientation = {LTE_ORB_ORIENTATION};
        constexpr glm::vec3 trajectoryStart = {LTE_ORB_TRAJECTORY_START};
        constexpr int32_t width = 1080;
        constexpr int32_t height = 1080;

        std::string appName = "ReSTIR SSS";
        {
            const std::string subdirectory = m_directory + "/lte_orb_" + std::to_string(sceneMode);
            if (!std::filesystem::create_directories(subdirectory)) {
                return false;
            }
            const std::string namePrefix = "lte_orb_";

            if (m_groundTruth) {
                // GROUND TRUTH
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::LTE_ORB,
                        .m_sceneMode = sceneMode,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 8192,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_tonemapper = m_png ? 1 : 0,
                        .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                        .m_spp = 8,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 8192,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "ground_truth",
                        .m_png = m_png};
                HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                try {
                    app.run();
                } catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            std::vector<uint32_t> spps = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};

            for (const auto &spp: spps) {
                {
                    // SPATIOTEMPORAL - RECONNECTION
                    auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                            .m_scene = Rayscenes::LTE_ORB,
                            .m_sceneMode = sceneMode,
                            .m_position = position,
                            .m_orientation = orientation,
                            .m_maxFrames = spp,
                            .m_tonemapper = m_png ? 1 : 0,
                            .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                            .m_candidate_samples_light = 1,
                            .m_candidate_samples_bsdf = 0,
                            .m_candidate_sss_all_intersections = true,
                            .m_shift = RESTIRSSS_SHIFT_RECONNECTION,
                            .m_temporal = true,
                            .m_spatial = true,
                            .m_spatial_count = 4,
                            .m_spatial2 = false};
                    const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                    const auto settings = HeadlessApplication::ApplicationSettings{
                            .m_msaaSamples = vk::SampleCountFlagBits::e1,
                            .m_deviceExtensions = DEVICE_EXTENSIONS,
                            .m_width = width,
                            .m_height = height,
                            .m_rendererExecutions = spp + 16,
                            .m_directory = subdirectory,
                            .m_name = namePrefix + "spatiotemporal_reconnection_" + std::to_string(spp),
                            .m_executeAfterInit = [&](GPUContext *gpuContext) {
                                constexpr glm::vec3 start = trajectoryStart;
                                constexpr glm::vec3 end = position;
                                gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                                gpuContext->getCamera()->initTrajectory(start, end, 16);
                            },
                            .m_png = m_png};
                    HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                    try {
                        app.run();
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << std::endl;
                        return false;
                    }
                }

                {
                    // SPATIOTEMPORAL - DELAYED RECONNECTION
                    auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                            .m_scene = Rayscenes::LTE_ORB,
                            .m_sceneMode = sceneMode,
                            .m_position = position,
                            .m_orientation = orientation,
                            .m_maxFrames = spp,
                            .m_tonemapper = m_png ? 1 : 0,
                            .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                            .m_candidate_samples_light = 1,
                            .m_candidate_samples_bsdf = 0,
                            .m_candidate_sss_all_intersections = true,
                            .m_shift = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                            .m_temporal = true,
                            .m_spatial = true,
                            .m_spatial_count = 4,
                            .m_spatial2 = false};
                    const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                    const auto settings = HeadlessApplication::ApplicationSettings{
                            .m_msaaSamples = vk::SampleCountFlagBits::e1,
                            .m_deviceExtensions = DEVICE_EXTENSIONS,
                            .m_width = width,
                            .m_height = height,
                            .m_rendererExecutions = spp + 16,
                            .m_directory = subdirectory,
                            .m_name = namePrefix + "spatiotemporal_delayed_reconnection_" + std::to_string(spp),
                            .m_executeAfterInit = [&](GPUContext *gpuContext) {
                                constexpr glm::vec3 start = trajectoryStart;
                                constexpr glm::vec3 end = position;
                                gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                                gpuContext->getCamera()->initTrajectory(start, end, 16);
                            },
                            .m_png = m_png};
                    HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                    try {
                        app.run();
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << std::endl;
                        return false;
                    }
                }

                {
                    // SPATIOTEMPORAL - HYBRID
                    auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                            .m_scene = Rayscenes::LTE_ORB,
                            .m_sceneMode = sceneMode,
                            .m_position = position,
                            .m_orientation = orientation,
                            .m_maxFrames = spp,
                            .m_tonemapper = m_png ? 1 : 0,
                            .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                            .m_candidate_samples_light = 1,
                            .m_candidate_samples_bsdf = 0,
                            .m_candidate_sss_all_intersections = true,
                            .m_shift = RESTIRSSS_SHIFT_HYBRID,
                            .m_temporal = true,
                            .m_spatial = true,
                            .m_spatial_count = 4,
                            .m_spatial2 = false};
                    const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                    const auto settings = HeadlessApplication::ApplicationSettings{
                            .m_msaaSamples = vk::SampleCountFlagBits::e1,
                            .m_deviceExtensions = DEVICE_EXTENSIONS,
                            .m_width = width,
                            .m_height = height,
                            .m_rendererExecutions = spp + 16,
                            .m_directory = subdirectory,
                            .m_name = namePrefix + "spatiotemporal_hybrid_" + std::to_string(spp),
                            .m_executeAfterInit = [&](GPUContext *gpuContext) {
                                constexpr glm::vec3 start = trajectoryStart;
                                constexpr glm::vec3 end = position;
                                gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                                gpuContext->getCamera()->initTrajectory(start, end, 16);
                            },
                            .m_png = m_png};
                    HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                    try {
                        app.run();
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << std::endl;
                        return false;
                    }
                }

                {
                    // SPATIOTEMPORAL - SEQUENTIAL
                    auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                            .m_scene = Rayscenes::LTE_ORB,
                            .m_sceneMode = sceneMode,
                            .m_position = position,
                            .m_orientation = orientation,
                            .m_maxFrames = spp,
                            .m_tonemapper = m_png ? 1 : 0,
                            .m_nonSSObjects = m_nonSSSObject ? 1 : 0,
                            .m_candidate_samples_light = 1,
                            .m_candidate_samples_bsdf = 0,
                            .m_candidate_sss_all_intersections = true,
                            .m_shift = RESTIRSSS_SHIFT_SEQUENTIAL,
                            .m_temporal = true,
                            .m_temporal_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                            .m_spatial = true,
                            .m_spatial_count = 2,
                            .m_spatial_shift_sequential = RESTIRSSS_SHIFT_RECONNECTION,
                            .m_spatial2 = true,
                            .m_spatial2_count = 2,
                            .m_spatial2_shift_sequential = RESTIRSSS_SHIFT_DELAYED_RECONNECTION,
                    };
                    const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                    const auto settings = HeadlessApplication::ApplicationSettings{
                            .m_msaaSamples = vk::SampleCountFlagBits::e1,
                            .m_deviceExtensions = DEVICE_EXTENSIONS,
                            .m_width = width,
                            .m_height = height,
                            .m_rendererExecutions = spp + 16,
                            .m_directory = subdirectory,
                            .m_name = namePrefix + "spatiotemporal_sequential_" + std::to_string(spp),
                            .m_executeAfterInit = [&](GPUContext *gpuContext) {
                                constexpr glm::vec3 start = trajectoryStart;
                                constexpr glm::vec3 end = position;
                                gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                                gpuContext->getCamera()->initTrajectory(start, end, 16);
                            },
                            .m_png = m_png};
                    HeadlessApplication app(appName, settings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

                    try {
                        app.run();
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << std::endl;
                        return false;
                    }
                }
            }
        }

        return true;
    }
} // namespace raven