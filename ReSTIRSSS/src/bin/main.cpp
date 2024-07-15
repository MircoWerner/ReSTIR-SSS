#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION

#include "raven/core/RavenVkDynamicLoader.h"

#include "raven/core/Application.h"
#include "raven/core/HeadlessApplication.h"
#include "raven/util/Paths.h"

#include "restirsss/ReSTIRSSS.h"
#include "restirsss/ReSTIRSSSEvaluation.h"
#include "restirsss/ReSTIRSSSEvaluationTiming.h"
#include "restirsss/ReSTIRSSSVideo.h"

#include "argparse/include/argparse/argparse.hpp"

int main(const int argc, char *argv[]) {
#ifdef RESOURCE_DIRECTORY_PATH
    raven::Paths::m_resourceDirectoryPath = RESOURCE_DIRECTORY_PATH;
#endif

    argparse::ArgumentParser program("restirsss");
    program.add_argument("scene")
            .help("name of the scene (in resources/scenes/<name>.xml)")
            .required();
    program.add_argument("--preset")
            .nargs(1)
            .help("pathtracing, hybrid, sequential")
            .choices("pathtracing", "hybrid", "sequential")
            .default_value("hybrid");
    program.add_argument("--evaluate")
            .nargs(1)
            .help("renderings, timing, video")
            .choices("renderings", "timing", "video");

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    if (program.present("--evaluate")) {
        if (program.get("--evaluate") == "renderings") {
            return raven::ReSTIRSSSEvaluation().run();
        }
        if (program.get("--evaluate") == "timing") {
            return raven::ReSTIRSSSEvaluationTiming().run();
        }
        if (program.get("--evaluate") == "video") {
            return raven::ReSTIRSSSVideo().run();
        }
    }

    auto rendererSettings = raven::ReSTIRSSS::ReSTIRSSSSettings{.m_scene = raven::Rayscenes::UNDEFINED};
    const auto renderer = std::make_shared<raven::ReSTIRSSS>(rendererSettings, program.get("scene"), program.get("--preset"));
    std::string appName = "ReSTIR Subsurface Scattering for Real-Time Path Tracing";

#ifdef WIN32
    auto settings = raven::Application::ApplicationSettings{.m_msaaSamples = vk::SampleCountFlagBits::e1,
                                                            .m_deviceExtensions = {VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, VK_KHR_RAY_QUERY_EXTENSION_NAME,
                                                                                   VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME, VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME, VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME}};
#else
    auto settings = raven::Application::ApplicationSettings{.m_msaaSamples = vk::SampleCountFlagBits::e1,
                                                            .m_deviceExtensions = {VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, VK_KHR_RAY_QUERY_EXTENSION_NAME,
                                                                                   VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME, VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME, VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME}};
#endif
    raven::Application app(appName, settings, renderer, raven::Queues::QueueFamilies::COMPUTE_FAMILY | raven::Queues::QueueFamilies::GRAPHICS_FAMILY | raven::Queues::TRANSFER_FAMILY);
    app.setVSync(true);

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}