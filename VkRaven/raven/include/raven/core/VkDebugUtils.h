#pragma once

#include <string>

#include "raven/RavenInclude.h"

namespace raven {
    class VkDebugUtils {
    public:
        VkDebugUtils(vk::Device device, bool enabled) : m_device(device), m_enabled(enabled) {
        }

        static std::string getExtensionName() {
            return VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        }

        void setObjectName(vk::ObjectType objectType, uint64_t objectHandle, const char *name) {
            if (!m_enabled) {
                return;
            }
            vk::DebugUtilsObjectNameInfoEXT nameInfo;
            nameInfo.objectType = objectType;
            nameInfo.objectHandle = objectHandle;
            nameInfo.pObjectName = name;
            if (m_device.setDebugUtilsObjectNameEXT(&nameInfo) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to set debug object name!");
            }
        }

        void setName(vk::ShaderModule object, const std::string &name) {
            setObjectName(vk::ObjectType::eShaderModule, (uint64_t) static_cast<VkShaderModule>(object), ("[ShaderModule]_" + name).c_str());
        }

        void setName(vk::DeviceMemory object, const std::string &name) {
            setObjectName(vk::ObjectType::eDeviceMemory, (uint64_t) static_cast<VkDeviceMemory>(object), ("[DeviceMemory]_" + name).c_str());
        }

        void setName(vk::Image object, const std::string &name) {
            setObjectName(vk::ObjectType::eImage, (uint64_t) static_cast<VkImage>(object), ("[Image]_" + name).c_str());
        }
        void setName(vk::ImageView object, const std::string &name) {
            setObjectName(vk::ObjectType::eImageView, (uint64_t) static_cast<VkImageView>(object), ("[ImageView]_" + name).c_str());
        }
        void setName(vk::Sampler object, const std::string &name) {
            setObjectName(vk::ObjectType::eSampler, (uint64_t) static_cast<VkSampler>(object), ("[Sampler]_" + name).c_str());
        }
        void setName(vk::Buffer object, const std::string &name) {
            setObjectName(vk::ObjectType::eBuffer, (uint64_t) static_cast<VkBuffer>(object), ("[Buffer]_" + name).c_str());
        }

    private:
        vk::Device m_device;

        bool m_enabled = false;
    };
} // namespace raven