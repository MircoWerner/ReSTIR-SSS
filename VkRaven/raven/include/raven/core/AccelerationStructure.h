#pragma once

#include "Buffer.h"

namespace raven {
#define RAVEN_AS_RELEASE(as) \
if (as) as->release()

    class AccelerationStructure {
    public:
        explicit AccelerationStructure(GPUContext *gpuContext) : m_gpuContext(gpuContext) {
        }

        ~AccelerationStructure() { release(); }

        virtual void build() = 0;

        void release() {
            if (m_accelerationStructure) {
                m_gpuContext->m_device.destroyAccelerationStructureKHR(m_accelerationStructure, nullptr);
                m_accelerationStructure = nullptr;
            }
            if (m_accelerationStructureBuffer) {
                m_accelerationStructureBuffer->release();
                m_accelerationStructureBuffer = nullptr;
            }
        }

        [[nodiscard]] vk::AccelerationStructureKHR getAccelerationStructure() const { return m_accelerationStructure; }

        [[nodiscard]] uint64_t getSizeBytes() const {
            return m_accelerationStructureBuffer->getSizeBytes();
        }

    protected:
        GPUContext *m_gpuContext;

        vk::AccelerationStructureKHR m_accelerationStructure{};
        std::shared_ptr<Buffer> m_accelerationStructureBuffer;
    };

    class BLAS : public AccelerationStructure {
    public:
        explicit BLAS(GPUContext *gpuContext) : AccelerationStructure(gpuContext) {
        }

        void build() override {
            vk::AccelerationStructureKHR accelerationStructure{};
            std::shared_ptr<Buffer> accelerationStructureBuffer;

            {
                preBuild();

                vk::AccelerationStructureGeometryKHR asGeometry = getASGeometry();
                vk::AccelerationStructureBuildRangeInfoKHR asBuildRangeInfo = getASBuildRangeInfo();

                vk::AccelerationStructureBuildGeometryInfoKHR buildInfo;
                buildInfo.sType = vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR;
                buildInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction;
                buildInfo.geometryCount = 1;
                buildInfo.pGeometries = &asGeometry;
                buildInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
                buildInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
                buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;

                vk::AccelerationStructureBuildSizesInfoKHR sizeInfo;
                sizeInfo.sType = vk::StructureType::eAccelerationStructureBuildSizesInfoKHR;
                m_gpuContext->m_device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, &buildInfo, &asBuildRangeInfo.primitiveCount,
                                                                             &sizeInfo); // query worst case memory usage

                Buffer::BufferSettings bufferSettingsBLAS{
                        .m_sizeBytes = static_cast<uint32_t>(sizeInfo.accelerationStructureSize),
                        .m_bufferUsages = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
                        .m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
                        .m_memoryAllocateFlagBits = vk::MemoryAllocateFlagBits::eDeviceAddress,
                        .m_name = "blas"};
                accelerationStructureBuffer = std::make_shared<Buffer>(m_gpuContext, bufferSettingsBLAS);

                vk::AccelerationStructureCreateInfoKHR createInfo;
                createInfo.sType = vk::StructureType::eAccelerationStructureCreateInfoKHR;
                createInfo.type = buildInfo.type;
                createInfo.size = sizeInfo.accelerationStructureSize;
                createInfo.buffer = accelerationStructureBuffer->getBuffer();
                createInfo.offset = 0;
                if (m_gpuContext->m_device.createAccelerationStructureKHR(&createInfo, nullptr, &accelerationStructure) != vk::Result::eSuccess) {
                    throw std::runtime_error("Cannot create BLAS.");
                }
                buildInfo.dstAccelerationStructure = accelerationStructure;

                Buffer::BufferSettings bufferSettingsScratch{.m_sizeBytes = static_cast<uint32_t>(sizeInfo.buildScratchSize),
                                                             .m_bufferUsages = vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
                                                             .m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
                                                             .m_memoryAllocateFlagBits = vk::MemoryAllocateFlagBits::eDeviceAddress,
                                                             .m_name = "scratch"};
                std::shared_ptr<Buffer> scratchBuffer = std::make_shared<Buffer>(m_gpuContext, bufferSettingsScratch);
                buildInfo.scratchData.deviceAddress = scratchBuffer->getDeviceAddress();

                vk::AccelerationStructureBuildRangeInfoKHR *pRangeInfo = &asBuildRangeInfo;
                m_gpuContext->executeCommands([&](vk::CommandBuffer commandBuffer) { commandBuffer.buildAccelerationStructuresKHR(1, &buildInfo, &pRangeInfo); }, Queues::COMPUTE);

                postBuild();
            }

            {
                // === compaction ===
                // query pool
                vk::QueryPool queryPool{};
                vk::QueryPoolCreateInfo queryPoolCreateInfo{};
                queryPoolCreateInfo.queryCount = 1;
                queryPoolCreateInfo.queryType = vk::QueryType::eAccelerationStructureCompactedSizeKHR;
                if (m_gpuContext->m_device.createQueryPool(&queryPoolCreateInfo, nullptr, &queryPool) != vk::Result::eSuccess) {
                    throw std::runtime_error("Cannot create query pool!");
                }
                m_gpuContext->m_device.resetQueryPool(queryPool, 0, 1);

                // query compact size
                m_gpuContext->executeCommands([&](vk::CommandBuffer commandBuffer) { commandBuffer.writeAccelerationStructuresPropertiesKHR(1, &accelerationStructure, vk::QueryType::eAccelerationStructureCompactedSizeKHR, queryPool, 0); }, Queues::COMPUTE);
                vk::DeviceSize compactSize;
                if (m_gpuContext->m_device.getQueryPoolResults(queryPool, 0, 1, sizeof(vk::DeviceSize), &compactSize, sizeof(vk::DeviceSize), vk::QueryResultFlagBits::eWait) != vk::Result::eSuccess) {
                    throw std::runtime_error("Cannot get compact size!");
                }

                // compacted version buffer
                Buffer::BufferSettings bufferSettingsTLAS{.m_sizeBytes = static_cast<uint32_t>(compactSize),
                                                          .m_bufferUsages = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
                                                          .m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
                                                          .m_memoryAllocateFlagBits = vk::MemoryAllocateFlagBits::eDeviceAddress,
                                                          .m_name = "blas"};
                m_accelerationStructureBuffer = std::make_shared<Buffer>(m_gpuContext, bufferSettingsTLAS);

                // compact version
                vk::AccelerationStructureCreateInfoKHR createInfo;
                createInfo.sType = vk::StructureType::eAccelerationStructureCreateInfoKHR;
                createInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
                createInfo.size = static_cast<uint32_t>(compactSize);
                createInfo.buffer = m_accelerationStructureBuffer->getBuffer();
                createInfo.offset = 0;
                if (m_gpuContext->m_device.createAccelerationStructureKHR(&createInfo, nullptr, &m_accelerationStructure) != vk::Result::eSuccess) {
                    throw std::runtime_error("Cannot create BLAS.");
                }

                // copy the original BLAS to a compact version
                vk::CopyAccelerationStructureInfoKHR copyInfo;
                copyInfo.src = accelerationStructure;
                copyInfo.dst = m_accelerationStructure;
                copyInfo.mode = vk::CopyAccelerationStructureModeKHR::eCompact;

                m_gpuContext->executeCommands([&](vk::CommandBuffer commandBuffer) { commandBuffer.copyAccelerationStructureKHR(&copyInfo); }, Queues::COMPUTE);

                // destroy query pool
                m_gpuContext->m_device.destroyQueryPool(queryPool);
            }

            m_gpuContext->m_device.destroyAccelerationStructureKHR(accelerationStructure, nullptr);
            accelerationStructureBuffer->release();
        }

    protected:
        virtual vk::AccelerationStructureGeometryKHR getASGeometry() = 0;
        virtual vk::AccelerationStructureBuildRangeInfoKHR getASBuildRangeInfo() = 0;

        virtual void preBuild() {

        }

        virtual void postBuild() {

        }
    };

    class TLAS : public AccelerationStructure {
    public:
        struct BLASHandle {
            std::shared_ptr<BLAS> m_blas;
            uint32_t m_instanceCustomIndex = 0; // 24bit only
            vk::TransformMatrixKHR m_transform = std::array<std::array<float, 4>, 3>{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

            static vk::TransformMatrixKHR translate(glm::vec3 translate) {
                return std::array<std::array<float, 4>, 3>{1.0f, 0.0f, 0.0f, translate.x, 0.0f, 1.0f, 0.0f, translate.y, 0.0f, 0.0f, 1.0f, translate.z};
            }

            static vk::TransformMatrixKHR transform(glm::mat4 transformation) {
                return std::array<std::array<float, 4>, 3>{transformation[0].x, transformation[1].x, transformation[2].x, transformation[3].x,
                                                           transformation[0].y, transformation[1].y, transformation[2].y, transformation[3].y,
                                                           transformation[0].z, transformation[1].z, transformation[2].z, transformation[3].z};
            }
        };

        explicit TLAS(GPUContext *gpuContext) : AccelerationStructure(gpuContext) {
        }

        void build() override {
            vk::AccelerationStructureKHR accelerationStructure{};
            std::shared_ptr<Buffer> accelerationStructureBuffer;

            {
                std::vector<vk::AccelerationStructureInstanceKHR> instances;
                createInstances(instances);

                Buffer::BufferSettings bufferSettingsInstances{.m_sizeBytes = static_cast<uint32_t>(sizeof(instances[0]) * instances.size()),
                                                               .m_bufferUsages = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress |
                                                                                 vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
                                                               .m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
                                                               .m_memoryAllocateFlagBits = vk::MemoryAllocateFlagBits::eDeviceAddress,
                                                               .m_name = "instances"};
                std::shared_ptr<Buffer> instancesBuffer = Buffer::fillDeviceWithStagingBuffer(m_gpuContext, bufferSettingsInstances, instances.data());

                vk::AccelerationStructureBuildRangeInfoKHR rangeInfo;
                rangeInfo.primitiveOffset = 0;
                rangeInfo.primitiveCount = instances.size(); // #instances
                rangeInfo.firstVertex = 0;
                rangeInfo.transformOffset = 0;

                vk::AccelerationStructureGeometryInstancesDataKHR instancesData;
                instancesData.sType = vk::StructureType::eAccelerationStructureGeometryInstancesDataKHR;
                instancesData.arrayOfPointers = VK_FALSE;
                instancesData.data.deviceAddress = instancesBuffer->getDeviceAddress();

                vk::AccelerationStructureGeometryKHR geometry;
                geometry.sType = vk::StructureType::eAccelerationStructureGeometryKHR;
                geometry.geometryType = vk::GeometryTypeKHR::eInstances;
                geometry.geometry.instances = instancesData;

                vk::AccelerationStructureBuildGeometryInfoKHR buildInfo;
                buildInfo.sType = vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR;
                buildInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction;
                buildInfo.geometryCount = 1;
                buildInfo.pGeometries = &geometry;
                buildInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
                buildInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
                buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;

                vk::AccelerationStructureBuildSizesInfoKHR sizeInfo;
                sizeInfo.sType = vk::StructureType::eAccelerationStructureBuildSizesInfoKHR;
                m_gpuContext->m_device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, &buildInfo, &rangeInfo.primitiveCount, &sizeInfo);

                Buffer::BufferSettings bufferSettingsTLAS{
                        .m_sizeBytes = static_cast<uint32_t>(sizeInfo.accelerationStructureSize),
                        .m_bufferUsages = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
                        .m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
                        .m_memoryAllocateFlagBits = vk::MemoryAllocateFlagBits::eDeviceAddress,
                        .m_name = "tlas"};
                accelerationStructureBuffer = std::make_shared<Buffer>(m_gpuContext, bufferSettingsTLAS);

                vk::AccelerationStructureCreateInfoKHR createInfo;
                createInfo.sType = vk::StructureType::eAccelerationStructureCreateInfoKHR;
                createInfo.type = buildInfo.type;
                createInfo.size = sizeInfo.accelerationStructureSize;
                createInfo.buffer = accelerationStructureBuffer->getBuffer();
                createInfo.offset = 0;
                if (m_gpuContext->m_device.createAccelerationStructureKHR(&createInfo, nullptr, &accelerationStructure) != vk::Result::eSuccess) {
                    throw std::runtime_error("Cannot create TLAS.");
                }
                buildInfo.dstAccelerationStructure = accelerationStructure;

                Buffer::BufferSettings bufferSettingsScratch{.m_sizeBytes = static_cast<uint32_t>(sizeInfo.buildScratchSize),
                                                             .m_bufferUsages = vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
                                                             .m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
                                                             .m_memoryAllocateFlagBits = vk::MemoryAllocateFlagBits::eDeviceAddress,
                                                             .m_name = "scratch"};
                std::shared_ptr<Buffer> scratchBuffer = std::make_shared<Buffer>(m_gpuContext, bufferSettingsScratch);
                buildInfo.scratchData.deviceAddress = scratchBuffer->getDeviceAddress();

                vk::AccelerationStructureBuildRangeInfoKHR *pRangeInfo = &rangeInfo;
                m_gpuContext->executeCommands([&](vk::CommandBuffer commandBuffer) { commandBuffer.buildAccelerationStructuresKHR(1, &buildInfo, &pRangeInfo); }, Queues::COMPUTE);
            }

            {
                // === compaction ===
                // query pool
                vk::QueryPool queryPool{};
                vk::QueryPoolCreateInfo queryPoolCreateInfo{};
                queryPoolCreateInfo.queryCount = 1;
                queryPoolCreateInfo.queryType = vk::QueryType::eAccelerationStructureCompactedSizeKHR;
                if (m_gpuContext->m_device.createQueryPool(&queryPoolCreateInfo, nullptr, &queryPool) != vk::Result::eSuccess) {
                    throw std::runtime_error("Cannot create query pool!");
                }
                m_gpuContext->m_device.resetQueryPool(queryPool, 0, 1);

                // query compact size
                m_gpuContext->executeCommands([&](vk::CommandBuffer commandBuffer) { commandBuffer.writeAccelerationStructuresPropertiesKHR(1, &accelerationStructure, vk::QueryType::eAccelerationStructureCompactedSizeKHR, queryPool, 0); }, Queues::COMPUTE);
                vk::DeviceSize compactSize;
                if (m_gpuContext->m_device.getQueryPoolResults(queryPool, 0, 1, sizeof(vk::DeviceSize), &compactSize, sizeof(vk::DeviceSize), vk::QueryResultFlagBits::eWait) != vk::Result::eSuccess) {
                    throw std::runtime_error("Cannot get compact size!");
                }

                // compacted version buffer
                Buffer::BufferSettings bufferSettingsTLAS{.m_sizeBytes = static_cast<uint32_t>(compactSize),
                                                          .m_bufferUsages = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
                                                          .m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
                                                          .m_memoryAllocateFlagBits = vk::MemoryAllocateFlagBits::eDeviceAddress,
                                                          .m_name = "tlas"};
                m_accelerationStructureBuffer = std::make_shared<Buffer>(m_gpuContext, bufferSettingsTLAS);

                // compact version
                vk::AccelerationStructureCreateInfoKHR createInfo;
                createInfo.sType = vk::StructureType::eAccelerationStructureCreateInfoKHR;
                createInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
                createInfo.size = static_cast<uint32_t>(compactSize);
                createInfo.buffer = m_accelerationStructureBuffer->getBuffer();
                createInfo.offset = 0;
                if (m_gpuContext->m_device.createAccelerationStructureKHR(&createInfo, nullptr, &m_accelerationStructure) != vk::Result::eSuccess) {
                    throw std::runtime_error("Cannot create TLAS.");
                }

                // copy the original TLAS to a compact version
                vk::CopyAccelerationStructureInfoKHR copyInfo;
                copyInfo.src = accelerationStructure;
                copyInfo.dst = m_accelerationStructure;
                copyInfo.mode = vk::CopyAccelerationStructureModeKHR::eCompact;

                m_gpuContext->executeCommands([&](vk::CommandBuffer commandBuffer) { commandBuffer.copyAccelerationStructureKHR(&copyInfo); }, Queues::COMPUTE);

                // destroy query pool
                m_gpuContext->m_device.destroyQueryPool(queryPool);
            }

            m_gpuContext->m_device.destroyAccelerationStructureKHR(accelerationStructure, nullptr);
            accelerationStructureBuffer->release();
        }

        void addBLAS(const BLASHandle &blas) { m_blas.push_back(blas); }

    private:
        void createInstances(std::vector<vk::AccelerationStructureInstanceKHR> &instances) {
            for (const auto &blas: m_blas) {
                vk::AccelerationStructureDeviceAddressInfoKHR addressInfo;
                addressInfo.sType = vk::StructureType::eAccelerationStructureDeviceAddressInfoKHR;
                addressInfo.accelerationStructure = blas.m_blas->getAccelerationStructure();
                vk::DeviceAddress blasAddress = m_gpuContext->m_device.getAccelerationStructureAddressKHR(&addressInfo);

                vk::AccelerationStructureInstanceKHR instance{};
                instance.transform = blas.m_transform;
                instance.instanceCustomIndex = blas.m_instanceCustomIndex;
                instance.mask = 0xFF;
                instance.instanceShaderBindingTableRecordOffset = 0;
                //                instance.flags = static_cast<VkGeometryInstanceFlagsKHR>(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable);
                instance.accelerationStructureReference = blasAddress;

                instances.push_back(instance);
            }
        }

        std::vector<BLASHandle> m_blas;
    };

    class TriangleBLAS : public BLAS {
    public:
        TriangleBLAS(GPUContext *gpuContext, vk::DeviceAddress vertexBufferAddress, uint32_t numVertices, vk::DeviceSize vertexStride, vk::DeviceAddress indexBufferAddress, uint32_t numPrimitives)
            : BLAS(gpuContext), m_vertexBufferAddress(vertexBufferAddress), m_numVertices(numVertices), m_vertexStride(vertexStride), m_indexBufferAddress(indexBufferAddress), m_numPrimitives(numPrimitives) {
        }

        void transform(glm::mat4 transformation) {
            m_transform = getTransform(transformation);
        }

    protected:
        vk::AccelerationStructureGeometryKHR getASGeometry() override {
            vk::AccelerationStructureGeometryTrianglesDataKHR triangles;
            triangles.sType = vk::StructureType::eAccelerationStructureGeometryTrianglesDataKHR;
            triangles.vertexFormat = vk::Format::eR32G32B32Sfloat; // vec3 vertex position data.
            triangles.vertexData.deviceAddress = m_vertexBufferAddress;
            triangles.vertexStride = m_vertexStride;
            triangles.indexType = vk::IndexType::eUint32;
            triangles.indexData.deviceAddress = m_indexBufferAddress;
            triangles.transformData.deviceAddress = m_transformData->getDeviceAddress();
            triangles.maxVertex = m_numVertices - 1;

            vk::AccelerationStructureGeometryKHR asGeom;
            asGeom.sType = vk::StructureType::eAccelerationStructureGeometryKHR;
            asGeom.geometryType = vk::GeometryTypeKHR::eTriangles;
            asGeom.flags = vk::GeometryFlagBitsKHR::eNoDuplicateAnyHitInvocation | vk::GeometryFlagBitsKHR::eOpaque;
            asGeom.geometry.triangles = triangles;
            return asGeom;
        }

        vk::AccelerationStructureBuildRangeInfoKHR getASBuildRangeInfo() override {
            vk::AccelerationStructureBuildRangeInfoKHR offset{};
            offset.firstVertex = 0;
            offset.primitiveCount = m_numPrimitives;
            offset.primitiveOffset = 0;
            offset.transformOffset = 0;
            return offset;
        }

        void preBuild() override {
            Buffer::BufferSettings bufferSettingsTransformData{.m_sizeBytes = static_cast<uint32_t>(sizeof(m_transform)),
                                                               .m_bufferUsages = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
                                                               .m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
                                                               .m_memoryAllocateFlagBits = vk::MemoryAllocateFlagBits::eDeviceAddress,
                                                               .m_name = "transformData"};
            m_transformData = Buffer::fillDeviceWithStagingBuffer(m_gpuContext, bufferSettingsTransformData, m_transform.matrix.data());
        }

        void postBuild() override {
            m_transformData->release();
            m_transformData.reset();
        }

        vk::DeviceAddress m_vertexBufferAddress;
        uint32_t m_numVertices;
        vk::DeviceSize m_vertexStride;

        vk::DeviceAddress m_indexBufferAddress;
        uint32_t m_numPrimitives; // == numIndices / 3

        vk::TransformMatrixKHR m_transform = std::array<std::array<float, 4>, 3>{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

        static vk::TransformMatrixKHR getTranslate(glm::vec3 translate) {
            return std::array<std::array<float, 4>, 3>{1.0f, 0.0f, 0.0f, translate.x, 0.0f, 1.0f, 0.0f, translate.y, 0.0f, 0.0f, 1.0f, translate.z};
        }

        static vk::TransformMatrixKHR getTransform(glm::mat4 transformation) {
            return std::array<std::array<float, 4>, 3>{transformation[0].x, transformation[1].x, transformation[2].x, transformation[3].x,
                                                       transformation[0].y, transformation[1].y, transformation[2].y, transformation[3].y,
                                                       transformation[0].z, transformation[1].z, transformation[2].z, transformation[3].z};
        }

        std::shared_ptr<Buffer> m_transformData;
    };

    class AabbBLAS : public BLAS {
    public:
        AabbBLAS(GPUContext *gpuContext, vk::DeviceAddress m_aabbBufferAddress, vk::DeviceSize m_aabbStride, uint32_t numPrimitives)
            : BLAS(gpuContext), m_aabbBufferAddress(m_aabbBufferAddress), m_aabbStride(m_aabbStride), m_numPrimitives(numPrimitives) {
        }

    protected:
        vk::AccelerationStructureGeometryKHR getASGeometry() override {
            vk::AccelerationStructureGeometryAabbsDataKHR aabbs;
            aabbs.sType = vk::StructureType::eAccelerationStructureGeometryAabbsDataKHR;
            aabbs.data.deviceAddress = m_aabbBufferAddress;
            aabbs.stride = m_aabbStride;

            vk::AccelerationStructureGeometryKHR asGeom;
            asGeom.sType = vk::StructureType::eAccelerationStructureGeometryKHR;
            asGeom.geometryType = vk::GeometryTypeKHR::eAabbs;
            asGeom.flags = vk::GeometryFlagBitsKHR::eNoDuplicateAnyHitInvocation | vk::GeometryFlagBitsKHR::eOpaque;
            asGeom.geometry.aabbs = aabbs;
            return asGeom;
        }

        vk::AccelerationStructureBuildRangeInfoKHR getASBuildRangeInfo() override {
            vk::AccelerationStructureBuildRangeInfoKHR offset{};
            offset.firstVertex = 0;
            offset.primitiveCount = m_numPrimitives;
            offset.primitiveOffset = 0;
            offset.transformOffset = 0;
            return offset;
        }

        vk::DeviceAddress m_aabbBufferAddress;
        vk::DeviceSize m_aabbStride;
        uint32_t m_numPrimitives; // == numAabbs
    };
} // namespace raven