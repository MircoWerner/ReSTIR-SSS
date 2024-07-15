#include "raven/core/Application.h"
#include "raven/core/Image.h"

#ifdef WIN32
#include <windows.h>
#include <winuser.h>
#endif

#include <utility>

#include "imgui_internal.h"

namespace raven {
    Application::Application(std::string &appName, ApplicationSettings appSettings, std::shared_ptr<Renderer> renderer, uint32_t requiredQueueFamilies)
        : GPUContext(requiredQueueFamilies), m_appName(appName), m_applicationSettings(std::move(appSettings)), m_renderer(std::move(renderer)) {}

    void Application::run() {
        init();
        loop();
        shutdown();
    }

    void Application::init() {
        initWindow();
        GPUContext::init();
        createSurface();
        assertMSAASamplesSupported(m_applicationSettings.m_msaaSamples);
        m_present = std::make_shared<SwapChain>(reinterpret_cast<GPUContext *>(this), m_surface, m_window, m_vsync);
        m_present->create();
        m_guiPass = std::make_shared<ImGuiPass>(reinterpret_cast<GPUContext *>(this), m_present);
        m_guiPass->create();

        initImGui();

        m_camera = std::make_shared<Camera>();
        m_camera->fromSphericalCoordinates(glm::vec3(0.f), 0.f, 0.f, 5.f * glm::pi<float>() / 8.f);

        m_renderer->initResources(this);
        m_renderer->initShaderResources(this);
        m_renderer->initSwapchainResources(this, m_present->getSwapchainExtent());

        createBlitPassResources();

        getCamera()->onCameraUpdate();
    }

    void Application::loop() {
        double accumulatedTime = 0.0;
        uint32_t frameCount = 0;

        m_time = std::chrono::high_resolution_clock::now();

        while (!glfwWindowShouldClose(m_window)) {
            double startTime = glfwGetTime();

            glfwPollEvents();
            update();
            render();

            // fps counter
            accumulatedTime += glfwGetTime() - startTime;
            frameCount++;
            if (accumulatedTime > 1.0) {
                std::ostringstream oss;
                oss << m_appName << "  " << std::round(frameCount / accumulatedTime) << " fps";
                glfwSetWindowTitle(m_window, oss.str().c_str());
                accumulatedTime = 0.0;
                frameCount = 0;
            }
        }
        vkDeviceWaitIdle(m_device);
    }

    void Application::update() {
        if (m_present->isNewSwapChainExtent()) {
            m_renderer->releaseSwapchainResources();
            m_renderer->initSwapchainResources(this, m_present->getSwapchainExtent());
            m_guiPass->recreateSwapChainResources();
            // recreateSwapchainImGui();
        }

        if (glfwGetKey(m_window, GLFW_KEY_F5) == GLFW_PRESS) {
            m_device.waitIdle();
            reloadShader();
        }
        if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(m_window, true);
        }

        const float deltaTime = incrementAndReturnDeltaTime();

        m_camera->update(m_window, deltaTime);
        m_camera->setAspect(Camera::extentToAspectRatio(m_present->getSwapchainExtent()));

        m_renderer->update(this, m_camera.get(), deltaTime);
    }

    void Application::render() {
        m_guiPass->awaitLastFrameFinished(); // waiting for the previous frame to finish, blocks the CPU

        Renderer::RendererResult rendererResult;
        m_renderer->render(this, m_camera.get(), getActiveIndex(), &rendererResult);

        vk::Image image;
        vk::Semaphore awaitBeforeExecution;
        if (!m_present->acquireNextImage(&image, &awaitBeforeExecution)) {
            return;
        }

        std::vector<vk::Semaphore> waitTimelineSemaphore;
        std::vector<uint64_t> waitTimelineSemaphoreValue;
        if (rendererResult.m_waitTimelineSemaphore) {
            waitTimelineSemaphore.push_back(rendererResult.m_waitTimelineSemaphore);
            waitTimelineSemaphoreValue.push_back(rendererResult.m_waitTimelineSemaphoreValue);
        }
        if (awaitBeforeExecution) {
            waitTimelineSemaphore.push_back(awaitBeforeExecution);
            waitTimelineSemaphoreValue.push_back(0);
        }

        executeBlitPass([&](vk::CommandBuffer commandBuffer) {
            {
                vk::ImageMemoryBarrier barrier{};
                barrier.oldLayout = vk::ImageLayout::eGeneral;
                barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = rendererResult.m_image->getImage();
                barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;
                barrier.srcAccessMask = vk::AccessFlagBits::eNone;
                barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
                vk::PipelineStageFlagBits sourceStage = vk::PipelineStageFlagBits::eHost;
                vk::PipelineStageFlagBits destinationStage = vk::PipelineStageFlagBits::eTransfer;
                commandBuffer.pipelineBarrier(
                        sourceStage, destinationStage,
                        {},
                        nullptr, nullptr, {barrier});
            }
            {
                vk::ImageMemoryBarrier barrier{};
                barrier.oldLayout = vk::ImageLayout::ePresentSrcKHR;
                barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = image;
                barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;
                barrier.srcAccessMask = vk::AccessFlagBits::eNone;
                barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
                vk::PipelineStageFlagBits sourceStage = vk::PipelineStageFlagBits::eHost;
                vk::PipelineStageFlagBits destinationStage = vk::PipelineStageFlagBits::eTransfer;
                commandBuffer.pipelineBarrier(
                        sourceStage, destinationStage,
                        {},
                        nullptr, nullptr, {barrier});
            }

            {
                vk::ImageBlit copyRegion{};
                copyRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                copyRegion.srcSubresource.layerCount = 1;
                copyRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                copyRegion.dstSubresource.layerCount = 1;
                copyRegion.srcOffsets[0] = vk::Offset3D{0, 0, 0};
                copyRegion.srcOffsets[1] = vk::Offset3D{static_cast<int32_t>(m_present->getSwapchainExtent().width), static_cast<int32_t>(m_present->getSwapchainExtent().height), 1};
                copyRegion.dstOffsets[0] = vk::Offset3D{0, 0, 0};
                copyRegion.dstOffsets[1] = vk::Offset3D{static_cast<int32_t>(m_present->getSwapchainExtent().width), static_cast<int32_t>(m_present->getSwapchainExtent().height), 1};
                vk::Filter filter = vk::Filter::eNearest;
                commandBuffer.blitImage(rendererResult.m_image->getImage(), vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion, filter);
            }

            {
                vk::ImageMemoryBarrier barrier{};
                barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
                barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = image;
                barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                barrier.dstAccessMask = vk::AccessFlagBits::eNone;
                vk::PipelineStageFlagBits sourceStage = vk::PipelineStageFlagBits::eTransfer;
                vk::PipelineStageFlagBits destinationStage = vk::PipelineStageFlagBits::eHost;
                commandBuffer.pipelineBarrier(
                        sourceStage, destinationStage,
                        {},
                        nullptr, nullptr, {barrier});
            }
            {
                vk::ImageMemoryBarrier barrier{};
                barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
                barrier.newLayout = vk::ImageLayout::eGeneral;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = rendererResult.m_image->getImage();
                barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
                barrier.dstAccessMask = vk::AccessFlagBits::eNone;
                vk::PipelineStageFlagBits sourceStage = vk::PipelineStageFlagBits::eTransfer;
                vk::PipelineStageFlagBits destinationStage = vk::PipelineStageFlagBits::eHost;
                commandBuffer.pipelineBarrier(
                        sourceStage, destinationStage,
                        {},
                        nullptr, nullptr, {barrier});
            }
        },
                        waitTimelineSemaphore.size(), waitTimelineSemaphore.data(), waitTimelineSemaphoreValue.data());

        recordImGui();

        std::vector<vk::Semaphore> awaitBeforePresentExecution{getBinarySemaphore()};
        const vk::Semaphore signalSemaphore = m_guiPass->render(awaitBeforePresentExecution);

        m_present->present(signalSemaphore);

        incrementActiveIndex();
    }

    void Application::shutdown() {
        releaseBlitPassResources();

        shutdownImGui();

        m_renderer->releaseSwapchainResources();
        m_renderer->releaseShaderResources();
        m_renderer->releaseResources();

        m_guiPass->release();
        m_present->release();
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        GPUContext::shutdown();
        releaseWindow();
    }


    void Application::initWindow() {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW!");
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_window = glfwCreateWindow(m_width, m_height, m_appName.c_str(), nullptr, nullptr);
        glfwMakeContextCurrent(m_window);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    }

    void Application::framebufferSizeCallback(GLFWwindow *window, int width, int height) {
        auto app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
        app->internalResizeWindow(width, height);
    }

    void Application::internalResizeWindow(int width, int height) {
        m_present->setFramebufferResized();
        m_width = width;
        m_height = height;
        std::cout << m_width << " " << m_height << std::endl;
    }

    void Application::resizeWindow(int width, int height) {
        glfwSetWindowSize(m_window, width, height);
        internalResizeWindow(width, height);
    }

    void Application::releaseWindow() {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void Application::createSurface() {
        if (glfwCreateWindowSurface(m_instance, m_window, nullptr, reinterpret_cast<VkSurfaceKHR *>(&m_surface)) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }
    }

    void Application::getDeviceExtensions(std::vector<const char *> &extensions) const {
        GPUContext::getDeviceExtensions(extensions);

        extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        extensions.insert(extensions.end(), m_applicationSettings.m_deviceExtensions.begin(), m_applicationSettings.m_deviceExtensions.end());
    }

    void Application::getInstanceExtensions(std::vector<const char *> &extensions) const {
        GPUContext::getInstanceExtensions(extensions);

        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);
    }

    void Application::setVSync(bool vsync) { m_vsync = vsync; }

    void Application::assertMSAASamplesSupported(vk::SampleCountFlagBits msaaSamples) {
        vk::PhysicalDeviceProperties physicalDeviceProperties;
        m_physicalDevice.getProperties(&physicalDeviceProperties);
        vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (!(counts & msaaSamples)) {
            throw std::runtime_error("MSAA sample count not supported.");
        }
    }

    void Application::initImGui() {
        //1: create descriptor pool for IMGUI
        // the size of the pool is very oversize, but it's copied from imgui demo itself.
        vk::DescriptorPoolSize pool_sizes[] =
                {
                        {vk::DescriptorType::eSampler, 1000},
                        {vk::DescriptorType::eCombinedImageSampler, 1000},
                        {vk::DescriptorType::eSampledImage, 1000},
                        {vk::DescriptorType::eStorageImage, 1000},
                        {vk::DescriptorType::eUniformTexelBuffer, 1000},
                        {vk::DescriptorType::eStorageTexelBuffer, 1000},
                        {vk::DescriptorType::eUniformBuffer, 1000},
                        {vk::DescriptorType::eStorageBuffer, 1000},
                        {vk::DescriptorType::eUniformBufferDynamic, 1000},
                        {vk::DescriptorType::eStorageBufferDynamic, 1000},
                        {vk::DescriptorType::eInputAttachment, 1000}};

        vk::DescriptorPoolCreateInfo pool_info = {};
        pool_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        if (m_device.createDescriptorPool(&pool_info, nullptr, &m_imGuiDescriptorPool) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to create descriptor pool for ImGui!");
        }

        // 2: initialize imgui library

        //this initializes the core structures of imgui
        ImGui::CreateContext();

#ifdef WIN32
        m_imGuiContext = ImGui::CreateContext();
        ImGui::SetCurrentContext(m_imGuiContext);
#endif

        // recreateSwapchainImGui();

        ImGui::StyleColorsDark();

        //this initializes imgui for GLFW
        ImGui_ImplGlfw_InitForVulkan(m_window, true);

        //this initializes imgui for Vulkan
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = m_instance;
        init_info.PhysicalDevice = m_physicalDevice;
        init_info.Device = m_device;
        init_info.Queue = m_queues->getQueue(Queues::GRAPHICS);
        init_info.DescriptorPool = m_imGuiDescriptorPool;
        init_info.MinImageCount = 2;
        init_info.ImageCount = 2;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.RenderPass = m_guiPass->getRenderPass();

        ImGui_ImplVulkan_Init(&init_info);

        //execute a gpu command to upload imgui font textures
        ImGui_ImplVulkan_CreateFontsTexture();

        m_imGuiInitialized = true;
    }

    void Application::shutdownImGui() {
        if (!m_imGuiInitialized) {
            return;
        }

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();

        ImGui::DestroyContext();

        vkDestroyDescriptorPool(m_device, m_imGuiDescriptorPool, nullptr);

        m_imGuiInitialized = false;
    }

    // void Application::recreateSwapchainImGui() {
    //     vk::SurfaceCapabilitiesKHR surfaceCapabilities{};
    //     if (m_physicalDevice.getSurfaceCapabilitiesKHR(m_surface, &surfaceCapabilities) != vk::Result::eSuccess) {
    //         throw std::runtime_error("Failed to get surface capabilities khr!");
    //     }
    //     if (m_imGuiInitialized) {
    //         ImGui_ImplVulkan_SetMinImageCount(surfaceCapabilities.minImageCount);
    //     }
    // }

    void Application::recordImGui() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //ImGui::ShowDemoWindow();

        m_renderer->recordImGui(this, m_camera.get());

        ImGui::Render();
    }

    void Application::reloadShader() {
        m_renderer->releaseSwapchainResources();
        m_renderer->releaseShaderResources();
        m_renderer->initShaderResources(this);
        m_renderer->initSwapchainResources(this, m_present->getSwapchainExtent());
    }
} // namespace raven
