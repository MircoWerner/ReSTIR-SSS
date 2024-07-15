#include "restirsss/ReSTIRSSSVideo.h"

#include "raven/core/VideoHeadlessApplication.h"

namespace raven {
    bool ReSTIRSSSVideo::videoAjaxReference() const {
        constexpr glm::vec3 position = {AJAX_POSITION};
        constexpr glm::vec3 orientation = {AJAX_ORIENTATION};
        constexpr int32_t width = 1920;
        constexpr int32_t height = 1080;

        std::string appName = "ReSTIR SSS";
        {
            const std::string subdirectory = m_directory + "/ajax_reference";
            if (!std::filesystem::create_directories(subdirectory)) {
                return false;
            }

            {
                float y = 8.049f;
                float dist = 55.f;
                BSpline cameraBSplinePosition;
                cameraBSplinePosition.m_controlPoints = {{0, y, dist}, {dist, y, 0}, {0, y, -dist}, {-dist, y, 0}, {0, y, dist}, {dist, y, 0}, {0, y, -dist}};
                BSpline cameraBSplineOrientation;
                cameraBSplineOrientation.m_controlPoints = {{0, 0.f * glm::pi<float>(), 1.818f}, {0, 0.5f * glm::pi<float>(), 1.818f}, {0, 1.0f * glm::pi<float>(), 1.818f}, {0, 1.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 1.0f * glm::pi<float>(), 1.818f}};
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1024,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_spp = 1,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false,
                        .m_cameraBSplinePosition = cameraBSplinePosition,
                        .m_cameraBSplineOrientation = cameraBSplineOrientation,
                        .m_cameraBSplineU = 0,
                        .m_cameraBSplineSpeed = 0.5f,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 60 * 8,
                        .m_directory = subdirectory,
                        .m_name = ""};
                const auto videoSettings = VideoHeadlessApplication::VideoApplicationSettings{
                        .m_framePerExecution = 1024};
                VideoHeadlessApplication app(appName, settings, videoSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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

    bool ReSTIRSSSVideo::videoAjaxPT1SPP() const {
        constexpr glm::vec3 position = {AJAX_POSITION};
        constexpr glm::vec3 orientation = {AJAX_ORIENTATION};
        constexpr int32_t width = 1920;
        constexpr int32_t height = 1080;

        std::string appName = "ReSTIR SSS";
        {
            const std::string subdirectory = m_directory + "/ajax_pt1spp";
            if (!std::filesystem::create_directories(subdirectory)) {
                return false;
            }

            {
                float y = 8.049f;
                float dist = 55.f;
                BSpline cameraBSplinePosition;
                cameraBSplinePosition.m_controlPoints = {{0, y, dist}, {dist, y, 0}, {0, y, -dist}, {-dist, y, 0}, {0, y, dist}, {dist, y, 0}, {0, y, -dist}};
                BSpline cameraBSplineOrientation;
                cameraBSplineOrientation.m_controlPoints = {{0, 0.f * glm::pi<float>(), 1.818f}, {0, 0.5f * glm::pi<float>(), 1.818f}, {0, 1.0f * glm::pi<float>(), 1.818f}, {0, 1.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 1.0f * glm::pi<float>(), 1.818f}};
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_spp = 1,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false,
                        .m_cameraBSplinePosition = cameraBSplinePosition,
                        .m_cameraBSplineOrientation = cameraBSplineOrientation,
                        .m_cameraBSplineU = 0,
                        .m_cameraBSplineSpeed = 0.5f,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 60 * 8,
                        .m_directory = subdirectory,
                        .m_name = ""};
                const auto videoSettings = VideoHeadlessApplication::VideoApplicationSettings{
                        .m_framePerExecution = 1};
                VideoHeadlessApplication app(appName, settings, videoSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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

    bool ReSTIRSSSVideo::videoAjaxPT1SPPDenoised() const {
        constexpr glm::vec3 position = {AJAX_POSITION};
        constexpr glm::vec3 orientation = {AJAX_ORIENTATION};
        constexpr int32_t width = 1920;
        constexpr int32_t height = 1080;

        std::string appName = "ReSTIR SSS";
        {
            const std::string subdirectory = m_directory + "/ajax_pt1sppdenoised";
            if (!std::filesystem::create_directories(subdirectory)) {
                return false;
            }

            {
                float y = 8.049f;
                float dist = 55.f;
                BSpline cameraBSplinePosition;
                cameraBSplinePosition.m_controlPoints = {{0, y, dist}, {dist, y, 0}, {0, y, -dist}, {-dist, y, 0}, {0, y, dist}, {dist, y, 0}, {0, y, -dist}};
                BSpline cameraBSplineOrientation;
                cameraBSplineOrientation.m_controlPoints = {{0, 0.f * glm::pi<float>(), 1.818f}, {0, 0.5f * glm::pi<float>(), 1.818f}, {0, 1.0f * glm::pi<float>(), 1.818f}, {0, 1.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 1.0f * glm::pi<float>(), 1.818f}};
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_spp = 1,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false,
                        .m_cameraBSplinePosition = cameraBSplinePosition,
                        .m_cameraBSplineOrientation = cameraBSplineOrientation,
                        .m_cameraBSplineU = 0,
                        .m_cameraBSplineSpeed = 0.5f,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 60 * 8,
                        .m_directory = subdirectory,
                        .m_name = ""};
                const auto videoSettings = VideoHeadlessApplication::VideoApplicationSettings{
                        .m_framePerExecution = 1};
                VideoHeadlessApplication app(appName, settings, videoSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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

    bool ReSTIRSSSVideo::videoAjaxPT6SPP() const {
               constexpr glm::vec3 position = {AJAX_POSITION};
        constexpr glm::vec3 orientation = {AJAX_ORIENTATION};
        constexpr int32_t width = 1920;
        constexpr int32_t height = 1080;

        std::string appName = "ReSTIR SSS";
        {
            const std::string subdirectory = m_directory + "/ajax_pt6spp";
            if (!std::filesystem::create_directories(subdirectory)) {
                return false;
            }

            {
                float y = 8.049f;
                float dist = 55.f;
                BSpline cameraBSplinePosition;
                cameraBSplinePosition.m_controlPoints = {{0, y, dist}, {dist, y, 0}, {0, y, -dist}, {-dist, y, 0}, {0, y, dist}, {dist, y, 0}, {0, y, -dist}};
                BSpline cameraBSplineOrientation;
                cameraBSplineOrientation.m_controlPoints = {{0, 0.f * glm::pi<float>(), 1.818f}, {0, 0.5f * glm::pi<float>(), 1.818f}, {0, 1.0f * glm::pi<float>(), 1.818f}, {0, 1.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 1.0f * glm::pi<float>(), 1.818f}};
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_spp = 6,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false,
                        .m_cameraBSplinePosition = cameraBSplinePosition,
                        .m_cameraBSplineOrientation = cameraBSplineOrientation,
                        .m_cameraBSplineU = 0,
                        .m_cameraBSplineSpeed = 0.5f,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 60 * 8,
                        .m_directory = subdirectory,
                        .m_name = ""};
                const auto videoSettings = VideoHeadlessApplication::VideoApplicationSettings{
                        .m_framePerExecution = 1};
                VideoHeadlessApplication app(appName, settings, videoSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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

    bool ReSTIRSSSVideo::videoAjaxPT6SPPDenoised() const {
                constexpr glm::vec3 position = {AJAX_POSITION};
        constexpr glm::vec3 orientation = {AJAX_ORIENTATION};
        constexpr int32_t width = 1920;
        constexpr int32_t height = 1080;

        std::string appName = "ReSTIR SSS";
        {
            const std::string subdirectory = m_directory + "/ajax_pt6sppdenoised";
            if (!std::filesystem::create_directories(subdirectory)) {
                return false;
            }

            {
                float y = 8.049f;
                float dist = 55.f;
                BSpline cameraBSplinePosition;
                cameraBSplinePosition.m_controlPoints = {{0, y, dist}, {dist, y, 0}, {0, y, -dist}, {-dist, y, 0}, {0, y, dist}, {dist, y, 0}, {0, y, -dist}};
                BSpline cameraBSplineOrientation;
                cameraBSplineOrientation.m_controlPoints = {{0, 0.f * glm::pi<float>(), 1.818f}, {0, 0.5f * glm::pi<float>(), 1.818f}, {0, 1.0f * glm::pi<float>(), 1.818f}, {0, 1.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 1.0f * glm::pi<float>(), 1.818f}};
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_spp = 6,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false,
                        .m_cameraBSplinePosition = cameraBSplinePosition,
                        .m_cameraBSplineOrientation = cameraBSplineOrientation,
                        .m_cameraBSplineU = 0,
                        .m_cameraBSplineSpeed = 0.5f,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 60 * 8,
                        .m_directory = subdirectory,
                        .m_name = ""};
                const auto videoSettings = VideoHeadlessApplication::VideoApplicationSettings{
                        .m_framePerExecution = 1};
                VideoHeadlessApplication app(appName, settings, videoSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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

    bool ReSTIRSSSVideo::videoAjaxPT12SPP() const {
               constexpr glm::vec3 position = {AJAX_POSITION};
        constexpr glm::vec3 orientation = {AJAX_ORIENTATION};
        constexpr int32_t width = 1920;
        constexpr int32_t height = 1080;

        std::string appName = "ReSTIR SSS";
        {
            const std::string subdirectory = m_directory + "/ajax_pt12spp";
            if (!std::filesystem::create_directories(subdirectory)) {
                return false;
            }

            {
                float y = 8.049f;
                float dist = 55.f;
                BSpline cameraBSplinePosition;
                cameraBSplinePosition.m_controlPoints = {{0, y, dist}, {dist, y, 0}, {0, y, -dist}, {-dist, y, 0}, {0, y, dist}, {dist, y, 0}, {0, y, -dist}};
                BSpline cameraBSplineOrientation;
                cameraBSplineOrientation.m_controlPoints = {{0, 0.f * glm::pi<float>(), 1.818f}, {0, 0.5f * glm::pi<float>(), 1.818f}, {0, 1.0f * glm::pi<float>(), 1.818f}, {0, 1.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 1.0f * glm::pi<float>(), 1.818f}};
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_spp = 12,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false,
                        .m_cameraBSplinePosition = cameraBSplinePosition,
                        .m_cameraBSplineOrientation = cameraBSplineOrientation,
                        .m_cameraBSplineU = 0,
                        .m_cameraBSplineSpeed = 0.5f,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 60 * 8,
                        .m_directory = subdirectory,
                        .m_name = ""};
                const auto videoSettings = VideoHeadlessApplication::VideoApplicationSettings{
                        .m_framePerExecution = 1};
                VideoHeadlessApplication app(appName, settings, videoSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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

    bool ReSTIRSSSVideo::videoAjaxPT12SPPDenoised() const {
                constexpr glm::vec3 position = {AJAX_POSITION};
        constexpr glm::vec3 orientation = {AJAX_ORIENTATION};
        constexpr int32_t width = 1920;
        constexpr int32_t height = 1080;

        std::string appName = "ReSTIR SSS";
        {
            const std::string subdirectory = m_directory + "/ajax_pt12sppdenoised";
            if (!std::filesystem::create_directories(subdirectory)) {
                return false;
            }

            {
                float y = 8.049f;
                float dist = 55.f;
                BSpline cameraBSplinePosition;
                cameraBSplinePosition.m_controlPoints = {{0, y, dist}, {dist, y, 0}, {0, y, -dist}, {-dist, y, 0}, {0, y, dist}, {dist, y, 0}, {0, y, -dist}};
                BSpline cameraBSplineOrientation;
                cameraBSplineOrientation.m_controlPoints = {{0, 0.f * glm::pi<float>(), 1.818f}, {0, 0.5f * glm::pi<float>(), 1.818f}, {0, 1.0f * glm::pi<float>(), 1.818f}, {0, 1.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 1.0f * glm::pi<float>(), 1.818f}};
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
                        .m_renderer = RENDERER_PATHTRACE,
                        .m_spp = 12,
                        .m_denoiser = true,
                        .m_candidate_samples_light = 1,
                        .m_candidate_samples_bsdf = 0,
                        .m_candidate_sss_all_intersections = true,
                        .m_temporal = false,
                        .m_spatial = false,
                        .m_spatial2 = false,
                        .m_cameraBSplinePosition = cameraBSplinePosition,
                        .m_cameraBSplineOrientation = cameraBSplineOrientation,
                        .m_cameraBSplineU = 0,
                        .m_cameraBSplineSpeed = 0.5f,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 60 * 8,
                        .m_directory = subdirectory,
                        .m_name = ""};
                const auto videoSettings = VideoHeadlessApplication::VideoApplicationSettings{
                        .m_framePerExecution = 1};
                VideoHeadlessApplication app(appName, settings, videoSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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

    bool ReSTIRSSSVideo::videoAjaxSequential() const {
        constexpr glm::vec3 position = {AJAX_POSITION};
        constexpr glm::vec3 orientation = {AJAX_ORIENTATION};
        constexpr int32_t width = 1920;
        constexpr int32_t height = 1080;

        std::string appName = "ReSTIR SSS";
        {
            const std::string subdirectory = m_directory + "/ajax_sequential";
            if (!std::filesystem::create_directories(subdirectory)) {
                return false;
            }

            {
                float y = 8.049f;
                float dist = 55.f;
                BSpline cameraBSplinePosition;
                cameraBSplinePosition.m_controlPoints = {{0, y, dist}, {dist, y, 0}, {0, y, -dist}, {-dist, y, 0}, {0, y, dist}, {dist, y, 0}, {0, y, -dist}};
                BSpline cameraBSplineOrientation;
                cameraBSplineOrientation.m_controlPoints = {{0, 0.f * glm::pi<float>(), 1.818f}, {0, 0.5f * glm::pi<float>(), 1.818f}, {0, 1.0f * glm::pi<float>(), 1.818f}, {0, 1.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 1.0f * glm::pi<float>(), 1.818f}};
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
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
                        .m_cameraBSplinePosition = cameraBSplinePosition,
                        .m_cameraBSplineOrientation = cameraBSplineOrientation,
                        .m_cameraBSplineU = 0,
                        .m_cameraBSplineSpeed = 0.5f,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 60 * 8,
                        .m_directory = subdirectory,
                        .m_name = ""};
                const auto videoSettings = VideoHeadlessApplication::VideoApplicationSettings{
                        .m_framePerExecution = 1};
                VideoHeadlessApplication app(appName, settings, videoSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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

    bool ReSTIRSSSVideo::videoAjaxSequentialDenoised() const {
        constexpr glm::vec3 position = {AJAX_POSITION};
        constexpr glm::vec3 orientation = {AJAX_ORIENTATION};
        constexpr int32_t width = 1920;
        constexpr int32_t height = 1080;

        std::string appName = "ReSTIR SSS";
        {
            const std::string subdirectory = m_directory + "/ajax_sequentialdenoised";
            if (!std::filesystem::create_directories(subdirectory)) {
                return false;
            }

            {
                float y = 8.049f;
                float dist = 55.f;
                BSpline cameraBSplinePosition;
                cameraBSplinePosition.m_controlPoints = {{0, y, dist}, {dist, y, 0}, {0, y, -dist}, {-dist, y, 0}, {0, y, dist}, {dist, y, 0}, {0, y, -dist}};
                BSpline cameraBSplineOrientation;
                cameraBSplineOrientation.m_controlPoints = {{0, 0.f * glm::pi<float>(), 1.818f}, {0, 0.5f * glm::pi<float>(), 1.818f}, {0, 1.0f * glm::pi<float>(), 1.818f}, {0, 1.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 0.5f * glm::pi<float>(), 1.818f}, {0, glm::two_pi<float>() + 1.0f * glm::pi<float>(), 1.818f}};
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
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
                        .m_cameraBSplinePosition = cameraBSplinePosition,
                        .m_cameraBSplineOrientation = cameraBSplineOrientation,
                        .m_cameraBSplineU = 0,
                        .m_cameraBSplineSpeed = 0.5f,
                };
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 60 * 8,
                        .m_directory = subdirectory,
                        .m_name = ""};
                const auto videoSettings = VideoHeadlessApplication::VideoApplicationSettings{
                        .m_framePerExecution = 1};
                VideoHeadlessApplication app(appName, settings, videoSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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

    bool ReSTIRSSSVideo::videoAsianDragonSpatiotemporalSequential() const {
        constexpr glm::vec3 position = {ASIAN_DRAGON_POSITION};
        constexpr glm::vec3 orientation = {ASIAN_DRAGON_ORIENTATION};
        constexpr glm::vec3 trajectoryStart = {ASIAN_DRAGON_TRAJECTORY_START};
        constexpr int32_t width = 1920;
        constexpr int32_t height = 1080;

        std::string appName = "ReSTIR SSS";
        {
            const std::string subdirectory = m_directory + "/asian_dragon";
            if (!std::filesystem::create_directories(subdirectory)) {
                return false;
            }
            const std::string namePrefix = "asian_dragon_";

            {
                // SPATIOTEMPORAL - SEQUENTIAL
                BSpline cameraBSplinePosition;
                cameraBSplinePosition.m_controlPoints = {{15.1209, 6.008, 3.6638}, {15.1209, 6.008, 3.6638}, {16.2165, 6.008, -0.964753}, {6.26706, 4.5061, -8.82182}, {-4.83692, 4.5061, -11.9024}, {-4.83692, 4.5061, -11.9024}};
                BSpline cameraBSplineOrientation;
                cameraBSplineOrientation.m_controlPoints = {{0, 1.20975, 2.15569}, {0, 1.20975, 2.15569}, {0, 1.64846, 2.09287}, {0, 2.88467, 1.96191}, {0, 3.41063, 1.98242}, {0, 3.41063, 1.98242}};
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::ASIAN_DRAGON,
                        .m_sceneMode = 3,
                        .m_position = position,
                        .m_orientation = orientation,
                        .m_maxFrames = 1,
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
                        .m_cameraBSplinePosition = cameraBSplinePosition,
                        .m_cameraBSplineOrientation = cameraBSplineOrientation,
                        .m_cameraBSplineU = 0};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 60 * 5,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatiotemporal_sequential",
                };
                const auto videoSettings = VideoHeadlessApplication::VideoApplicationSettings{};
                VideoHeadlessApplication app(appName, settings, videoSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
} // namespace raven