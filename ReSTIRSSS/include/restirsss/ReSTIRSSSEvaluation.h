#pragma once

#include "ReSTIRSSS.h"
#include "raven/core/HeadlessApplication.h"

#include <memory>

namespace raven {
    class ReSTIRSSSEvaluation {
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

        const bool m_groundTruth = true;
        const bool m_png = true;
        const bool m_nonSSSObject = true;

        bool initialize() {
            const auto t = std::time(nullptr);
            const auto tm = *std::localtime(&t);
            std::ostringstream oss;
            oss << std::put_time(&tm, "evaluation/%Y-%m-%d-%H-%M-%S");
            m_directory = oss.str();

            return std::filesystem::create_directories(m_directory);
        }

        [[nodiscard]] bool evaluateAjax() const;

        [[nodiscard]] bool evaluateAjaxManyLights() const;

        [[nodiscard]] bool evaluateAjaxIntersectionCount() const;

        [[nodiscard]] bool evaluateAsianDragon(uint32_t sceneMode) const;

        [[nodiscard]] bool evaluateLTEOrb(uint32_t sceneMode) const;

        [[nodiscard]] bool evaluateSpatioTemporalConvergenceAjax() const;
        [[nodiscard]] bool evaluateSpatioTemporalConvergenceLTEOrb(uint32_t sceneMode) const;

        [[nodiscard]] bool evaluate() const {
            if (!evaluateAjax()) {
                return false;
            }

            if (!evaluateAjaxManyLights()) {
                return false;
            }

            if (!evaluateAjaxIntersectionCount()) {
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

            // if (!evaluateSpatioTemporalConvergenceAjax()) {
            //     return false;
            // }
            // if (!evaluateSpatioTemporalConvergenceLTEOrb(6)) {
            //     return false;
            // }

            return true;
        }

        static void writeRMSECompare(std::ofstream &compareScript, const std::string &namePrefix, const std::string &nameSuffix) {
            compareScript << "printf \"" << namePrefix << nameSuffix << ".png\\n\" 2>&1 | tee -a rmse.txt\n";
            compareScript << "compare -metric RMSE " << namePrefix << "ground_truth.png " << namePrefix << nameSuffix << ".png null: 2>&1 | tee -a rmse.txt\n";
            compareScript << "printf \"\\n\" 2>&1 | tee -a rmse.txt\n";
        }

        static void writeHDRRMSECompare(std::ofstream &compareScript, const std::string &namePrefix, const std::string &nameSuffix) {
            compareScript << "printf \"" << namePrefix << nameSuffix << ".hdr\\n\" 2>&1 | tee -a hdr_rmse.txt\n";
            compareScript << "./hdr_rmse.sh " << namePrefix << "ground_truth.hdr " << namePrefix << nameSuffix << ".hdr null: 2>&1 | tee -a hdr_rmse.txt\n";
            compareScript << "printf \"\\n\" 2>&1 | tee -a hdr_rmse.txt\n";
        }

        static void writeMSECompare(std::ofstream &compareScript, const std::string &namePrefix, const std::string &nameSuffix) {
            compareScript << "printf \"" << namePrefix << nameSuffix << ".png\\n\" 2>&1 | tee -a mse.txt\n";
            compareScript << "compare -metric MSE " << namePrefix << "ground_truth.png " << namePrefix << nameSuffix << ".png null: 2>&1 | tee -a mse.txt\n";
            compareScript << "printf \"\\n\" 2>&1 | tee -a mse.txt\n";
        }

        static void writeFLIPCompare(std::ofstream &compareScript, const std::string &namePrefix, const std::string &nameSuffix) {
            compareScript << "printf \"" << namePrefix << nameSuffix << ".png\\n\" 2>&1 | tee -a flip.txt\n";
            compareScript << "./flip.sh " << namePrefix << "ground_truth.png " << namePrefix << nameSuffix << ".png 2>&1 | tee -a flip.txt\n";
            compareScript << "printf \"\\n\" 2>&1 | tee -a flip.txt\n";
        }
    };
} // namespace raven