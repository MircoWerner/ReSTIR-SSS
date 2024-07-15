#pragma once

#include "../../../VkRaven/lib/imgui/imgui.h"
#include "../../../VkRaven/raven/include/raven/core/AccelerationStructure.h"
#include "../../../VkRaven/raven/include/raven/core/Image.h"
#include "../../../VkRaven/raven/include/raven/core/Texture.h"
#include "../../../VkRaven/raven/include/raven/scene/RavenScene.h"
#include "../../../VkRaven/raven/include/raven/scene/RavenSceneObject.h"
#include "Raystructs.h"

#include <memory>
#include <vector>

namespace raven {
    class Raydata {
    public:
        std::vector<std::shared_ptr<Texture>> m_textures;
        std::shared_ptr<Texture> m_textureEnvironment;

        std::shared_ptr<Buffer> m_objectDescriptorBuffer;

        std::shared_ptr<Buffer> m_materialBuffer;
        std::shared_ptr<Buffer> m_lightBuffer;

        std::shared_ptr<TLAS> m_tlas;

        std::vector<RavenSceneObject::GPUSceneObject> m_gpuMeshInfos;

        RavenScene m_scene;

        int32_t m_selectedMaterial = -1;

        //
        void initResources(GPUContext *gpuContext, uint32_t *numLights, const std::function<void(const RavenSceneObject::GPUSceneObject &sceneObject, ObjectDescriptor &objectDescriptor)> &executeForSceneObject = [](const RavenSceneObject::GPUSceneObject &, ObjectDescriptor &) {}) {
            std::vector<ObjectDescriptor> objectDescriptors;
            {
                // ACCELERATION STRUCTURE

                // == TLAS ===
                m_tlas = std::make_shared<TLAS>(gpuContext);

                // === BLAS ===
                m_scene.recordGPUSceneObjects(gpuContext, m_gpuMeshInfos);
                for (auto &gpuMeshInfo: m_gpuMeshInfos) {
                    const auto &info = gpuMeshInfo;
                    m_tlas->addBLAS({info.m_blas, static_cast<uint32_t>(objectDescriptors.size()), TLAS::BLASHandle::transform(info.m_globalTransformation)});
                    glm::mat4 objectToWorld = info.m_globalTransformation * gpuMeshInfo.m_modelMatrix;
                    objectDescriptors.push_back({info.m_vertexBuffer->getDeviceAddress(), info.m_indexBuffer->getDeviceAddress(), info.m_materialId, objectToWorld, glm::transpose(glm::inverse(objectToWorld))});
                    executeForSceneObject(info, objectDescriptors.back());
                }

                // == TLAS ===
                m_tlas->build();
            }

            // object descriptors
            {
                auto bufferSettings = Buffer::BufferSettings{.m_sizeBytes = static_cast<uint32_t>(sizeof(ObjectDescriptor) * objectDescriptors.size()), .m_bufferUsages = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer, .m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal, .m_name = "objectDescriptors"};
                m_objectDescriptorBuffer = Buffer::fillDeviceWithStagingBuffer(gpuContext, bufferSettings, objectDescriptors.data());
            }

            initMaterials(gpuContext);
            initLights(gpuContext, numLights);

            // textures
            for (const auto &texture: m_scene.m_textures) {
                const std::string TEXTURE_PATH = texture->m_directory + "/" + texture->m_uri;
                auto settings = Image::ImageSettings{.m_mipMapping = true, .m_name = texture->m_directory + "/" + texture->m_uri};
                m_textures.push_back(std::make_shared<Texture>(gpuContext, settings, TEXTURE_PATH));
                m_textures[m_textures.size() - 1]->create();
            }

            // environment texture
            {
                const std::string TEXTURE_PATH = raven::Paths::m_resourceDirectoryPath + "/environment/white.png";
                auto settings = Image::ImageSettings{.m_mipMapping = true, .m_name = "environment"};
                m_textureEnvironment = std::make_shared<Texture>(gpuContext, settings, TEXTURE_PATH);
                m_textureEnvironment->create();
            }
        }

        void releaseResources() {
            for (const auto &texture: m_textures) {
                texture->release();
            }
            m_textureEnvironment->release();

            for (const auto &info: m_gpuMeshInfos) {
                info.m_vertexBuffer->release();
                info.m_indexBuffer->release();
                info.m_blas->release();
            }
            m_objectDescriptorBuffer->release();
            m_materialBuffer->release();
            m_lightBuffer->release();

            m_tlas->release();

            m_textures.clear();
            m_gpuMeshInfos.clear();
            m_scene = RavenScene();
        }

        static void recordImGuiScene(const std::shared_ptr<RavenSceneNode> &node) {
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::TreeNode(node->m_name.c_str())) {
                for (const auto &child: node->m_children) {
                    recordImGuiScene(child);
                }
                if (node->m_object) {
                    ImGui::Text("%s", node->m_object->m_name.c_str());
                }
                ImGui::TreePop();
            }
        }

        void guiScene(GPUContext *gpuContext, bool *updateMaterial, bool *updateLight, uint32_t *numLights) {
            if (ImGui::CollapsingHeader("Scene Hierarchy")) {
                for (const auto &node: m_scene.m_nodes) {
                    recordImGuiScene(node);
                }
            }
            if (ImGui::CollapsingHeader("Materials")) {
                std::vector<std::string> materialNames;
                std::vector<const char *> materials;
                for (const auto &material: m_scene.m_materials) {
                    materialNames.push_back(material->m_key + " (" + material->m_name + ")");
                    materials.push_back(materialNames.back().c_str());
                }
                if (!materials.empty()) {
                    ImGui::Combo("Material", &m_selectedMaterial, materials.data(), (int) materials.size());

                    if (m_selectedMaterial >= 0 && m_selectedMaterial < materials.size()) {
                        const auto &materialInfo = m_scene.m_materials[m_selectedMaterial];
                        auto &material = materialInfo->m_material;

                        *updateMaterial = false;
                        *updateLight = false;
                        if (ImGui::ColorEdit3("Base Color Factor", &material.baseColorFactor.x)) {
                            *updateMaterial = true;
                        }
                        if (ImGui::SliderFloat("Metallic Factor", &material.metallicFactor, 0.f, 1.f)) {
                            *updateMaterial = true;
                        }
                        if (ImGui::SliderFloat("Roughness Factor", &material.roughnessFactor, 0.f, 1.f)) {
                            *updateMaterial = true;
                        }
                        if (ImGui::ColorEdit3("Emissive Factor", &materialInfo->m_emissiveFactor.x)) {
                            materialInfo->updateEmission();
                            *updateMaterial = true;
                            *updateLight = true;
                        }
                        if (ImGui::SliderFloat("Emissive Strength", &materialInfo->m_emissiveStrength, 0.f, 1.f)) {
                            materialInfo->updateEmission();
                            *updateMaterial = true;
                            *updateLight = true;
                        }
                        ImGui::Separator();
                        if (ImGui::SliderFloat("Subsurface Factor", &material.subsurface, 0.f, 1.f)) {
                            *updateMaterial = true;
                        }
                        if (ImGui::SliderFloat3("Subsurface Mean Free Path (l)", &material.meanFreePath.x, 0.f, 10.f)) {
                            *updateMaterial = true;
                        }
                        const glm::vec3 subsurfaceScaling = material.scatteringScaling(material.baseColorFactor);
                        ImGui::Text("Subsurface Scaling (%f,%f,%f)", subsurfaceScaling.x, subsurfaceScaling.y, subsurfaceScaling.z);
                        const glm::vec3 subsurfaceDistance = material.scatteringDistance(material.baseColorFactor);
                        ImGui::Text("Subsurface Distance (%f,%f,%f)", subsurfaceDistance.x, subsurfaceDistance.y, subsurfaceDistance.z);

                        if (*updateMaterial) {
                            initMaterials(gpuContext);
                        }
                        if (*updateLight) {
                            initLights(gpuContext, numLights);
                        }
                    }
                }
            }
        }

    private:
        void initMaterials(GPUContext *gpuContext) {
            gpuContext->m_device.waitIdle();

            if (m_materialBuffer) {
                m_materialBuffer->release();
            }
            // materials
            {
                std::vector<RavenMaterial::Material> materials;
                for (auto &material: m_scene.m_materials) {
                    material->updateTextureIds([&](const std::string &name) { return m_scene.getTextureId(name); });
                    materials.push_back(material->m_material);
                }
                if (materials.empty()) {
                    materials.emplace_back();
                }
                const auto bufferSettings = Buffer::BufferSettings{.m_sizeBytes = static_cast<uint32_t>(sizeof(RavenMaterial::Material) * materials.size()), .m_bufferUsages = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer, .m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal, .m_name = "materials"};
                m_materialBuffer = Buffer::fillDeviceWithStagingBuffer(gpuContext, bufferSettings, materials.data());
            }
        }

        void initLights(GPUContext *gpuContext, uint32_t *numLights) {
            gpuContext->m_device.waitIdle();

            if (m_lightBuffer) {
                m_lightBuffer->release();
            }
            // lights
            {
                m_scene.recordLights();
                std::vector<RavenLight::Light> lights;
                for (const auto &light: m_scene.m_lights) {
                    lights.push_back(light->m_light);
                }
                if (lights.empty()) {
                    lights.emplace_back();
                }
                const auto bufferSettings = Buffer::BufferSettings{.m_sizeBytes = static_cast<uint32_t>(sizeof(RavenLight::Light) * lights.size()), .m_bufferUsages = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer, .m_memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal, .m_name = "lights"};
                m_lightBuffer = Buffer::fillDeviceWithStagingBuffer(gpuContext, bufferSettings, lights.data());
                *numLights = m_scene.m_lights.size();
            }
        }
    };
} // namespace raven