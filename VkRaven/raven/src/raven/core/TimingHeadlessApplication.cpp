#include "raven/core/TimingHeadlessApplication.h"

#include <fstream>
#include <tinygltf/stb_image_write.h>
#include <utility>

namespace raven {
    TimingHeadlessApplication::TimingHeadlessApplication(std::string &appName, ApplicationSettings appSettings, TimingApplicationSettings timingAppSettings, std::shared_ptr<Renderer> renderer, uint32_t requiredQueueFamilies)
        : HeadlessApplication(appName, std::move(appSettings), std::move(renderer), requiredQueueFamilies), m_timingApplicationSettings(std::move(timingAppSettings)) {}

    void TimingHeadlessApplication::run() {
        for (m_timingExecution = 0; m_timingExecution < m_timingApplicationSettings.m_timingExecutions; m_timingExecution++) {
            HeadlessApplication::run();
        }
    }

    void TimingHeadlessApplication::loop() {
        for (uint32_t execution = 0; execution < m_timingApplicationSettings.m_startupExecutions; execution++) {
            render();

            const float progress = static_cast<float>(execution + 1) / static_cast<float>(m_timingApplicationSettings.m_startupExecutions);
            constexpr int barWidth = 70;

            std::cout << "[";
            const int pos = static_cast<int>(barWidth * progress);
            for (int i = 0; i < barWidth; i++) {
                if (i < pos) {
                    std::cout << "=";
                } else if (i == pos) {
                    std::cout << ">";
                } else {
                    std::cout << " ";
                }
            }
            std::cout << "] " << execution + 1 << "/" << m_timingApplicationSettings.m_startupExecutions << " " << static_cast<int>(progress * 100.0) << " %\r";
            std::cout << std::flush;
        }
        std::cout << std::endl;

        std::cout << "Startup executions finished." << std::endl;

        m_timingApplicationSettings.m_executeAfterStartupExecutions(this);

        for (uint32_t execution = 0; execution < m_applicationSettings.m_rendererExecutions; execution++) {
            getCamera()->updateTrajectory();
            render();
            m_timingApplicationSettings.m_executeEachFrame(execution);

            const float progress = static_cast<float>(execution + 1) / static_cast<float>(m_applicationSettings.m_rendererExecutions);
            constexpr int barWidth = 70;

            std::cout << "[";
            const int pos = static_cast<int>(barWidth * progress);
            for (int i = 0; i < barWidth; i++) {
                if (i < pos) {
                    std::cout << "=";
                } else if (i == pos) {
                    std::cout << ">";
                } else {
                    std::cout << " ";
                }
            }
            std::cout << "] " << execution + 1 << "/" << m_applicationSettings.m_rendererExecutions << " " << static_cast<int>(progress * 100.0) << " %\r";
            std::cout << std::flush;
        }
        std::cout << std::endl;

        std::cout << "Rendering finished." << std::endl;

        m_device.waitIdle();
    }

    void TimingHeadlessApplication::finish() {
        std::ofstream stream;
        stream.open(m_applicationSettings.m_directory + "/" + m_applicationSettings.m_name + ".txt", std::ios_base::app);
        m_timingApplicationSettings.m_executeQueryTiming(stream);
        stream.close();

        if (m_timingExecution == 0) {
            std::cout << prefix() << "Rendering finished." << std::endl;

            if (!m_image) {
                std::cout << prefix() << "No image rendered." << std::endl;
                return;
            }

            const std::string path = m_applicationSettings.m_directory + "/" + m_applicationSettings.m_name + ".png";

            std::cout << prefix() << "Saving image to disk..." << std::endl;

            m_device.waitIdle();

            {
                // create target image
                auto settings = Image::ImageSettings{.m_width = m_image->getWidth(), .m_height = m_image->getHeight(), .m_format = vk::Format::eR8G8B8A8Unorm, .m_tiling = vk::ImageTiling::eOptimal, .m_usageFlags = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage, .m_name = "screenshotBuffer"};
                auto screenshotImage = std::make_shared<Image>(static_cast<GPUContext *>(this), settings);
                screenshotImage->create();

                const auto priorLayout = m_image->getLayout();

                executeCommands([this, screenshotImage, priorLayout](vk::CommandBuffer commandBuffer) {
                    {
                        vk::ImageMemoryBarrier barrier{};
                        barrier.oldLayout = priorLayout;
                        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
                        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        barrier.image = m_image->getImage();
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
                        barrier.oldLayout = screenshotImage->getLayout();
                        barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
                        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        barrier.image = screenshotImage->getImage();
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
                        copyRegion.srcOffsets[1] = vk::Offset3D{static_cast<int32_t>(m_image->getWidth()), static_cast<int32_t>(m_image->getHeight()), 1};
                        copyRegion.dstOffsets[0] = vk::Offset3D{0, 0, 0};
                        copyRegion.dstOffsets[1] = vk::Offset3D{static_cast<int32_t>(m_image->getWidth()), static_cast<int32_t>(m_image->getHeight()), 1};
                        vk::Filter filter = vk::Filter::eNearest;
                        commandBuffer.blitImage(m_image->getImage(), vk::ImageLayout::eTransferSrcOptimal, screenshotImage->getImage(), vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion, filter);
                    }

                    {
                        vk::ImageMemoryBarrier barrier{};
                        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
                        barrier.newLayout = vk::ImageLayout::eGeneral;
                        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        barrier.image = screenshotImage->getImage();
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
                        screenshotImage->setLayout(vk::ImageLayout::eGeneral);
                    }
                    {
                        vk::ImageMemoryBarrier barrier{};
                        barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
                        barrier.newLayout = priorLayout;
                        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        barrier.image = m_image->getImage();
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
                });

//                vk::ImageSubresource subResource{vk::ImageAspectFlagBits::eColor, 0, 0};
//                vk::SubresourceLayout subResourceLayout;
//                m_device.getImageSubresourceLayout(screenshotImage->getImage(), &subResource, &subResourceLayout);

                std::vector<uint8_t> bytes(screenshotImage->getSizeBytes());
                screenshotImage->download(bytes.data());

                const uint8_t *data = bytes.data();
//                data += subResourceLayout.offset;

                stbi_write_png(path.c_str(), static_cast<int>(m_image->getWidth()), static_cast<int>(m_image->getHeight()), 4, data, static_cast<int>(m_image->getWidth()) * 4);
            }

            m_device.waitIdle();

            std::cout << prefix() << "Image saved: " << path << std::endl;
        }
    }

    void TimingHeadlessApplication::render() {
        Renderer::RendererResult rendererResult;
        m_renderer->render(this, m_camera.get(), getActiveIndex(), &rendererResult);
        m_image = rendererResult.m_image;

        incrementActiveIndex();
    }
} // namespace raven