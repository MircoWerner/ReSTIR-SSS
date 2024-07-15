#include "restirsss/ReSTIRSSSEvaluationTiming.h"

namespace raven {
    constexpr uint32_t timingExecutions = 4;
    constexpr uint32_t startupExecutions = 128;

    bool ReSTIRSSSEvaluationTiming::evaluateAjax() const {
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

            {
                // PT
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
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_1spp",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_6spp",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_12spp",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_reconnection",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_delayed_reconnection",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_hybrid",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_sequential",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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

    bool ReSTIRSSSEvaluationTiming::evaluateAjaxManyLights() const {
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

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::AJAX_MANY_LIGHTS_WO_LARGE_LIGHT,
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
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_1spp",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_6spp",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_12spp",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_reconnection",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_delayed_reconnection",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_hybrid",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "spatial_sequential",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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

    bool ReSTIRSSSEvaluationTiming::evaluateAsianDragon(const uint32_t sceneMode) const {
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

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::ASIAN_DRAGON,
                        .m_sceneMode = sceneMode,
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
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_1spp",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_6spp",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_8spp",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_9spp",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_12spp",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
            //             .m_rendererExecutions = 16,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatial_reconnection",
            //             .m_executeAfterInit = [&](GPUContext *gpuContext) {
            //                 constexpr glm::vec3 start = trajectoryStart;
            //                 constexpr glm::vec3 end = position;
            //                 gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
            //                 gpuContext->getCamera()->initTrajectory(start, end, 16);
            //             }};
            //     const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
            //             .m_timingExecutions = timingExecutions,
            //             .m_startupExecutions = startupExecutions,
            //             .m_executeQueryTiming = [&](std::ofstream &stream) {
            //                 const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
            //                 const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
            //                 const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
            //                 const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
            //                 const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
            //                 const auto total = candidateGeneration + temporal + spatial + spatial2 + shade;
            //                 stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(total) << std::endl;
            //                 std::cout << "Last frame time: " << total << "ms" << std::endl;
            //             }};
            //     TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
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
            //     const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
            //             .m_timingExecutions = timingExecutions,
            //             .m_startupExecutions = startupExecutions,
            //             .m_executeQueryTiming = [&](std::ofstream &stream) {
            //                 const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
            //                 const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
            //                 const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
            //                 const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
            //                 const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
            //                 const auto total = candidateGeneration + temporal + spatial + spatial2 + shade;
            //                 stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(total) << std::endl;
            //                 std::cout << "Last frame time: " << total << "ms" << std::endl;
            //             }};
            //     TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
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
            //             .m_rendererExecutions = 16,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatial_delayed_reconnection",
            //             .m_executeAfterInit = [&](GPUContext *gpuContext) {
            //                 constexpr glm::vec3 start = trajectoryStart;
            //                 constexpr glm::vec3 end = position;
            //                 gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
            //                 gpuContext->getCamera()->initTrajectory(start, end, 16);
            //             }};
            //     const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
            //             .m_timingExecutions = timingExecutions,
            //             .m_startupExecutions = startupExecutions,
            //             .m_executeQueryTiming = [&](std::ofstream &stream) {
            //                 const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
            //                 const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
            //                 const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
            //                 const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
            //                 const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
            //                 const auto total = candidateGeneration + temporal + spatial + spatial2 + shade;
            //                 stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(total) << std::endl;
            //                 std::cout << "Last frame time: " << total << "ms" << std::endl;
            //             }};
            //     TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
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
            //     const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
            //             .m_timingExecutions = timingExecutions,
            //             .m_startupExecutions = startupExecutions,
            //             .m_executeQueryTiming = [&](std::ofstream &stream) {
            //                 const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
            //                 const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
            //                 const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
            //                 const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
            //                 const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
            //                 const auto total = candidateGeneration + temporal + spatial + spatial2 + shade;
            //                 stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(total) << std::endl;
            //                 std::cout << "Last frame time: " << total << "ms" << std::endl;
            //             }};
            //     TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
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
            //             .m_rendererExecutions = 16,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatial_hybrid",
            //             .m_executeAfterInit = [&](GPUContext *gpuContext) {
            //                 constexpr glm::vec3 start = trajectoryStart;
            //                 constexpr glm::vec3 end = position;
            //                 gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
            //                 gpuContext->getCamera()->initTrajectory(start, end, 16);
            //             }};
            //     const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
            //             .m_timingExecutions = timingExecutions,
            //             .m_startupExecutions = startupExecutions,
            //             .m_executeQueryTiming = [&](std::ofstream &stream) {
            //                 const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
            //                 const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
            //                 const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
            //                 const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
            //                 const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
            //                 const auto total = candidateGeneration + temporal + spatial + spatial2 + shade;
            //                 stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(total) << std::endl;
            //                 std::cout << "Last frame time: " << total << "ms" << std::endl;
            //             }};
            //     TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
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
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
            //             .m_rendererExecutions = 16,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatial_sequential",
            //             .m_executeAfterInit = [&](GPUContext *gpuContext) {
            //                 constexpr glm::vec3 start = trajectoryStart;
            //                 constexpr glm::vec3 end = position;
            //                 gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
            //                 gpuContext->getCamera()->initTrajectory(start, end, 16);
            //             }};
            //     const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
            //             .m_timingExecutions = timingExecutions,
            //             .m_startupExecutions = startupExecutions,
            //             .m_executeQueryTiming = [&](std::ofstream &stream) {
            //                 const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
            //                 const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
            //                 const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
            //                 const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
            //                 const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
            //                 const auto total = candidateGeneration + temporal + spatial + spatial2 + shade;
            //                 stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(total) << std::endl;
            //                 std::cout << "Last frame time: " << total << "ms" << std::endl;
            //             }};
            //     TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
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
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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

    bool ReSTIRSSSEvaluationTiming::evaluateLTEOrb(const uint32_t sceneMode) const {
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

            {
                // PT
                auto rendererSettings = ReSTIRSSS::ReSTIRSSSSettings{
                        .m_scene = Rayscenes::LTE_ORB,
                        .m_sceneMode = sceneMode,
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
                        .m_spatial2 = false};
                const auto renderer = std::make_shared<ReSTIRSSS>(rendererSettings);

                const auto settings = HeadlessApplication::ApplicationSettings{
                        .m_msaaSamples = vk::SampleCountFlagBits::e1,
                        .m_deviceExtensions = DEVICE_EXTENSIONS,
                        .m_width = width,
                        .m_height = height,
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_1spp",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_6spp",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
                        .m_rendererExecutions = 16,
                        .m_directory = subdirectory,
                        .m_name = namePrefix + "pathtracer_12spp",
                        .m_executeAfterInit = [&](GPUContext *gpuContext) {
                            constexpr glm::vec3 start = trajectoryStart;
                            constexpr glm::vec3 end = position;
                            gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
                            gpuContext->getCamera()->initTrajectory(start, end, 16);
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
            //             .m_rendererExecutions = 16,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatial_reconnection",
            //             .m_executeAfterInit = [&](GPUContext *gpuContext) {
            //                 constexpr glm::vec3 start = trajectoryStart;
            //                 constexpr glm::vec3 end = position;
            //                 gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
            //                 gpuContext->getCamera()->initTrajectory(start, end, 16);
            //             }};
            //     const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
            //             .m_timingExecutions = timingExecutions,
            //             .m_startupExecutions = startupExecutions,
            //             .m_executeQueryTiming = [&](std::ofstream &stream) {
            //                 const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
            //                 const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
            //                 const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
            //                 const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
            //                 const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
            //                 const auto total = candidateGeneration + temporal + spatial + spatial2 + shade;
            //                 stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(total) << std::endl;
            //                 std::cout << "Last frame time: " << total << "ms" << std::endl;
            //             }};
            //     TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
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
            //     const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
            //             .m_timingExecutions = timingExecutions,
            //             .m_startupExecutions = startupExecutions,
            //             .m_executeQueryTiming = [&](std::ofstream &stream) {
            //                 const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
            //                 const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
            //                 const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
            //                 const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
            //                 const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
            //                 const auto total = candidateGeneration + temporal + spatial + spatial2 + shade;
            //                 stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(total) << std::endl;
            //                 std::cout << "Last frame time: " << total << "ms" << std::endl;
            //             }};
            //     TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
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
            //             .m_rendererExecutions = 16,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatial_delayed_reconnection",
            //             .m_executeAfterInit = [&](GPUContext *gpuContext) {
            //                 constexpr glm::vec3 start = trajectoryStart;
            //                 constexpr glm::vec3 end = position;
            //                 gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
            //                 gpuContext->getCamera()->initTrajectory(start, end, 16);
            //             }};
            //     const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
            //             .m_timingExecutions = timingExecutions,
            //             .m_startupExecutions = startupExecutions,
            //             .m_executeQueryTiming = [&](std::ofstream &stream) {
            //                 const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
            //                 const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
            //                 const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
            //                 const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
            //                 const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
            //                 const auto total = candidateGeneration + temporal + spatial + spatial2 + shade;
            //                 stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(total) << std::endl;
            //                 std::cout << "Last frame time: " << total << "ms" << std::endl;
            //             }};
            //     TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
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
            //     const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
            //             .m_timingExecutions = timingExecutions,
            //             .m_startupExecutions = startupExecutions,
            //             .m_executeQueryTiming = [&](std::ofstream &stream) {
            //                 const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
            //                 const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
            //                 const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
            //                 const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
            //                 const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
            //                 const auto total = candidateGeneration + temporal + spatial + spatial2 + shade;
            //                 stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(total) << std::endl;
            //                 std::cout << "Last frame time: " << total << "ms" << std::endl;
            //             }};
            //     TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
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
            //             .m_rendererExecutions = 16,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatial_hybrid",
            //             .m_executeAfterInit = [&](GPUContext *gpuContext) {
            //                 constexpr glm::vec3 start = trajectoryStart;
            //                 constexpr glm::vec3 end = position;
            //                 gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
            //                 gpuContext->getCamera()->initTrajectory(start, end, 16);
            //             }};
            //     const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
            //             .m_timingExecutions = timingExecutions,
            //             .m_startupExecutions = startupExecutions,
            //             .m_executeQueryTiming = [&](std::ofstream &stream) {
            //                 const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
            //                 const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
            //                 const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
            //                 const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
            //                 const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
            //                 const auto total = candidateGeneration + temporal + spatial + spatial2 + shade;
            //                 stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(total) << std::endl;
            //                 std::cout << "Last frame time: " << total << "ms" << std::endl;
            //             }};
            //     TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
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
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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
            //             .m_rendererExecutions = 16,
            //             .m_directory = subdirectory,
            //             .m_name = namePrefix + "spatial_sequential",
            //             .m_executeAfterInit = [&](GPUContext *gpuContext) {
            //                 constexpr glm::vec3 start = trajectoryStart;
            //                 constexpr glm::vec3 end = position;
            //                 gpuContext->getCamera()->fromSphericalCoordinates(start, orientation.x, orientation.y, orientation.z);
            //                 gpuContext->getCamera()->initTrajectory(start, end, 16);
            //             }};
            //     const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
            //             .m_timingExecutions = timingExecutions,
            //             .m_startupExecutions = startupExecutions,
            //             .m_executeQueryTiming = [&](std::ofstream &stream) {
            //                 const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
            //                 const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
            //                 const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
            //                 const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
            //                 const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
            //                 const auto total = candidateGeneration + temporal + spatial + spatial2 + shade;
            //                 stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(total) << std::endl;
            //                 std::cout << "Last frame time: " << total << "ms" << std::endl;
            //             }};
            //     TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);
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
                        }};
                const auto timingSettings = TimingHeadlessApplication::TimingApplicationSettings{
                        .m_timingExecutions = timingExecutions,
                        .m_startupExecutions = startupExecutions,
                        .m_executeQueryTiming = [&](std::ofstream &stream) {
                            const auto candidateGeneration = std::isnan(renderer->getCandidateGenerationTimeAveraged()) ? 0 : renderer->getCandidateGenerationTimeAveraged();
                            const auto temporal = std::isnan(renderer->getTemporalReuseTimeAveraged()) ? 0 : renderer->getTemporalReuseTimeAveraged();
                            const auto spatial = std::isnan(renderer->getSpatialReuseTimeAveraged()) ? 0 : renderer->getSpatialReuseTimeAveraged();
                            const auto spatial2 = std::isnan(renderer->getSpatialReuse2TimeAveraged()) ? 0 : renderer->getSpatialReuse2TimeAveraged();
                            const auto shade = std::isnan(renderer->getShadeTimeAveraged()) ? 0 : renderer->getShadeTimeAveraged();
                            const auto pt = std::isnan(renderer->getPathtraceTimeAveraged()) ? 0 : renderer->getPathtraceTimeAveraged();
                            const auto total = candidateGeneration + temporal + spatial + spatial2 + shade + pt;
                            stream << std::to_string(candidateGeneration) << ";" << std::to_string(temporal) << ";" << std::to_string(spatial) << ";" << std::to_string(spatial2) << ";" << std::to_string(shade) << ";" << std::to_string(pt) << ";" << std::to_string(total) << std::endl;
                            std::cout << "Last frame time: " << total << "ms" << std::endl;
                        }};
                TimingHeadlessApplication app(appName, settings, timingSettings, renderer, Queues::QueueFamilies::COMPUTE_FAMILY | Queues::QueueFamilies::GRAPHICS_FAMILY | Queues::TRANSFER_FAMILY);

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