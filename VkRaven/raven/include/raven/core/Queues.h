#pragma once

#include <array>
#include <memory>
#include <optional>
#include <set>
#include <vector>

#include "raven/RavenInclude.h"

namespace raven {
    class Queues {
    public:
        enum QueueFamilies {
            GRAPHICS_FAMILY = 0x00000001,
            COMPUTE_FAMILY = 0x00000002,
            TRANSFER_FAMILY = 0x00000004,
        };

        enum Queue {
            GRAPHICS = 0,
            COMPUTE = 1,
            TRANSFER = 2,
        };

        explicit Queues(uint32_t requiredQueueFamilies) : m_requiredQueueFamilies(requiredQueueFamilies){};

        struct QueueFamilyIndices {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> computeFamily;
            std::optional<uint32_t> transferFamily;

            [[nodiscard]] bool isComplete(uint32_t requiredQueueFamilies) const {
                uint32_t graphics = GRAPHICS_FAMILY & requiredQueueFamilies;
                uint32_t compute = COMPUTE_FAMILY & requiredQueueFamilies;
                uint32_t transfer = TRANSFER_FAMILY & requiredQueueFamilies;
                return (!graphics || graphicsFamily.has_value()) && (!compute || computeFamily.has_value()) && (!transfer || transferFamily.has_value());
            }

            void generateQueueCreateInfos(std::vector<vk::DeviceQueueCreateInfo> *queueCreateInfos, float *queuePriorities) {
                std::set<uint32_t> uniqueQueueFamilies;
                if (graphicsFamily.has_value()) {
                    uniqueQueueFamilies.emplace(graphicsFamily.value());
                }
                if (computeFamily.has_value()) {
                    uniqueQueueFamilies.emplace(computeFamily.value());
                }
                if (transferFamily.has_value()) {
                    uniqueQueueFamilies.emplace(transferFamily.value());
                }

                for (uint32_t queueFamily: uniqueQueueFamilies) { // queue create infos for all required queues
                    vk::DeviceQueueCreateInfo queueCreateInfo{};
                    queueCreateInfo.queueFamilyIndex = queueFamily;
                    queueCreateInfo.queueCount = 1;
                    queueCreateInfo.pQueuePriorities = queuePriorities;
                    queueCreateInfos->push_back(queueCreateInfo);
                }
            }

            static bool requireGraphicsFamily(uint32_t requiredQueueFamilies) {
                return GRAPHICS_FAMILY & requiredQueueFamilies;
            }
            static bool requireComputeFamily(uint32_t requiredQueueFamilies) {
                return COMPUTE_FAMILY & requiredQueueFamilies;
            }
            static bool requireTransferFamily(uint32_t requiredQueueFamilies) {
                return TRANSFER_FAMILY & requiredQueueFamilies;
            }
        };

        [[nodiscard]] QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice) const;

        void generateQueueCreateInfos(vk::PhysicalDevice physicalDevice, std::vector<vk::DeviceQueueCreateInfo> *queueCreateInfos, float *queuePriorities) const;

        void createQueues(vk::Device device, vk::PhysicalDevice physicalDevice);

        vk::Queue getQueue(Queue queue);

    private:
        uint32_t m_requiredQueueFamilies;
        std::array<vk::Queue, 3> m_queues{}; // destroyed implicitly with the device

        [[nodiscard]] bool isFamilyRequired(QueueFamilies queueFamily) const;

        static uint32_t getQueueFamilyIndex(std::vector<vk::QueueFamilyProperties> &queueFamilyProperties, vk::QueueFlags queueFlags) {
            // dedicated compute queue
//            if (queueFlags & vk::QueueFlagBits::eCompute) {
//                for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
//                    if ((queueFamilyProperties[i].queueFlags & queueFlags) && !(queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)) {
//                        return i;
//                    }
//                }
//            }
//
//            // dedicated transfer queue
//            if (queueFlags & vk::QueueFlagBits::eTransfer) {
//                for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
//                    if ((queueFamilyProperties[i].queueFlags & queueFlags) && !(queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) && !(queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute)) {
//                        return i;
//                    }
//                }
//            }

            // graphics queue or fallback
            for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
                if (queueFamilyProperties[i].queueFlags & queueFlags) {
                    return i;
                }
            }

            throw std::runtime_error("Cannot find matching queue family index!");
        }
    };
} // namespace raven