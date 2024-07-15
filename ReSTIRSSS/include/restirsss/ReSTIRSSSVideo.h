#pragma once

#include "ReSTIRSSS.h"

#include <filesystem>

namespace raven {
    class ReSTIRSSSVideo {
#ifdef WIN32
#define DEVICE_EXTENSIONS \
    { VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, VK_KHR_RAY_QUERY_EXTENSION_NAME, VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME, VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME, VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME }
#else
#define DEVICE_EXTENSIONS \
    { VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, VK_KHR_RAY_QUERY_EXTENSION_NAME, VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME, VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME, VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME }
#endif

    public:
        int run() {
            if (!initialize()) {
                return EXIT_FAILURE;
            }
            if (!evaluate()) {
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }

    private:
        std::string m_directory;

        bool initialize() {
            const auto t = std::time(nullptr);
            const auto tm = *std::localtime(&t);
            std::ostringstream oss;
            oss << std::put_time(&tm, "video/%Y-%m-%d-%H-%M-%S");
            m_directory = oss.str();

            return std::filesystem::create_directories(m_directory);
        }

        [[nodiscard]] bool videoAjaxReference() const;
        [[nodiscard]] bool videoAjaxPT1SPP() const;
        [[nodiscard]] bool videoAjaxPT1SPPDenoised() const;
        [[nodiscard]] bool videoAjaxPT6SPP() const;
        [[nodiscard]] bool videoAjaxPT6SPPDenoised() const;
        [[nodiscard]] bool videoAjaxPT12SPP() const;
        [[nodiscard]] bool videoAjaxPT12SPPDenoised() const;
        [[nodiscard]] bool videoAjaxSequential() const;
        [[nodiscard]] bool videoAjaxSequentialDenoised() const;

        [[nodiscard]] bool videoAsianDragonSpatiotemporalSequential() const;

        [[nodiscard]] bool evaluate() const {
            if (!videoAjaxReference()) {
                return false;
            }
            if (!videoAjaxPT1SPP()) {
                return false;
            }
            if (!videoAjaxPT1SPPDenoised()) {
                return false;
            }
            // if (!videoAjaxPT6SPP()) {
            //     return false;
            // }
            // if (!videoAjaxPT6SPPDenoised()) {
            //     return false;
            // }
            // if (!videoAjaxPT12SPP()) {
            //     return false;
            // }
            // if (!videoAjaxPT12SPPDenoised()) {
            //     return false;
            // }
            if (!videoAjaxSequential()) {
                return false;
            }
            if (!videoAjaxSequentialDenoised()) {
                return false;
            }

            // if (!videoAsianDragonSpatiotemporalSequential()) {
            //     return false;
            // }

            return true;
        }
    };
} // namespace raven