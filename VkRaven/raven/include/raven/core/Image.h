#pragma once

#include "Buffer.h"
#include "raven/util/Paths.h"
#include "stb/stb_image.h"
#include <cmath>
#include <stdexcept>
#include <utility>

namespace raven {
#define RAVEN_IMAGE_RELEASE(image) \
    if (image)                     \
    image->release()

    class Image {
    public:
        struct ImageSettings {
            uint32_t m_width = 0;
            uint32_t m_height = 0;

            vk::SampleCountFlagBits m_sampleCountFlagBits = vk::SampleCountFlagBits::e1; // msaa

            vk::Format m_format = vk::Format::eR8G8B8A8Unorm; // eR8G8B8A8Srgb TODO: Unorm is required for normal maps - does this have any consequences for other textures? do we have to change back to Srgb for other textures?
            vk::ImageTiling m_tiling = vk::ImageTiling::eOptimal;
            vk::ImageUsageFlags m_usageFlags = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
            vk::MemoryPropertyFlags m_memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;

            vk::ImageAspectFlags m_aspectFlags = vk::ImageAspectFlagBits::eColor;

            bool m_mipMapping = false;

            std::string m_name = "undefined";

            bool m_exportFlag = false;
        };

        Image(GPUContext *gpuContext, ImageSettings imageSettings) : m_gpuContext(gpuContext), m_imageSettings(std::move(imageSettings)) {
        }

        ~Image() {
            release();
        }

        void release() {
            if (m_imageView) {
                m_gpuContext->m_device.destroyImageView(m_imageView);
            }
            if (m_image) {
                m_gpuContext->m_device.destroyImage(m_image);
            }
            if (m_imageMemory) {
                m_gpuContext->m_device.freeMemory(m_imageMemory);
            }
            if (m_imageSampler) {
                m_gpuContext->m_device.destroySampler(m_imageSampler);
            }
            m_imageView = nullptr;
            m_image = nullptr;
            m_imageMemory = nullptr;
            m_imageSampler = nullptr;
        }

        virtual void create() {
            initImage();
            initImageView();
            createTextureSampler();

            m_gpuContext->getDebug()->setName(m_image, m_imageSettings.m_name);
            m_gpuContext->getDebug()->setName(m_imageMemory, m_imageSettings.m_name);
            m_gpuContext->getDebug()->setName(m_imageView, m_imageSettings.m_name);
            m_gpuContext->getDebug()->setName(m_imageSampler, m_imageSettings.m_name);
        }

        static vk::ImageView createImageView(vk::Device device, vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels) {
            vk::ImageViewCreateInfo viewInfo{};
            viewInfo.image = image;
            viewInfo.viewType = vk::ImageViewType::e2D;
            viewInfo.format = format;
            viewInfo.components.r = vk::ComponentSwizzle::eIdentity; // optional remapping of color channels
            viewInfo.components.g = vk::ComponentSwizzle::eIdentity;
            viewInfo.components.b = vk::ComponentSwizzle::eIdentity;
            viewInfo.components.a = vk::ComponentSwizzle::eIdentity;
            viewInfo.subresourceRange.aspectMask = aspectFlags;
            viewInfo.subresourceRange.baseMipLevel = 0; // no mip mapping
            viewInfo.subresourceRange.levelCount = mipLevels;
            viewInfo.subresourceRange.baseArrayLayer = 0; // one layer only
            viewInfo.subresourceRange.layerCount = 1;

            vk::ImageView imageView;
            if (device.createImageView(&viewInfo, nullptr, &imageView) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create texture image view!");
            }

            return imageView;
        }

        vk::Image getImage() {
            return m_image;
        }

        vk::ImageView getImageView() {
            return m_imageView;
        }

        vk::Sampler getSampler() {
            return m_imageSampler;
        }

        void setImageLayout(vk::ImageLayout layout) {
            transitionImageLayout(layout);
        }

        void download(void *data) {
            Buffer stagingBuffer(m_gpuContext, {m_imageSizeBytes, vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent});

            //            copyBuffer(m_buffer, stagingBuffer.m_buffer, m_bufferSettings.m_sizeBytes, queue, m_device, commandPool); // copy contents from staging buffer to high performance memory on GPU, which cannot be accessed directly by the CPU (therefore the staging buffer)

            vk::BufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = vk::Offset3D{0, 0, 0};
            region.imageExtent = vk::Extent3D{m_imageSettings.m_width, m_imageSettings.m_height, 1};

            m_gpuContext->executeCommands([&](vk::CommandBuffer commandBuffer) {
                commandBuffer.copyImageToBuffer(m_image, m_imageLayout, stagingBuffer.getBuffer(), 1, &region);
            });

            stagingBuffer.download(data);

            stagingBuffer.release();
        }

        [[nodiscard]] uint32_t getSizeBytes() const {
            return m_imageSizeBytes;
        }

        [[nodiscard]] uint32_t getWidth() const {
            return m_imageSettings.m_width;
        }

        [[nodiscard]] uint32_t getHeight() const {
            return m_imageSettings.m_height;
        }

        [[nodiscard]] vk::ImageLayout getLayout() const {
            return m_imageLayout;
        }

        void setLayout(const vk::ImageLayout layout) {
            m_imageLayout = layout;
        }

    protected:
        GPUContext *m_gpuContext;

        ImageSettings m_imageSettings;

        vk::Image m_image{};
        vk::DeviceMemory m_imageMemory{};
        vk::ImageView m_imageView{};
        vk::Sampler m_imageSampler{};

        uint32_t m_imageSizeBytes{};
        vk::ImageLayout m_imageLayout = vk::ImageLayout::eUndefined;

        uint32_t m_mipLevels = 1;

        virtual void initImage() {
            createImage(m_imageSettings.m_width, m_imageSettings.m_height, 1, m_imageSettings.m_sampleCountFlagBits, m_imageSettings.m_format, m_imageSettings.m_tiling, m_imageSettings.m_usageFlags, m_imageSettings.m_memoryPropertyFlags, m_image, m_imageMemory);
        };

        virtual void initImageView() {
            m_imageView = createImageView(m_gpuContext->m_device, m_image, m_imageSettings.m_format, m_imageSettings.m_aspectFlags, 1);
        }

        void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format,
                         vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image &image,
                         vk::DeviceMemory &imageMemory) {
            vk::ImageCreateInfo imageInfo{};
            imageInfo.imageType = vk::ImageType::e2D;
            imageInfo.extent.width = width;
            imageInfo.extent.height = height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = mipLevels;
            imageInfo.arrayLayers = 1;
            imageInfo.format = format;
            imageInfo.tiling = tiling;
            imageInfo.initialLayout = vk::ImageLayout::eUndefined;
            imageInfo.usage = usage;
            imageInfo.samples = numSamples;
            imageInfo.sharingMode = vk::SharingMode::eExclusive;
            vk::ExternalMemoryImageCreateInfo externalMemoryImageCreateInfo{};
#ifdef WIN32
            externalMemoryImageCreateInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
#else
            externalMemoryImageCreateInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd;
#endif
            if (m_imageSettings.m_exportFlag) {
                imageInfo.pNext = &externalMemoryImageCreateInfo;
            }

            if (m_gpuContext->m_device.createImage(&imageInfo, nullptr, &image) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create image!");
            }

            vk::MemoryRequirements memRequirements;
            m_gpuContext->m_device.getImageMemoryRequirements(image, &memRequirements);
            m_imageSizeBytes = memRequirements.size;

            vk::MemoryAllocateInfo allocInfo{};
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
            vk::ExportMemoryAllocateInfo exportAllocInfo{};
#ifdef WIN32
            exportAllocInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
#else
            exportAllocInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd;
#endif
            if (m_imageSettings.m_exportFlag) {
                allocInfo.pNext = &exportAllocInfo;
            }

            if (m_gpuContext->m_device.allocateMemory(&allocInfo, nullptr, &imageMemory) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to allocate image memory!");
            }

            vkBindImageMemory(m_gpuContext->m_device, image, imageMemory, 0);
        }

        void transitionImageLayout(vk::ImageLayout newLayout) {
            vk::ImageMemoryBarrier barrier{};
            barrier.oldLayout = m_imageLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = m_image;
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = m_mipLevels;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            vk::PipelineStageFlagBits sourceStage;
            vk::PipelineStageFlagBits destinationStage;
            if (m_imageLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
                barrier.srcAccessMask = vk::AccessFlagBits::eNone;
                ;
                barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
                sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
                destinationStage = vk::PipelineStageFlagBits::eTransfer;
            } else if (m_imageLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
                sourceStage = vk::PipelineStageFlagBits::eTransfer;
                destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
            } else if (m_imageLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eGeneral) {
                barrier.srcAccessMask = vk::AccessFlagBits::eNone;
                ;
                barrier.dstAccessMask = vk::AccessFlagBits::eNone;
                ;
                sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
                destinationStage = vk::PipelineStageFlagBits::eHost;
            } else if (m_imageLayout == vk::ImageLayout::eGeneral && newLayout == vk::ImageLayout::eTransferSrcOptimal) {
                barrier.srcAccessMask = vk::AccessFlagBits::eNone;
                barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
                sourceStage = vk::PipelineStageFlagBits::eHost;
                destinationStage = vk::PipelineStageFlagBits::eTransfer;
            } else if (m_imageLayout == vk::ImageLayout::eTransferSrcOptimal && newLayout == vk::ImageLayout::eGeneral) {
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
                barrier.dstAccessMask = vk::AccessFlagBits::eNone;
                ;
                sourceStage = vk::PipelineStageFlagBits::eTransfer;
                destinationStage = vk::PipelineStageFlagBits::eHost;
            } else {
                throw std::invalid_argument("Unsupported layout transition!");
            }

            m_gpuContext->executeCommands([&](vk::CommandBuffer commandBuffer) {
                commandBuffer.pipelineBarrier(
                        sourceStage, destinationStage,
                        {},
                        nullptr, nullptr, {barrier});
            });

            m_imageLayout = newLayout;
        }

        void createTextureSampler() {
            vk::PhysicalDeviceProperties properties{};
            m_gpuContext->m_physicalDevice.getProperties(&properties);

            vk::SamplerCreateInfo samplerInfo{};
            samplerInfo.magFilter = vk::Filter::eLinear;
            samplerInfo.minFilter = vk::Filter::eLinear;
            samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
            samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
            samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
            samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
            samplerInfo.unnormalizedCoordinates = VK_FALSE; // access [0, 1), otherwise [0, texDimension)
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = vk::CompareOp::eAlways;
            samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
            samplerInfo.minLod = 0.0f; // optional
            samplerInfo.maxLod = static_cast<float>(m_mipLevels);
            samplerInfo.mipLodBias = 0.0f; // optional

            if (m_gpuContext->m_device.createSampler(&samplerInfo, nullptr, &m_imageSampler) != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create texture sampler!");
            }
        }

    private:
        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
            vk::PhysicalDeviceMemoryProperties memProperties;
            m_gpuContext->m_physicalDevice.getMemoryProperties(&memProperties);

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }

            throw std::runtime_error("Failed to find suitable memory type!");
        }
    };
} // namespace raven