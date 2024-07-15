#pragma once

#include <utility>

#include "Image.h"

namespace raven {
    class Texture : public Image {
    public:
        Texture(GPUContext *gpuContext, ImageSettings imageSettings, std::string path) : Image(gpuContext, std::move(imageSettings)), m_path(std::move(path)) {
        }

        ~Texture() {
            Texture::release();
        }

        void create() override {
            Image::create();
        }

    protected:
        void initImage() override {
            createTextureImage(m_path);
        }

        void initImageView() override {
            m_imageView = createImageView(m_gpuContext->m_device, m_image, m_imageSettings.m_format, vk::ImageAspectFlagBits::eColor, m_mipLevels);
        }

    private:
        std::string m_path;

        void createTextureImage(const std::string &path) {
            int texWidth, texHeight, texChannels;
            stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            m_imageSettings.m_width = texWidth;
            m_imageSettings.m_height = texHeight;
            vk::DeviceSize imageSize = texWidth * texHeight * 4;

            if (!pixels) {
                throw std::runtime_error("Failed to load texture image!");
            }

            if (m_imageSettings.m_mipMapping) {
                m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
            }

            Buffer stagingBuffer(m_gpuContext, {static_cast<uint32_t>(imageSize), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent});
            stagingBuffer.updateHostMemory(imageSize, pixels);


            stbi_image_free(pixels);

            createImage(m_imageSettings.m_width, m_imageSettings.m_height, m_mipLevels, vk::SampleCountFlagBits::e1, m_imageSettings.m_format,
                        m_imageSettings.m_tiling, m_imageSettings.m_usageFlags, m_imageSettings.m_memoryPropertyFlags, m_image, m_imageMemory);
            transitionImageLayout(vk::ImageLayout::eTransferDstOptimal);
            copyBufferToImage(stagingBuffer.getBuffer(), m_image, static_cast<uint32_t>(texWidth),
                              static_cast<uint32_t>(texHeight));
            if (m_imageSettings.m_mipMapping) {
                generateMipmaps(m_image, m_imageSettings.m_format, texWidth, texHeight, m_mipLevels);
            } else {
                transitionImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
            }

            stagingBuffer.release();
        }

        void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) {
            vk::BufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = vk::Offset3D{0, 0, 0};
            region.imageExtent = vk::Extent3D{
                    width,
                    height,
                    1};

            m_gpuContext->executeCommands([&](vk::CommandBuffer commandBuffer) {
                commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
            });
        }

        void generateMipmaps(vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
            vk::FormatProperties formatProperties;
            m_gpuContext->m_physicalDevice.getFormatProperties(imageFormat, &formatProperties);
            if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
                throw std::runtime_error("Texture image format does not support linear blitting!");
            }

            m_gpuContext->executeCommands([&](vk::CommandBuffer commandBuffer) {
                vk::ImageMemoryBarrier barrier{};
                barrier.image = image;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;
                barrier.subresourceRange.levelCount = 1;

                int32_t mipWidth = texWidth;
                int32_t mipHeight = texHeight;

                for (uint32_t i = 1; i < mipLevels; i++) {
                    barrier.subresourceRange.baseMipLevel = i - 1;
                    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
                    barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
                    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                    barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

                    commandBuffer.pipelineBarrier(
                            vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {},
                            nullptr,
                            nullptr,
                            {barrier});

                    vk::ImageBlit blit{};
                    blit.srcOffsets[0] = vk::Offset3D{0, 0, 0};
                    blit.srcOffsets[1] = vk::Offset3D{mipWidth, mipHeight, 1};
                    blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                    blit.srcSubresource.mipLevel = i - 1;
                    blit.srcSubresource.baseArrayLayer = 0;
                    blit.srcSubresource.layerCount = 1;
                    blit.dstOffsets[0] = vk::Offset3D{0, 0, 0};
                    blit.dstOffsets[1] = vk::Offset3D{mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
                    blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                    blit.dstSubresource.mipLevel = i;
                    blit.dstSubresource.baseArrayLayer = 0;
                    blit.dstSubresource.layerCount = 1;

                    commandBuffer.blitImage(
                            image, vk::ImageLayout::eTransferSrcOptimal,
                            image, vk::ImageLayout::eTransferDstOptimal,
                            1, &blit,
                            vk::Filter::eLinear);

                    barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
                    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                    barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
                    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

                    commandBuffer.pipelineBarrier(
                            vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                            {},
                            nullptr,
                            nullptr,
                            {barrier});

                    if (mipWidth > 1) {
                        mipWidth /= 2;
                    }
                    if (mipHeight > 1) {
                        mipHeight /= 2;
                    }
                }

                barrier.subresourceRange.baseMipLevel = mipLevels - 1;
                barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
                barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

                commandBuffer.pipelineBarrier(
                        vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                        {},
                        nullptr, nullptr, {barrier});
            }, Queues::GRAPHICS);

            m_imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        }
    };
} // namespace raven