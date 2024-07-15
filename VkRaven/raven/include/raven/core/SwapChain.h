#pragma once

#include "GPUContext.h"
#include "Image.h"

namespace raven {
    class SwapChain {
    public:
        SwapChain(GPUContext *gpuContext, vk::SurfaceKHR surface, GLFWwindow *window, bool vsync)
            : m_gpuContext(gpuContext), m_surface(surface), m_window(window), m_vsync(vsync) {}

        void create() {
            createSwapChain();
            createSwapchainImageViews();
            setImageLayout();
            createSyncObjects();
        }

        void release() {
            for (size_t i = 0; i < m_gpuContext->getMultiBufferedCount(); i++) { vkDestroySemaphore(m_gpuContext->m_device, m_imageAvailableSemaphores[i], nullptr); }
            releaseSwapChain();
        }

        bool acquireNextImage(vk::Image *image, vk::Semaphore *awaitBeforeExecution) {
            vk::Result result = m_gpuContext->m_device.acquireNextImageKHR(m_swapChain, UINT64_MAX,
                                                                           m_imageAvailableSemaphores[m_gpuContext->getActiveIndex()], VK_NULL_HANDLE,
                                                                           &m_nextImageIndex); // acquire image from the swap chain, send signal to semaphore when GPU finished, blocks all following calls to the GPU, does not block the CPU
            if (result == vk::Result::eErrorOutOfDateKHR) {
                recreateSwapChain();
                return false;
            } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) { throw std::runtime_error("failed to acquire swap chain image!"); }

            *image = m_swapChainImages[m_nextImageIndex];
            *awaitBeforeExecution = m_imageAvailableSemaphores[m_gpuContext->getActiveIndex()];
            return true;
        }

        void present(const vk::Semaphore &awaitBeforeExecution) {
            vk::PresentInfoKHR presentInfo{}; // present after the command buffer is executed
            if (awaitBeforeExecution) {
                presentInfo.waitSemaphoreCount = 1;
                presentInfo.pWaitSemaphores = &awaitBeforeExecution;
            }
            const vk::SwapchainKHR swapChains[] = {m_swapChain};
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;
            presentInfo.pImageIndices = &m_nextImageIndex;
            presentInfo.pResults = nullptr; // optional

            const vk::Result result = m_gpuContext->m_queues->getQueue(Queues::GRAPHICS).presentKHR(&presentInfo);
            if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_framebufferResized) {
                m_framebufferResized = false;
                recreateSwapChain();
            } else if (result != vk::Result::eSuccess) { throw std::runtime_error("Failed to present swap chain image!"); }
        }

        void setFramebufferResized() { m_framebufferResized = true; }

        vk::Extent2D getSwapchainExtent() { return m_swapChainExtent; }

        vk::Format getSwapChainImageFormat() {
            return m_swapChainImageFormat;
        }

        size_t getNumSwapChainImageViews() {
            return m_swapChainImageViews.size();
        }

        vk::ImageView getSwapChainImageView(size_t index) {
            return m_swapChainImageViews[index];
        }

        bool isNewSwapChainExtent() {
            bool b = m_newSwapChainExtent;
            m_newSwapChainExtent = false;
            return b;
        }

        [[nodiscard]] uint32_t getNextImageIndex() const {
            return m_nextImageIndex;
        }

    private:
        GPUContext *m_gpuContext;
        vk::SurfaceKHR m_surface;
        GLFWwindow *m_window{};
        bool m_vsync;

        vk::SwapchainKHR m_swapChain{};
        std::vector<vk::Image> m_swapChainImages; // cleaned up when the swap chain is destroyed
        vk::Format m_swapChainImageFormat{};
        vk::Extent2D m_swapChainExtent{};
        std::vector<vk::ImageView> m_swapChainImageViews;

        uint32_t m_nextImageIndex = 0;

        std::vector<vk::Semaphore> m_imageAvailableSemaphores;

        bool m_framebufferResized = false;
        bool m_newSwapChainExtent = false;

        void createSwapChain() {
            SwapChainSupportDetails swapChainSupportDetails = querySwapChainSupport(m_gpuContext->m_physicalDevice);

            vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupportDetails.formats);
            vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupportDetails.presentModes, m_vsync);
            vk::Extent2D extent = chooseSwapExtent(swapChainSupportDetails.capabilities);

            uint32_t imageCount = std::max(swapChainSupportDetails.capabilities.minImageCount, m_gpuContext->getMultiBufferedCount()); // TODO(Mirco): check if sizes differ

            vk::SwapchainCreateInfoKHR createInfo{};
            createInfo.surface = m_surface;
            createInfo.minImageCount = imageCount;
            createInfo.imageFormat = surfaceFormat.format;
            createInfo.imageColorSpace = surfaceFormat.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
            createInfo.imageSharingMode = vk::SharingMode::eExclusive;
            createInfo.queueFamilyIndexCount = 0;     // optional
            createInfo.pQueueFamilyIndices = nullptr; // optional

            createInfo.preTransform = swapChainSupportDetails.capabilities.currentTransform; // no transformation of the image (e.g. rotate 90 degrees...)
            createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;              // ignore the alpha channel
            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE;             // pixels that are not visible, e.g. overlapped by other window, are not rendered -> better performance, but has to be changed if all pixels are relevant (e.g. screenshot)
            createInfo.oldSwapchain = VK_NULL_HANDLE; // pointer of the old swap chain if it is recreated (e.g. when resizing the window)

            if (m_gpuContext->m_device.createSwapchainKHR(&createInfo, nullptr, &m_swapChain) != vk::Result::eSuccess) { throw std::runtime_error("Failed to create swap chain!"); }

            vkGetSwapchainImagesKHR(m_gpuContext->m_device, m_swapChain, &imageCount,
                                    nullptr); // implementation can create more images than the specified minimum number of images before, therefore query actual number
            m_swapChainImages.resize(imageCount);
            if (m_gpuContext->m_device.getSwapchainImagesKHR(m_swapChain, &imageCount, m_swapChainImages.data()) != vk::Result::eSuccess) { throw std::runtime_error("Failed to get swapchain images!"); }

            m_swapChainImageFormat = surfaceFormat.format; // save for later
            m_swapChainExtent = extent;
        }

        void releaseSwapChain() {
            for (auto imageView: m_swapChainImageViews) { vkDestroyImageView(m_gpuContext->m_device, imageView, nullptr); }
            vkDestroySwapchainKHR(m_gpuContext->m_device, m_swapChain, nullptr);
        }

        void recreateSwapChain() {
            int width = 0, height = 0;
            glfwGetFramebufferSize(m_window, &width, &height);
            while (width == 0 || height == 0) {
                // wait until window is not minimized anymore
                glfwGetFramebufferSize(m_window, &width, &height);
                glfwWaitEvents();
            }

            vkDeviceWaitIdle(m_gpuContext->m_device);

            releaseSwapChain();

            createSwapChain();
            createSwapchainImageViews();
            setImageLayout();

            m_newSwapChainExtent = true;
        }

        void createSwapchainImageViews() {
            m_swapChainImageViews.resize(m_swapChainImages.size());

            for (size_t i = 0; i < m_swapChainImages.size(); i++) {
                m_swapChainImageViews[i] = Image::createImageView(m_gpuContext->m_device, m_swapChainImages[i], m_swapChainImageFormat, vk::ImageAspectFlagBits::eColor, 1);

                std::stringstream ss;
                ss << "swapchain[" << i << "]";
                m_gpuContext->getDebug()->setName(m_swapChainImages[i], ss.str());
                m_gpuContext->getDebug()->setName(m_swapChainImageViews[i], ss.str());
            }
        }

        vk::Format findDepthFormat() {
            return findSupportedFormat(
                    {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                    vk::ImageTiling::eOptimal,
                    vk::FormatFeatureFlagBits::eDepthStencilAttachment);
        }

        vk::Format
        findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
            for (vk::Format format: candidates) {
                vk::FormatProperties props;
                m_gpuContext->m_physicalDevice.getFormatProperties(format, &props);

                if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) { return format; } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) { return format; }
            }

            throw std::runtime_error("Failed to find supported format!");
        }

        struct SwapChainSupportDetails {
            vk::SurfaceCapabilitiesKHR capabilities{};    // min/max number of images in swap chain, min/max width and height of images
            std::vector<vk::SurfaceFormatKHR> formats;    // pixel format, color space
            std::vector<vk::PresentModeKHR> presentModes; // available presentation modes
        };

        SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device) {
            SwapChainSupportDetails details;

            if (device.getSurfaceCapabilitiesKHR(m_surface, &details.capabilities) != vk::Result::eSuccess) { throw std::runtime_error("Failed to get surface capabilities!"); }

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
            if (formatCount != 0) {
                details.formats.resize(formatCount);
                if (device.getSurfaceFormatsKHR(m_surface, &formatCount, details.formats.data()) != vk::Result::eSuccess) { throw std::runtime_error("Failed to get surface formats!"); }
            }

            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);
            if (presentModeCount != 0) {
                details.presentModes.resize(presentModeCount);
                if (device.getSurfacePresentModesKHR(m_surface, &presentModeCount, details.presentModes.data()) != vk::Result::eSuccess) { throw std::runtime_error("Failed to get surface present modes!"); }
            }

            return details;
        }

        static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
            for (const auto &availableFormat: availableFormats) {
#ifdef WIN32
                if (availableFormat.format == vk::Format::eR8G8B8A8Snorm &&
                    availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                    return availableFormat; // use this if it is available
                }
#else
                if (availableFormat.format == vk::Format::eR8G8B8A8Srgb &&
                    availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                    return availableFormat; // use this if it is available
                }
#endif
            }

            return availableFormats[0]; // otherwise just return any
        }

        static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes, bool vsync) {
            if (vsync) {
                return vk::PresentModeKHR::eFifo; // v-sync (rendered images inserted at the back of the queue, program has to wait if queue is full), guaranteed to be available
            }

            auto presentMode = vk::PresentModeKHR::eFifo;

            for (const auto &availablePresentMode: availablePresentModes) {
                if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                    return vk::PresentModeKHR::eMailbox; // triple buffering (like v-sync, but does not block program if queue is full by replacing already queued images, allows rendering fast as possible, more energy consuming)
                }
                if (availablePresentMode == vk::PresentModeKHR::eImmediate) { presentMode = vk::PresentModeKHR::eImmediate; }
            }

            if (presentMode == vk::PresentModeKHR::eFifo) { throw std::runtime_error("Cannot create a swapchain without vsync."); }

            return presentMode;
        }

        vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) { return capabilities.currentExtent; } else {
                int width, height;
                glfwGetFramebufferSize(m_window, &width, &height);

                vk::Extent2D actualExtent = {
                        static_cast<uint32_t>(width),
                        static_cast<uint32_t>(height)};

                actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                                capabilities.maxImageExtent.width);
                actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                                 capabilities.maxImageExtent.height);

                return actualExtent;
            }
        }

        void createSyncObjects() {
            m_imageAvailableSemaphores.resize(m_gpuContext->getMultiBufferedCount());

            vk::SemaphoreCreateInfo semaphoreInfo{};

            for (size_t i = 0; i < m_gpuContext->getMultiBufferedCount(); i++) { if (m_gpuContext->m_device.createSemaphore(&semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != vk::Result::eSuccess) { throw std::runtime_error("Failed to create synchronization objects for a frame!"); } }
        }

        void setImageLayout() {
            for (const auto &image: m_swapChainImages) {
                vk::ImageMemoryBarrier barrier{};
                barrier.oldLayout = vk::ImageLayout::eUndefined;
                barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = image;
                barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;
                barrier.srcAccessMask = vk::AccessFlagBits::eNone;;
                barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

                vk::PipelineStageFlagBits sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
                vk::PipelineStageFlagBits destinationStage = vk::PipelineStageFlagBits::eTransfer;

                m_gpuContext->executeCommands([&](vk::CommandBuffer commandBuffer) {
                    commandBuffer.pipelineBarrier(
                            sourceStage, destinationStage,
                            {},
                            nullptr, nullptr, {barrier});
                });
            }
        }
    };
} // namespace raven
