#include "raven/core/GPUContext.h"

namespace raven {
    GPUContext::GPUContext(uint32_t requiredQueueFamilies) : m_queues(std::make_shared<Queues>(requiredQueueFamilies)) {
    }

    void GPUContext::init() {
        initVulkan();
    }

    void GPUContext::shutdown() {
        releaseVulkan();
    }

    void GPUContext::initVulkan() {
        createInstance();
        setupDebugMessenger();
        //            createSurface(); -> moved to WSI
        pickPhysicalDevice();
        queryPhysicalDeviceProperties();
        createLogicalDevice();
        //        load_VK_EXTENSIONS(m_instance, vkGetInstanceProcAddr, m_device, vkGetDeviceProcAddr);
        m_debug = std::make_shared<VkDebugUtils>(m_device, enableValidationLayers);
        m_queues->createQueues(m_device, m_physicalDevice);
        //            createSwapChain(); -> WSI...
        //            createImageViews();
        //            createRenderPass();
        //            createDescriptorSetLayout();
        //            createGraphicsPipeline();
        //            createColorResources();
        //            createDepthResources();
        //            createFramebuffers();
        createCommandPool();
        //            createTextureImage();
        //            createTextureImageView();
        //            createTextureSampler();
        //            loadModel();
        //            createVertexBuffer();
        //            createIndexBuffer();
        //            createUniformBuffers();
        //            createDescriptorPool();
        //            createDescriptorSets();
        createCommandBuffers();
        //            createSyncObjects();
    }

    void GPUContext::releaseVulkan() {
        vkDestroyCommandPool(m_device, m_graphicsCommandPool, nullptr);
        vkDestroyCommandPool(m_device, m_computeCommandPool, nullptr);
        vkDestroyCommandPool(m_device, m_transferCommandPool, nullptr);
        vkDestroyDevice(m_device, nullptr);
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
        }
        vkDestroyInstance(m_instance, nullptr);
    }

    void GPUContext::createInstance() {
#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
        vk::DynamicLoader dl;
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
#endif

        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("Validation layers requested, but not available!");
        }

        vk::ApplicationInfo applicationInfo{};
        applicationInfo.pApplicationName = "Raven Application";
        applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.pEngineName = "VkRaven";
        applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.apiVersion = VK_API_VERSION_1_3;

        vk::InstanceCreateInfo instanceCreateInfo{};
        vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        std::vector<const char *> extensions;
        getInstanceExtensions(extensions);
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
        if (enableValidationLayers) {
            instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
            populateDebugMessengerCreateInfo(debugCreateInfo);
            instanceCreateInfo.pNext = (vk::DebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
        } else {
            instanceCreateInfo.enabledLayerCount = 0;
            instanceCreateInfo.pNext = nullptr;
        }
        if (vk::createInstance(&instanceCreateInfo, nullptr, &m_instance) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to create instance!");
        }

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);
#endif
    }

    bool GPUContext::checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<vk::LayerProperties> availableLayers(layerCount);
        if (vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data()) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to enumerate instance layer properties!");
        }

        for (const char *layerName: validationLayers) {
            bool layerFound = false;

            for (const auto &layerProperties: availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    void GPUContext::getInstanceExtensions(std::vector<const char *> &extensions) const {
        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
    }

    void GPUContext::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    VkBool32 GPUContext::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            std::cerr << "[Vulkan] " << pCallbackData->pMessage << std::endl;
        }

        return VK_FALSE;
    }

    void GPUContext::setupDebugMessenger() {
        if (!enableValidationLayers) {
            return;
        }

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up debug messenger!");
        }
    }

    VkResult GPUContext::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void GPUContext::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
                                                                                "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    void GPUContext::pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }
        std::vector<vk::PhysicalDevice> devices(deviceCount);
        if (m_instance.enumeratePhysicalDevices(&deviceCount, devices.data()) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to enumerate physical devices!");
        }
        std::cout << "Available devices: (" << deviceCount << ")" << std::endl;
        for (uint32_t i = 0; i < deviceCount; i++) {
            const auto &device = devices[i];
            vk::PhysicalDeviceProperties deviceProperties;
            device.getProperties(&deviceProperties);
            std::cout << "[" << i << "] " << deviceProperties.deviceName << std::endl;
        }
        std::cout << "Enter the number of the device... ";
        std::string s;
        if (deviceCount == 1) {
            s = "0";
            std::cout << s << std::endl;
        } else {
            std::cin >> s;
        }
        int i;
        try {
            i = std::stoi(s);
        } catch (std::invalid_argument const &ex) {
            std::cout << ex.what() << std::endl;
        } catch (std::out_of_range const &ex) {
            std::cout << ex.what() << std::endl;
        }
        if (i >= 0 && i < deviceCount) {
            m_physicalDevice = devices[i];
        } else {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }
    }

    void GPUContext::queryPhysicalDeviceProperties() {
        vk::PhysicalDeviceProperties deviceProperties;
        m_physicalDevice.getProperties(&deviceProperties);

        vk::PhysicalDeviceLimits deviceLimits = deviceProperties.limits;
        m_deviceLimitsTimestampPeriod = deviceLimits.timestampPeriod;
    }

    void GPUContext::createLogicalDevice() {
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        float queuePriority = 1.0f;
        m_queues->generateQueueCreateInfos(m_physicalDevice, &queueCreateInfos, &queuePriority);

        std::vector<const char *> deviceExtensions;
        getDeviceExtensions(deviceExtensions);

        vk::PhysicalDeviceFeatures2 features2;
        vk::PhysicalDeviceVulkan13Features features13;
        vk::PhysicalDeviceVulkan12Features features12;
        vk::PhysicalDeviceVulkan11Features features11;
        vk::PhysicalDeviceAccelerationStructureFeaturesKHR asFeatures;
        vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeatures;
        vk::PhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures;
        // vk::PhysicalDeviceHostQueryResetFeatures hostQueryReset;
        features2.pNext = &features13;
        features13.pNext = &features12;
        features12.pNext = &features11;
        features11.pNext = &asFeatures;
        asFeatures.pNext = &rtPipelineFeatures;
        rtPipelineFeatures.pNext = &rayQueryFeatures;
        // rayQueryFeatures.pNext = &hostQueryReset;

        m_physicalDevice.getFeatures2(&features2);

        vk::DeviceCreateInfo createInfo{};
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = nullptr;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        if (enableValidationLayers) {                                                      // not really necessary anymore, but still good to be compatible with older implementations (no distinction between instance and device specific validation layers anymore)
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()); // fields ignored by modern implementations
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }
        createInfo.pNext = &features2;
        if (m_physicalDevice.createDevice(&createInfo, nullptr, &m_device) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to create logical device!");
        }

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_device);
#endif
    }

    void GPUContext::getDeviceExtensions(std::vector<const char *> &extensions) const {}

    void GPUContext::createCommandPool() {
        Queues::QueueFamilyIndices queueFamilyIndices = m_queues->findQueueFamilies(m_physicalDevice);

        {
            vk::CommandPoolCreateInfo poolInfo{};
            poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
            poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

            if (m_device.createCommandPool(&poolInfo, nullptr, &m_graphicsCommandPool) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create command pool!");
            }
        }
        {
            vk::CommandPoolCreateInfo poolInfo{};
            poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
            poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();

            if (m_device.createCommandPool(&poolInfo, nullptr, &m_computeCommandPool) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create command pool!");
            }
        }
        {
            vk::CommandPoolCreateInfo poolInfo{};
            poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
            poolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

            if (m_device.createCommandPool(&poolInfo, nullptr, &m_transferCommandPool) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create command pool!");
            }
        }
    }

    void GPUContext::createCommandBuffers() {
        {
            m_graphicsCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

            vk::CommandBufferAllocateInfo allocInfo{};
            allocInfo.commandPool = m_graphicsCommandPool;
            allocInfo.level = vk::CommandBufferLevel::ePrimary;
            allocInfo.commandBufferCount = (uint32_t) m_graphicsCommandBuffers.size();

            if (m_device.allocateCommandBuffers(&allocInfo, m_graphicsCommandBuffers.data()) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to allocate command buffers!");
            }
        }
        {
            m_computeCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

            vk::CommandBufferAllocateInfo allocInfo{};
            allocInfo.commandPool = m_computeCommandPool;
            allocInfo.level = vk::CommandBufferLevel::ePrimary;
            allocInfo.commandBufferCount = (uint32_t) m_computeCommandBuffers.size();

            if (m_device.allocateCommandBuffers(&allocInfo, m_computeCommandBuffers.data()) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to allocate command buffers!");
            }
        }
        {
            m_transferCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

            vk::CommandBufferAllocateInfo allocInfo{};
            allocInfo.commandPool = m_transferCommandPool;
            allocInfo.level = vk::CommandBufferLevel::ePrimary;
            allocInfo.commandBufferCount = (uint32_t) m_transferCommandBuffers.size();

            if (m_device.allocateCommandBuffers(&allocInfo, m_transferCommandBuffers.data()) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to allocate command buffers!");
            }
        }
    }
} // namespace raven