// taken from https://github.com/nvpro-samples/vk_denoise/tree/master, adjusted
#ifdef OPTIX_SUPPORT

#include "denoiser/DenoiserOptiX.h"

#include <optix.h>
#include <optix_function_table_definition.h>
#include <optix_host.h>
#include <optix_stubs.h>

namespace raven {
    DenoiserOptiX::DenoiserOptiX(GPUContext *gpuContext) : m_gpuContext(gpuContext) {
    }

    void DenoiserOptiX::create(const OptixDenoiserOptions &options, const OptixPixelFormat pixelFormat, const vk::Extent2D &imgSize, const vk::Semaphore &semaphore) {
        initOptiX(options, pixelFormat);
        createSemaphore();
        linkWaitSemaphore(semaphore);
        allocateBuffers(imgSize);
    }

    void DenoiserOptiX::release() {
        m_gpuContext->m_device.destroySemaphore(m_timelineSemaphore.m_vulkan, nullptr);

        optixDenoiserDestroy(m_denoiser);
        optixDeviceContextDestroy(m_optixDevice);

        destroyBuffers();

        for (auto &p: m_pixelBufferIn) {
            p.destroy();
        }
        m_pixelBufferOut.destroy();
    }

    bool DenoiserOptiX::initOptiX(const OptixDenoiserOptions &options, OptixPixelFormat pixelFormat) {
        CUDA_CHECK(cudaFree(nullptr));

        CUcontext cu_ctx = nullptr;
        OPTIX_CHECK(optixInit());

        OptixDeviceContextOptions optixoptions = {};
        optixoptions.logCallbackFunction = &contextLogCb;
        optixoptions.logCallbackLevel = 3;

        OPTIX_CHECK(optixDeviceContextCreate(cu_ctx, &optixoptions, &m_optixDevice));
        OPTIX_CHECK(optixDeviceContextSetLogCallback(m_optixDevice, contextLogCb, nullptr, 3));

        m_pixelFormat = pixelFormat;
        switch (pixelFormat) {
            case OPTIX_PIXEL_FORMAT_FLOAT3:
                m_sizeofPixel = static_cast<uint32_t>(3 * sizeof(float));
                m_denoiserAlpha = OPTIX_DENOISER_ALPHA_MODE_COPY;
                break;
            case OPTIX_PIXEL_FORMAT_FLOAT4:
                m_sizeofPixel = static_cast<uint32_t>(4 * sizeof(float));
#if OPTIX_VERSION == 80000
                m_denoiserAlpha = OPTIX_DENOISER_ALPHA_MODE_DENOISE;
#else
                m_denoiserAlpha = OPTIX_DENOISER_ALPHA_MODE_ALPHA_AS_AOV;
#endif
                break;
            case OPTIX_PIXEL_FORMAT_UCHAR3:
                m_sizeofPixel = static_cast<uint32_t>(3 * sizeof(uint8_t));
                m_denoiserAlpha = OPTIX_DENOISER_ALPHA_MODE_COPY;
                break;
            case OPTIX_PIXEL_FORMAT_UCHAR4:
                m_sizeofPixel = static_cast<uint32_t>(4 * sizeof(uint8_t));
#if OPTIX_VERSION == 80000
                m_denoiserAlpha = OPTIX_DENOISER_ALPHA_MODE_DENOISE;
#else
                m_denoiserAlpha = OPTIX_DENOISER_ALPHA_MODE_ALPHA_AS_AOV;
#endif
                break;
            case OPTIX_PIXEL_FORMAT_HALF3:
                m_sizeofPixel = static_cast<uint32_t>(3 * sizeof(uint16_t));
                m_denoiserAlpha = OPTIX_DENOISER_ALPHA_MODE_COPY;
                break;
            case OPTIX_PIXEL_FORMAT_HALF4:
                m_sizeofPixel = static_cast<uint32_t>(4 * sizeof(uint16_t));
#if OPTIX_VERSION == 80000
                m_denoiserAlpha = OPTIX_DENOISER_ALPHA_MODE_DENOISE;
#else
                m_denoiserAlpha = OPTIX_DENOISER_ALPHA_MODE_ALPHA_AS_AOV;
#endif
                break;
            default:
                assert(!"unsupported");
        }

        // This is to use RGB + Albedo + Normal
        m_denoiserOptions = options;
        OptixDenoiserModelKind modelKind = OPTIX_DENOISER_MODEL_KIND_AOV;
        OPTIX_CHECK(optixDenoiserCreate(m_optixDevice, modelKind, &m_denoiserOptions, &m_denoiser));

        return true;
    }

    void DenoiserOptiX::denoiseImageBuffer(uint64_t waitTimelineSemaphoreValue, float blendFactor) {
        if (m_timelineSemaphore.m_vulkan == nullptr) {
            throw std::runtime_error("Timeline semaphore not created");
        }
        if (m_waitTimelineSemaphore.m_cuda == nullptr) {
            throw std::runtime_error("Wait timeline semaphore not linked");
        }

        try {
            OptixPixelFormat pixel_format = m_pixelFormat;
            auto sizeof_pixel = m_sizeofPixel;
            uint32_t row_stride_in_bytes = sizeof_pixel * m_imageSize.width;

            OptixDenoiserLayer layer = {};
            // Input
            layer.input.data = reinterpret_cast<CUdeviceptr>(m_pixelBufferIn[0].cudaPtr);
            layer.input.width = m_imageSize.width;
            layer.input.height = m_imageSize.height;
            layer.input.rowStrideInBytes = row_stride_in_bytes;
            layer.input.pixelStrideInBytes = m_sizeofPixel;
            layer.input.format = pixel_format;

            // Output
            layer.output.data = reinterpret_cast<CUdeviceptr>(m_pixelBufferOut.cudaPtr);
            layer.output.width = m_imageSize.width;
            layer.output.height = m_imageSize.height;
            layer.output.rowStrideInBytes = row_stride_in_bytes;
            layer.output.pixelStrideInBytes = sizeof(float) * 4;
            layer.output.format = pixel_format;


            OptixDenoiserGuideLayer guide_layer = {};
            // albedo
            if (m_denoiserOptions.guideAlbedo != 0u) {
                guide_layer.albedo.data = reinterpret_cast<CUdeviceptr>(m_pixelBufferIn[1].cudaPtr);
                guide_layer.albedo.width = m_imageSize.width;
                guide_layer.albedo.height = m_imageSize.height;
                guide_layer.albedo.rowStrideInBytes = row_stride_in_bytes;
                guide_layer.albedo.pixelStrideInBytes = m_sizeofPixel;
                guide_layer.albedo.format = pixel_format;
            }

            // normal
            if (m_denoiserOptions.guideNormal != 0u) {
                guide_layer.normal.data = reinterpret_cast<CUdeviceptr>(m_pixelBufferIn[2].cudaPtr);
                guide_layer.normal.width = m_imageSize.width;
                guide_layer.normal.height = m_imageSize.height;
                guide_layer.normal.rowStrideInBytes = row_stride_in_bytes;
                guide_layer.normal.pixelStrideInBytes = m_sizeofPixel;
                guide_layer.normal.format = pixel_format;
            }

            cudaExternalSemaphoreWaitParams wait_params{};
            wait_params.flags = 0;
            wait_params.params.fence.value = waitTimelineSemaphoreValue;
            cudaWaitExternalSemaphoresAsync(&m_waitTimelineSemaphore.m_cuda, &wait_params, 1, nullptr);

            if (m_dIntensity != 0) {
                OPTIX_CHECK(optixDenoiserComputeIntensity(m_denoiser, m_cuStream, &layer.input, m_dIntensity, m_dScratchBuffer,
                                                          m_denoiserSizes.withoutOverlapScratchSizeInBytes));
            }

            OptixDenoiserParams denoiser_params{};
#if OPTIX_VERSION < 80000
            denoiser_params.denoiseAlpha = m_denoiserAlpha;
#endif
            denoiser_params.hdrIntensity = m_dIntensity;
            denoiser_params.blendFactor = blendFactor;

            OPTIX_CHECK(optixDenoiserInvoke(m_denoiser, m_cuStream, &denoiser_params, m_dStateBuffer,
                                            m_denoiserSizes.stateSizeInBytes, &guide_layer, &layer, 1, 0, 0, m_dScratchBuffer,
                                            m_denoiserSizes.withoutOverlapScratchSizeInBytes));


            CUDA_CHECK(cudaDeviceSynchronize());
            CUDA_CHECK(cudaStreamSynchronize(m_cuStream));

            m_timelineSemaphore.m_value++;
            cudaExternalSemaphoreSignalParams sig_params{};
            sig_params.flags = 0;
            sig_params.params.fence.value = m_timelineSemaphore.m_value;
            cudaSignalExternalSemaphoresAsync(&m_timelineSemaphore.m_cuda, &sig_params, 1, m_cuStream);
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
        }
    }

    void DenoiserOptiX::imageToBuffer(const vk::CommandBuffer &cmdBuf, const std::vector<std::shared_ptr<Image>> &imgIn) const {
        vk::BufferImageCopy region{};
        region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        region.imageSubresource.layerCount = 1;
        region.imageExtent.width = m_imageSize.width;
        region.imageExtent.height = m_imageSize.height;
        region.imageExtent.depth = 1;

        for (int i = 0; i < static_cast<int>(imgIn.size()); i++) {
            {
                vk::ImageMemoryBarrier barrier{};
                barrier.oldLayout = vk::ImageLayout::eGeneral;
                barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = imgIn[i]->getImage();
                barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;
                barrier.srcAccessMask = vk::AccessFlagBits::eNone;
                barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
                constexpr auto sourceStage = vk::PipelineStageFlagBits::eHost;
                constexpr auto destinationStage = vk::PipelineStageFlagBits::eTransfer;
                cmdBuf.pipelineBarrier(
                        sourceStage, destinationStage,
                        {},
                        nullptr, nullptr, {barrier});
            }
            cmdBuf.copyImageToBuffer(imgIn[i]->getImage(), vk::ImageLayout::eTransferSrcOptimal, m_pixelBufferIn[i].bufVk->getBuffer(), 1, &region);
            {
                vk::ImageMemoryBarrier barrier{};
                barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
                barrier.newLayout = vk::ImageLayout::eGeneral;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = imgIn[i]->getImage();
                barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
                barrier.dstAccessMask = vk::AccessFlagBits::eNone;
                constexpr auto sourceStage = vk::PipelineStageFlagBits::eTransfer;
                constexpr auto destinationStage = vk::PipelineStageFlagBits::eHost;
                cmdBuf.pipelineBarrier(
                        sourceStage, destinationStage,
                        {},
                        nullptr, nullptr, {barrier});
            }
        }
    }

    void DenoiserOptiX::bufferToImage(const vk::CommandBuffer &cmdBuf, const std::shared_ptr<Image> &imgOut) const {
        vk::BufferImageCopy region{};
        region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        region.imageSubresource.layerCount = 1;
        region.imageExtent.width = m_imageSize.width;
        region.imageExtent.height = m_imageSize.height;
        region.imageExtent.depth = 1;

        {
            vk::ImageMemoryBarrier barrier{};
            barrier.oldLayout = vk::ImageLayout::eGeneral;
            barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = imgOut->getImage();
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.srcAccessMask = vk::AccessFlagBits::eNone;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
            constexpr auto sourceStage = vk::PipelineStageFlagBits::eHost;
            constexpr auto destinationStage = vk::PipelineStageFlagBits::eTransfer;
            cmdBuf.pipelineBarrier(
                    sourceStage, destinationStage,
                    {},
                    nullptr, nullptr, {barrier});
        }
        cmdBuf.copyBufferToImage(m_pixelBufferOut.bufVk->getBuffer(), imgOut->getImage(), vk::ImageLayout::eTransferDstOptimal, 1, &region);
        {
            vk::ImageMemoryBarrier barrier{};
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::eGeneral;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = imgOut->getImage();
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eNone;
            constexpr auto sourceStage = vk::PipelineStageFlagBits::eTransfer;
            constexpr auto destinationStage = vk::PipelineStageFlagBits::eHost;
            cmdBuf.pipelineBarrier(
                    sourceStage, destinationStage,
                    {},
                    nullptr, nullptr, {barrier});
        }
    }

    void DenoiserOptiX::destroyBuffers() {
        if (m_dStateBuffer != 0) {
            CUDA_CHECK(cudaFree(reinterpret_cast<void *>(m_dStateBuffer)));
            m_dStateBuffer = 0;
        }
        if (m_dScratchBuffer != 0) {
            CUDA_CHECK(cudaFree(reinterpret_cast<void *>(m_dScratchBuffer)));
            m_dScratchBuffer = 0;
        }
        if (m_dIntensity != 0) {
            CUDA_CHECK(cudaFree(reinterpret_cast<void *>(m_dIntensity)));
            m_dIntensity = 0;
        }
        if (m_dMinRGB != 0) {
            CUDA_CHECK(cudaFree(reinterpret_cast<void *>(m_dMinRGB)));
            m_dMinRGB = 0;
        }
    }

    void DenoiserOptiX::allocateBuffers(const vk::Extent2D &imgSize) {
        m_imageSize = imgSize;

        auto settings = Buffer::BufferSettings{
                .m_sizeBytes = static_cast<uint32_t>(m_imageSize.width * m_imageSize.height * 4 * sizeof(float)),
                .m_bufferUsages = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc,
                .m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
                .m_exportFlag = true};

        {
            settings.m_name = "OptiX_color";
            m_pixelBufferIn[0].bufVk = std::make_shared<Buffer>(m_gpuContext, settings);
            createBufferCuda(m_pixelBufferIn[0]); // Exporting the buffer to Cuda handle and pointers
        }

        {
            settings.m_name = "OptiX_albedo";
            m_pixelBufferIn[1].bufVk = std::make_shared<Buffer>(m_gpuContext, settings);
            createBufferCuda(m_pixelBufferIn[1]);
        }

        {
            settings.m_name = "OptiX_normal";
            m_pixelBufferIn[2].bufVk = std::make_shared<Buffer>(m_gpuContext, settings);
            createBufferCuda(m_pixelBufferIn[2]);
        }

        settings.m_name = "OptiX_output";
        m_pixelBufferOut.bufVk = std::make_shared<Buffer>(m_gpuContext, settings);
        createBufferCuda(m_pixelBufferOut);

        OPTIX_CHECK(optixDenoiserComputeMemoryResources(m_denoiser, m_imageSize.width, m_imageSize.height, &m_denoiserSizes));

        CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&m_dStateBuffer), m_denoiserSizes.stateSizeInBytes));
        CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&m_dScratchBuffer), m_denoiserSizes.withoutOverlapScratchSizeInBytes));
        CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&m_dMinRGB), 4 * sizeof(float)));
        if (m_pixelFormat == OPTIX_PIXEL_FORMAT_FLOAT3 || m_pixelFormat == OPTIX_PIXEL_FORMAT_FLOAT4)
            CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&m_dIntensity), sizeof(float)));

        OPTIX_CHECK(optixDenoiserSetup(m_denoiser, m_cuStream, m_imageSize.width, m_imageSize.height, m_dStateBuffer,
                                       m_denoiserSizes.stateSizeInBytes, m_dScratchBuffer, m_denoiserSizes.withoutOverlapScratchSizeInBytes));
    }

    void DenoiserOptiX::createBufferCuda(BufferCuda &buf) const {
#ifdef WIN32
        vk::MemoryGetWin32HandleInfoKHR info;
        info.memory = buf.bufVk->getDeviceMemory();
        info.handleType = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
        if (m_gpuContext->m_device.getMemoryWin32HandleKHR(&info, &buf.handle) != vk::Result::eSuccess) {
            assert(0 && "Error: failed to get memory fd");
        }
#else
        vk::MemoryGetFdInfoKHR info;
        info.memory = buf.bufVk->getDeviceMemory();
        info.handleType = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd;
        if (m_gpuContext->m_device.getMemoryFdKHR(&info, &buf.handle) != vk::Result::eSuccess) {
            assert(0 && "Error: failed to get memory fd");
        }
#endif

        vk::MemoryRequirements memory_req{};
        m_gpuContext->m_device.getBufferMemoryRequirements(buf.bufVk->getBuffer(), &memory_req);

        cudaExternalMemoryHandleDesc cuda_ext_mem_handle_desc{};
        cuda_ext_mem_handle_desc.size = memory_req.size;
#ifdef WIN32
        cuda_ext_mem_handle_desc.type = cudaExternalMemoryHandleTypeOpaqueWin32;
        cuda_ext_mem_handle_desc.handle.win32.handle = buf.handle;
#else
        cuda_ext_mem_handle_desc.type = cudaExternalMemoryHandleTypeOpaqueFd;
        cuda_ext_mem_handle_desc.handle.fd = buf.handle;
#endif

        cudaExternalMemory_t cuda_ext_mem_vertex_buffer{};
        CUDA_CHECK(cudaImportExternalMemory(&cuda_ext_mem_vertex_buffer, &cuda_ext_mem_handle_desc));

#ifndef WIN32
        cuda_ext_mem_handle_desc.handle.fd = -1;
#endif

        cudaExternalMemoryBufferDesc cuda_ext_buffer_desc{};
        cuda_ext_buffer_desc.offset = 0;
        cuda_ext_buffer_desc.size = memory_req.size;
        cuda_ext_buffer_desc.flags = 0;
        CUDA_CHECK(cudaExternalMemoryGetMappedBuffer(&buf.cudaPtr, cuda_ext_mem_vertex_buffer, &cuda_ext_buffer_desc));
    }

    void DenoiserOptiX::createSemaphore() {
        vk::SemaphoreTypeCreateInfo timelineCreateInfo{};
        timelineCreateInfo.semaphoreType = vk::SemaphoreType::eTimeline;
        timelineCreateInfo.initialValue = 0;

        vk::ExportSemaphoreCreateInfo esci{};
#ifdef WIN32
        auto handleType = vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32;
#else
        auto handleType = vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueFd;
#endif
        esci.handleTypes = handleType;
        timelineCreateInfo.pNext = &esci;

        vk::SemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.pNext = &timelineCreateInfo;

        if (m_gpuContext->m_device.createSemaphore(&semaphoreInfo, nullptr, &m_timelineSemaphore.m_vulkan) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to create timeline semaphore!");
        }

#ifdef WIN32
        vk::SemaphoreGetWin32HandleInfoKHR handleInfo;
        handleInfo.handleType = handleType;
        handleInfo.semaphore = m_timelineSemaphore.m_vulkan;
        if (m_gpuContext->m_device.getSemaphoreWin32HandleKHR(&handleInfo, &m_timelineSemaphore.m_handle) != vk::Result::eSuccess) {
            assert(0 && "Error: failed to get semaphore fd");
        }
#else
        vk::SemaphoreGetFdInfoKHR handleInfo;
        handleInfo.handleType = handleType;
        handleInfo.semaphore = m_timelineSemaphore.m_vulkan;
        if (m_gpuContext->m_device.getSemaphoreFdKHR(&handleInfo, &m_timelineSemaphore.m_handle) != vk::Result::eSuccess) {
            assert(0 && "Error: failed to get semaphore fd");
        }
#endif

        cudaExternalSemaphoreHandleDesc externalSemaphoreHandleDesc{};
        std::memset(&externalSemaphoreHandleDesc, 0, sizeof(externalSemaphoreHandleDesc));
        externalSemaphoreHandleDesc.flags = 0;
#ifdef WIN32
        externalSemaphoreHandleDesc.type = cudaExternalSemaphoreHandleTypeTimelineSemaphoreWin32;
        externalSemaphoreHandleDesc.handle.win32.handle = static_cast<void *>(m_timelineSemaphore.m_handle);
#else
        externalSemaphoreHandleDesc.type = cudaExternalSemaphoreHandleTypeTimelineSemaphoreFd;
        externalSemaphoreHandleDesc.handle.fd = m_timelineSemaphore.m_handle;
#endif

        CUDA_CHECK(cudaImportExternalSemaphore(&m_timelineSemaphore.m_cuda, &externalSemaphoreHandleDesc));
    }

    void DenoiserOptiX::linkWaitSemaphore(const vk::Semaphore &semaphore) {
#ifdef WIN32
        auto handleType = vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32;
#else
        auto handleType = vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueFd;
#endif

#ifdef WIN32
        vk::SemaphoreGetWin32HandleInfoKHR handleInfo;
        handleInfo.handleType = handleType;
        handleInfo.semaphore = semaphore;
        if (m_gpuContext->m_device.getSemaphoreWin32HandleKHR(&handleInfo, &m_waitTimelineSemaphore.m_handle) != vk::Result::eSuccess) {
            assert(0 && "Error: failed to get semaphore fd");
        }
#else
        vk::SemaphoreGetFdInfoKHR handleInfo;
        handleInfo.handleType = handleType;
        handleInfo.semaphore = semaphore;
        if (m_gpuContext->m_device.getSemaphoreFdKHR(&handleInfo, &m_waitTimelineSemaphore.m_handle) != vk::Result::eSuccess) {
            assert(0 && "Error: failed to get semaphore fd");
        }
#endif

        cudaExternalSemaphoreHandleDesc externalSemaphoreHandleDesc{};
        std::memset(&externalSemaphoreHandleDesc, 0, sizeof(externalSemaphoreHandleDesc));
        externalSemaphoreHandleDesc.flags = 0;
#ifdef WIN32
        externalSemaphoreHandleDesc.type = cudaExternalSemaphoreHandleTypeTimelineSemaphoreWin32;
        externalSemaphoreHandleDesc.handle.win32.handle = static_cast<void *>(m_timelineSemaphore.m_handle);
#else
        externalSemaphoreHandleDesc.type = cudaExternalSemaphoreHandleTypeTimelineSemaphoreFd;
        externalSemaphoreHandleDesc.handle.fd = m_waitTimelineSemaphore.m_handle;
#endif

        CUDA_CHECK(cudaImportExternalSemaphore(&m_waitTimelineSemaphore.m_cuda, &externalSemaphoreHandleDesc));
    }
} // namespace raven

#endif
