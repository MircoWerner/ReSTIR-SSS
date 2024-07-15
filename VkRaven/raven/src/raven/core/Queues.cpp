#include "raven/core/Queues.h"

namespace raven {

    Queues::QueueFamilyIndices Queues::findQueueFamilies(vk::PhysicalDevice physicalDevice) const {
        QueueFamilyIndices familyIndices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
        physicalDevice.getQueueFamilyProperties(&queueFamilyCount, queueFamilies.data());

        // graphics queue
        if (raven::Queues::QueueFamilyIndices::requireGraphicsFamily(m_requiredQueueFamilies)) {
            familyIndices.graphicsFamily = getQueueFamilyIndex(queueFamilies, vk::QueueFlagBits::eGraphics);
        }

        // dedicated compute queue
        if (raven::Queues::QueueFamilyIndices::requireComputeFamily(m_requiredQueueFamilies)) {
            familyIndices.computeFamily = getQueueFamilyIndex(queueFamilies, vk::QueueFlagBits::eCompute);
        }

        // dedicated transfer queue
        if (raven::Queues::QueueFamilyIndices::requireTransferFamily(m_requiredQueueFamilies)) {
            familyIndices.transferFamily = getQueueFamilyIndex(queueFamilies, vk::QueueFlagBits::eTransfer);
        }

        return familyIndices;
    }

    void Queues::generateQueueCreateInfos(vk::PhysicalDevice physicalDevice, std::vector<vk::DeviceQueueCreateInfo> *queueCreateInfos, float *queuePriorities) const {
        QueueFamilyIndices familyIndices = findQueueFamilies(physicalDevice);
        familyIndices.generateQueueCreateInfos(queueCreateInfos, queuePriorities);
    }

    void Queues::createQueues(vk::Device device, vk::PhysicalDevice physicalDevice) {
        QueueFamilyIndices familyIndices = findQueueFamilies(physicalDevice);
        if (isFamilyRequired(GRAPHICS_FAMILY)) {
            device.getQueue(familyIndices.graphicsFamily.value(), 0, &m_queues[GRAPHICS]);
        }
        if (isFamilyRequired(COMPUTE_FAMILY)) {
            device.getQueue(familyIndices.computeFamily.value(), 0, &m_queues[COMPUTE]);
        }
        if (isFamilyRequired(TRANSFER_FAMILY)) {
            device.getQueue(familyIndices.transferFamily.value(), 0, &m_queues[TRANSFER]);
        }
    }

    vk::Queue Queues::getQueue(Queues::Queue queue) {
        return m_queues[queue];
    }

    bool Queues::isFamilyRequired(Queues::QueueFamilies queueFamily) const {
        return m_requiredQueueFamilies & queueFamily;
    }
} // namespace raven