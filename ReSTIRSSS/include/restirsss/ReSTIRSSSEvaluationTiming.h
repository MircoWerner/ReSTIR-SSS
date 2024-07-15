#pragma once

#include "ReSTIRSSS.h"
#include "raven/core/TimingHeadlessApplication.h"

namespace raven {
    class ReSTIRSSSEvaluationTiming {
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
            oss << std::put_time(&tm, "evaluation-timing/%Y-%m-%d-%H-%M-%S");
            m_directory = oss.str();

            return std::filesystem::create_directories(m_directory);
        }

        [[nodiscard]] bool evaluateAjax() const;

        [[nodiscard]] bool evaluateAjaxManyLights() const;

        [[nodiscard]] bool evaluateAsianDragon(uint32_t sceneMode) const;

        [[nodiscard]] bool evaluateLTEOrb(uint32_t sceneMode) const;

        [[nodiscard]] bool evaluate() const {
            if (!evaluateAjax()) {
                return false;
            }

            if (!evaluateAjaxManyLights()) {
                return false;
            }

            for (uint32_t i = 0; i < 4; i++) {
                if (!evaluateAsianDragon(i)) {
                    return false;
                }
            }

            for (uint32_t i = 0; i < 11; i++) {
                if (i == 5 || i == 6 || i == 8 || i == 9 || i == 10) {
                    if (!evaluateLTEOrb(i)) {
                        return false;
                    }
                }
            }

            return true;
        }
    };
} // namespace raven