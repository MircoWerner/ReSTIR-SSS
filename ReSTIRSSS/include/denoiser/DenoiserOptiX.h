// taken from https://github.com/nvpro-samples/vk_denoise/tree/master, adjusted
#pragma once
#ifdef OPTIX_SUPPORT

#include "raven/core/Buffer.h"
#include "raven/core/GPUContext.h"
#include "raven/core/Image.h"

#include <array>
#include <iomanip>
#include <iostream>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <cuda.h>
#include <cuda_runtime.h>

#include "optix_types.h"
#include <driver_types.h>

namespace raven {

#define OPTIX_CHECK(call)                                                                                                \
    do {                                                                                                                 \
        OptixResult res = call;                                                                                          \
        if (res != OPTIX_SUCCESS) {                                                                                      \
            std::stringstream ss;                                                                                        \
            ss << "Optix call (" << #call << " ) failed with code " << res << " (" __FILE__ << ":" << __LINE__ << ")\n"; \
            std::cerr << ss.str().c_str() << std::endl;                                                                  \
            throw std::runtime_error(ss.str().c_str());                                                                  \
        }                                                                                                                \
    } while (false)

#define CUDA_CHECK(call)                                                                                                  \
    do {                                                                                                                  \
        cudaError_t error = call;                                                                                         \
        if (error != cudaSuccess) {                                                                                       \
            std::stringstream ss;                                                                                         \
            ss << "CUDA call (" << #call << " ) failed with code " << error << " (" __FILE__ << ":" << __LINE__ << ")\n"; \
            throw std::runtime_error(ss.str().c_str());                                                                   \
        }                                                                                                                 \
    } while (false)

#define OPTIX_CHECK_LOG(call)                                                                                                 \
    do {                                                                                                                      \
        OptixResult res = call;                                                                                               \
        if (res != OPTIX_SUCCESS) {                                                                                           \
            std::stringstream ss;                                                                                             \
            ss << "Optix call (" << #call << " ) failed with code " << res << " (" __FILE__ << ":" << __LINE__ << ")\nLog:\n" \
               << log << "\n";                                                                                                \
            throw std::runtime_error(ss.str().c_str());                                                                       \
        }                                                                                                                     \
    } while (false)

    static void contextLogCb(unsigned int level, const char *tag, const char *message, void * /*cbdata */) {
        std::cerr << "[" << std::setw(2) << level << "][" << std::setw(12) << tag << "]: " << message << "\n";
    }

    class DenoiserOptiX {
    public:
        explicit DenoiserOptiX(GPUContext *gpuContext);
        ~DenoiserOptiX() = default;

        void create(const OptixDenoiserOptions &options, OptixPixelFormat pixelFormat, const vk::Extent2D &imgSize, const vk::Semaphore &semaphore);
        void release();

        void bufferToImage(const vk::CommandBuffer &cmdBuf, const std::shared_ptr<Image> &imgOut) const;
        void imageToBuffer(const vk::CommandBuffer &cmdBuf, const std::vector<std::shared_ptr<Image>> &imgIn) const;
        void denoiseImageBuffer(uint64_t waitTimelineSemaphoreValue, float blendFactor = 0.0f);

        [[nodiscard]] vk::Semaphore getTimelineSemaphore() const { return m_timelineSemaphore.m_vulkan; }
        [[nodiscard]] uint64_t getTimelineSemaphoreValue() const { return m_timelineSemaphore.m_value; }

    private:
        struct BufferCuda {
            std::shared_ptr<Buffer> bufVk;

#ifdef WIN32
            HANDLE handle = nullptr; // The Win32 handle
#else
            int handle = -1;
#endif
            void *cudaPtr = nullptr;

            void destroy() {
#ifdef WIN32
                CloseHandle(handle);
                handle = nullptr;
#else
                if (handle != -1) {
                    close(handle);
                    handle = -1;
                }
#endif

                RAVEN_BUFFER_RELEASE(bufVk);
            }
        };

        struct TimelineSemaphore {
            vk::Semaphore m_vulkan{};
            uint64_t m_value = 0U;
            cudaExternalSemaphore_t m_cuda{};
#ifdef WIN32
            HANDLE m_handle{INVALID_HANDLE_VALUE};
#else
            int m_handle{-1};
#endif
        } m_timelineSemaphore;

        struct WaitTimelineSemaphore {
            cudaExternalSemaphore_t m_cuda{};
#ifdef WIN32
            HANDLE m_handle{INVALID_HANDLE_VALUE};
#else
            int m_handle{-1};
#endif
        } m_waitTimelineSemaphore;

        OptixDeviceContext m_optixDevice = {};
        OptixDenoiser m_denoiser = {};
        OptixDenoiserOptions m_denoiserOptions = {};
        OptixDenoiserSizes m_denoiserSizes = {};
        OptixDenoiserAlphaMode m_denoiserAlpha = {OPTIX_DENOISER_ALPHA_MODE_COPY};
        OptixPixelFormat m_pixelFormat = {};

        CUdeviceptr m_dStateBuffer = {};
        CUdeviceptr m_dScratchBuffer = {};
        CUdeviceptr m_dIntensity = {};
        CUdeviceptr m_dMinRGB = {};
        CUstream m_cuStream = {};

        vk::Extent2D m_imageSize = {};
        uint32_t m_sizeofPixel = {};

        GPUContext *m_gpuContext = nullptr;

        std::array<BufferCuda, 3> m_pixelBufferIn{};
        BufferCuda m_pixelBufferOut{};

        void createBufferCuda(BufferCuda &buf) const;

        bool initOptiX(const OptixDenoiserOptions &options, OptixPixelFormat pixelFormat);
        void createSemaphore();
        void linkWaitSemaphore(const vk::Semaphore &semaphore);
        void allocateBuffers(const vk::Extent2D &imgSize);
        void destroyBuffers();
    };
} // namespace raven

#endif