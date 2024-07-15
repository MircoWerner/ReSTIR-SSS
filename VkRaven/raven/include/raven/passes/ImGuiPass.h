#pragma once

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "raven/core/GPUContext.h"
#include "../core/SwapChain.h"

namespace raven {
    class ImGuiPass {
    public:
        ImGuiPass(GPUContext *gpuContext, const std::shared_ptr<SwapChain>& swapChain)
            : m_gpuContext(gpuContext), m_swapChain(swapChain) {}

        void create() {
            m_queueFamilyIndex = findQueueFamilyIndex();
            createCommandPool();
            createCommandBuffers();
            createRenderPass();
            createSwapChainResources();
            createSyncObjects();
        }

        void release() {
            releaseSyncObjects();
            vkDestroyRenderPass(m_gpuContext->m_device, m_renderPass, nullptr);
            releaseSwapChainResources();
            vkDestroyCommandPool(m_gpuContext->m_device, m_commandPool, nullptr);
        }

        void recreateSwapChainResources() {
            releaseSwapChainResources();
            createSwapChainResources();
        }

        void recordCommandBuffer(vk::CommandBuffer commandBuffer, vk::Framebuffer framebuffer) {
            vk::CommandBufferBeginInfo beginInfo{};
            if (commandBuffer.begin(&beginInfo) != vk::Result::eSuccess) { throw std::runtime_error("Failed to begin recording command buffer!"); }

            vk::RenderPassBeginInfo renderPassInfo{};
            renderPassInfo.renderPass = m_renderPass;
            renderPassInfo.framebuffer = framebuffer;
            renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
            renderPassInfo.renderArea.extent = m_swapChain->getSwapchainExtent();

            std::array<vk::ClearValue, 2> clearValues{};
            clearValues[0].color = {vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f}};
            clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0}; // depth in [0,1] where 1 is at the far view plane
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

            vkCmdEndRenderPass(commandBuffer);

            commandBuffer.end();
        }

        vk::Semaphore render(std::vector<vk::Semaphore> &awaitBeforeExecution) {
            if (m_gpuContext->m_device.resetFences(1, &m_fences[m_gpuContext->getActiveIndex()]) != vk::Result::eSuccess) { throw std::runtime_error("Failed to reset fences!"); }

            m_commandBuffers[m_gpuContext->getActiveIndex()].reset();
            recordCommandBuffer(m_commandBuffers[m_gpuContext->getActiveIndex()], m_framebuffers[m_swapChain->getNextImageIndex()]);

            vk::SubmitInfo submitInfo{};

            vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eColorAttachmentOutput};
            submitInfo.waitSemaphoreCount = awaitBeforeExecution.size();
            submitInfo.pWaitSemaphores = awaitBeforeExecution.data(); // submit the command buffer after the image is available (signal by the vkAcquireNextImageKHR(...) call)
            submitInfo.pWaitDstStageMask = waitStages;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &m_commandBuffers[m_gpuContext->getActiveIndex()];

            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &m_signalSemaphores[m_gpuContext->getActiveIndex()];

            if (m_gpuContext->m_queues->getQueue(Queues::GRAPHICS).submit(1, &submitInfo, m_fences[m_gpuContext->getActiveIndex()]) != vk::Result::eSuccess) {
                // signal fence after the command buffer finished execution
                throw std::runtime_error("Failed to submit draw command buffer!");
            }

            return m_signalSemaphores[m_gpuContext->getActiveIndex()];
        }

        void awaitLastFrameFinished() {
            if (m_gpuContext->m_device.waitForFences(1, &m_fences[m_gpuContext->getActiveIndex()], VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
                // waiting for the previous frame to finish, blocks the CPU
                throw std::runtime_error("Failed to wait for fences!");
            }
        }

        vk::RenderPass getRenderPass() { return m_renderPass; }

    private:
        GPUContext *m_gpuContext;
        std::shared_ptr<SwapChain> m_swapChain;

        vk::RenderPass m_renderPass{};

        vk::CommandPool m_commandPool{};
        std::vector<vk::CommandBuffer> m_commandBuffers; // destroyed implicitly with the command pool

        // synchronization
        std::vector<vk::Semaphore> m_signalSemaphores;
        std::vector<vk::Fence> m_fences;

        uint32_t m_queueFamilyIndex{};

        std::vector<vk::Framebuffer> m_framebuffers;

        std::shared_ptr<Image> m_depthImage;

        uint32_t findQueueFamilyIndex() {
            Queues::QueueFamilyIndices queueFamilyIndices = m_gpuContext->m_queues->findQueueFamilies(m_gpuContext->m_physicalDevice);
            return queueFamilyIndices.graphicsFamily.value();
        }

        void createCommandPool() {
            vk::CommandPoolCreateInfo poolInfo{};
            poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
            poolInfo.queueFamilyIndex = m_queueFamilyIndex;

            if (m_gpuContext->m_device.createCommandPool(&poolInfo, nullptr, &m_commandPool) != vk::Result::eSuccess) { throw std::runtime_error("Failed to create command pool!"); }
        }

        void createCommandBuffers() {
            m_commandBuffers.resize(m_gpuContext->getMultiBufferedCount());

            vk::CommandBufferAllocateInfo allocInfo{};
            allocInfo.commandPool = m_commandPool;
            allocInfo.level = vk::CommandBufferLevel::ePrimary;
            allocInfo.commandBufferCount = (uint32_t) m_commandBuffers.size();

            if (m_gpuContext->m_device.allocateCommandBuffers(&allocInfo, m_commandBuffers.data()) != vk::Result::eSuccess) { throw std::runtime_error("Failed to allocate command buffers!"); }
        }

        void createSyncObjects() {
            m_signalSemaphores.resize(m_gpuContext->getMultiBufferedCount());
            m_fences.resize(m_gpuContext->getMultiBufferedCount());

            vk::SemaphoreCreateInfo semaphoreInfo{};

            vk::FenceCreateInfo fenceInfo{};
            fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled; // create in signaled state, such that the first vkWaitForFences(..) call immediately returns (there is no previous frame to wait for when starting the application)

            for (size_t i = 0; i < m_gpuContext->getMultiBufferedCount(); i++) {
                if (m_gpuContext->m_device.createSemaphore(&semaphoreInfo, nullptr, &m_signalSemaphores[i]) != vk::Result::eSuccess ||
                    m_gpuContext->m_device.createFence(&fenceInfo, nullptr, &m_fences[i]) != vk::Result::eSuccess) { throw std::runtime_error("Failed to create synchronization objects for a frame!"); }
            }
        }

        void releaseSyncObjects() {
            for (size_t i = 0; i < m_gpuContext->getMultiBufferedCount(); i++) {
                vkDestroySemaphore(m_gpuContext->m_device, m_signalSemaphores[i], nullptr);
                vkDestroyFence(m_gpuContext->m_device, m_fences[i], nullptr);
            }
        }

        void createRenderPass() {
            vk::AttachmentDescription colorAttachment{};
            colorAttachment.format = m_swapChain->getSwapChainImageFormat();
            colorAttachment.samples = vk::SampleCountFlagBits::e1;
            colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;              // clear framebuffer before drawing a new frame (color and depth data)
            colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;           // store rendered results in framebuffer (color and depth data)
            colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;   // (stencil data)
            colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare; // (stencil data)
            colorAttachment.initialLayout = vk::ImageLayout::ePresentSrcKHR;   // do not care which layout the image was in previously
            colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR; // multi sampled image

            vk::AttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal; // function as color buffer, this layout gives the best performance for that

            vk::AttachmentDescription depthAttachment{};
            depthAttachment.format = findDepthFormat();
            depthAttachment.samples = vk::SampleCountFlagBits::e1;
            depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
            depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
            depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
            depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

            vk::AttachmentReference depthAttachmentRef{};
            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

            vk::SubpassDescription subpass{}; // render pass can have multiple subpasses (e.g. multiple postprocessing steps)
            subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;
            subpass.pDepthStencilAttachment = &depthAttachmentRef;

            vk::SubpassDependency dependency{};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask =
                    vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
            dependency.srcAccessMask = vk::AccessFlagBits::eNone;
            dependency.dstStageMask =
                    vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
            dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

            std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
            vk::RenderPassCreateInfo renderPassInfo{};
            renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            renderPassInfo.pAttachments = attachments.data();
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;
            renderPassInfo.dependencyCount = 1;
            renderPassInfo.pDependencies = &dependency;

            if (m_gpuContext->m_device.createRenderPass(&renderPassInfo, nullptr, &m_renderPass) != vk::Result::eSuccess) { throw std::runtime_error("Failed to create render pass!"); }
        }

        void createSwapChainResources() {
            createDepthResources();
            createFramebuffers();
        }

        void releaseSwapChainResources() {
            releaseDepthResources();
            for (auto framebuffer: m_framebuffers) {
                vkDestroyFramebuffer(m_gpuContext->m_device, framebuffer, nullptr);
            }
        }

        void createFramebuffers() {
            m_framebuffers.resize(m_swapChain->getNumSwapChainImageViews());

            for (size_t i = 0; i < m_swapChain->getNumSwapChainImageViews(); i++) {
                std::vector<vk::ImageView> attachments = {
                        m_swapChain->getSwapChainImageView(i),
                        m_depthImage->getImageView()};

                vk::FramebufferCreateInfo framebufferInfo{};
                framebufferInfo.renderPass = m_renderPass;
                framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                framebufferInfo.pAttachments = attachments.data();
                framebufferInfo.width = m_swapChain->getSwapchainExtent().width;
                framebufferInfo.height = m_swapChain->getSwapchainExtent().height;
                framebufferInfo.layers = 1;

                if (m_gpuContext->m_device.createFramebuffer(&framebufferInfo, nullptr, &m_framebuffers[i]) != vk::Result::eSuccess) { throw std::runtime_error("Failed to create framebuffer!"); }
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

        void createDepthResources() {
            vk::Format depthFormat = findDepthFormat();
            auto settings = Image::ImageSettings{.m_width = m_swapChain->getSwapchainExtent().width, .m_height = m_swapChain->getSwapchainExtent().height, .m_sampleCountFlagBits = vk::SampleCountFlagBits::e1, .m_format = depthFormat, .m_tiling = vk::ImageTiling::eOptimal, .m_usageFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment, .m_memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal, .m_aspectFlags = vk::ImageAspectFlagBits::eDepth, .m_name = "depthAttachment"};
            m_depthImage = std::make_shared<Image>(m_gpuContext, settings);
            m_depthImage->create();
        }

        void releaseDepthResources() { m_depthImage->release(); }
    };
} // namespace raven
