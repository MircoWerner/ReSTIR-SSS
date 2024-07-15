#pragma once
#include "raven/core/AccelerationStructure.h"
#include "raven/util/AABB.h"


#include <vector>

namespace raven {
    class RavenSceneObject {
    public:
        std::string m_name = "RavenSceneObject";

        struct Vertex {
            float position_x;
            float position_y;
            float position_z;
            float normal_x;
            float normal_y;
            float normal_z;
            float tangent_x;
            float tangent_y;
            float tangent_z;
            float tangent_w; // bitangent sign
            float texCoord_u;
            float texCoord_v;
        };

        AABB m_aabb;

        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;

        std::string m_material{};

        // GPU
        struct GPUSceneObject {
            std::shared_ptr<Buffer> m_vertexBuffer = nullptr;
            std::shared_ptr<Buffer> m_indexBuffer = nullptr;
            uint32_t m_materialId = 0;

            uint32_t m_numPrimitives = 0;

            std::shared_ptr<TriangleBLAS> m_blas = nullptr;

            glm::mat4 m_modelMatrix{};

            glm::mat4 m_globalTransformation{};

            AABB m_abbb{};

            RavenSceneObject *m_sceneObject = nullptr;
        };

        GPUSceneObject gpu(GPUContext *gpuContext, uint32_t materialId, const glm::mat4 &transformation, const glm::mat4 &globalTransformation) {
            GPUSceneObject gpuSceneObject{};

            // if (m_material.isEmissive()) {
            //     for (uint32_t lId = 0; lId < getNumPrimitives(); lId++) {
            //         info.m_lights.push_back({lId + numPrimitives});
            //     }
            // }

            gpuSceneObject.m_vertexBuffer = Buffer::fillDeviceWithStagingBuffer(gpuContext,
                                                                                {.m_sizeBytes = static_cast<uint32_t>(m_vertices.size() * sizeof(Vertex)), .m_bufferUsages = vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR, .m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal, .m_memoryAllocateFlagBits = vk::MemoryAllocateFlagBits::eDeviceAddress, .m_name = "vertex0"},
                                                                                reinterpret_cast<void *>(m_vertices.data()));
            gpuSceneObject.m_indexBuffer = Buffer::fillDeviceWithStagingBuffer(gpuContext,
                                                                               {.m_sizeBytes = static_cast<uint32_t>(m_indices.size() * sizeof(uint32_t)), .m_bufferUsages = vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR, .m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal, .m_memoryAllocateFlagBits = vk::MemoryAllocateFlagBits::eDeviceAddress, .m_name = "index0"},
                                                                               reinterpret_cast<void *>(m_indices.data()));

            gpuSceneObject.m_materialId = materialId;

            gpuSceneObject.m_numPrimitives = m_indices.size() / 3;

            gpuSceneObject.m_blas = std::make_shared<TriangleBLAS>(gpuContext, gpuSceneObject.m_vertexBuffer->getDeviceAddress(), static_cast<uint32_t>(m_vertices.size()), sizeof(Vertex), gpuSceneObject.m_indexBuffer->getDeviceAddress(), m_indices.size() / 3);
            gpuSceneObject.m_blas->transform(transformation);
            gpuSceneObject.m_blas->build();

            gpuSceneObject.m_modelMatrix = transformation;

            gpuSceneObject.m_globalTransformation = globalTransformation;

            gpuSceneObject.m_abbb.expand(m_aabb.m_min);
            gpuSceneObject.m_abbb.expand(m_aabb.m_max);

            gpuSceneObject.m_sceneObject = this;

            return gpuSceneObject;
        }
    };
} // namespace raven