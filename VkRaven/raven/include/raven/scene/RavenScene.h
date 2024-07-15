#pragma once

#include "ISceneLoader.h"
#include "RavenLight.h"
#include "RavenMaterial.h"
#include "RavenSceneNode.h"
#include "RavenTexture.h"
#include "raven/core/GPUContext.h"

namespace raven {
    class RavenScene {
    public:
        std::string m_name = "RavenScene";

        std::vector<std::shared_ptr<RavenSceneNode>> m_nodes;

        std::map<std::string, uint32_t> m_materialKeys;
        std::map<std::string, uint32_t> m_materialNames;
        std::vector<std::shared_ptr<RavenMaterial>> m_materials;

        std::map<std::string, uint32_t> m_textureKeys;
        std::map<std::string, uint32_t> m_textureNames;
        std::vector<std::shared_ptr<RavenTexture>> m_textures;

        std::map<std::string, uint32_t> m_lightKeys;
        std::map<std::string, uint32_t> m_lightNames;
        std::vector<std::shared_ptr<RavenLight>> m_lights;

        std::shared_ptr<RavenMaterial> getMaterial(const std::string &name) {
            if (!m_materialNames.contains(name)) {
                throw std::runtime_error("Material " + name + " not found.");
            }
            return m_materials[m_materialNames[name]];
        }

        int32_t getTextureId(const std::string &name) {
            if (!m_textureKeys.contains(name)) {
                return -1;
            }
            return static_cast<int32_t>(m_textureKeys[name]);
        }

        void recordGPUSceneObjects(GPUContext *gpuContext, std::vector<RavenSceneObject::GPUSceneObject> &gpuSceneObjects) {
            for (auto &node: m_nodes) {
                recordGPUSceneObjectsRecursive(gpuContext, gpuSceneObjects, glm::mat4(1.f), node, glm::mat4(1.f));
            }
        }

        void recordLights() {
            m_lightKeys.clear();
            m_lightNames.clear();
            m_lights.clear();
            uint32_t objectDescriptorId = 0;
            for (auto &node: m_nodes) {
                recordLightsRecursive(node, objectDescriptorId);
            }
        }

        void loadScene(ISceneLoader *sceneLoader) {
            const auto node = sceneLoader->loadScene(
                    [&](const std::shared_ptr<RavenMaterial> &material) {
                        if (m_materialKeys.contains(material->m_key)) {
                            throw std::runtime_error("Material " + material->m_key + " already exists.");
                        }
                        m_materialKeys[material->m_key] = static_cast<uint32_t>(m_materials.size());

                        material->m_name = findNextFreeName(m_materialNames, material->m_name);
                        m_materialNames[material->m_name] = static_cast<uint32_t>(m_materials.size());
                        m_materials.push_back(material);
                    },
                    [&](const std::shared_ptr<RavenTexture> &texture) {
                        if (m_textureKeys.contains(texture->m_key)) {
                            throw std::runtime_error("Texture " + texture->m_key + " already exists.");
                        }
                        m_textureKeys[texture->m_key] = static_cast<uint32_t>(m_textures.size());

                        texture->m_name = findNextFreeName(m_textureNames, texture->m_name);
                        m_textureNames[texture->m_name] = static_cast<uint32_t>(m_textures.size());

                        m_textures.push_back(texture);
                    },
                    [&](const std::shared_ptr<RavenLight> &light) {
                        if (m_lightKeys.contains(light->m_key)) {
                            throw std::runtime_error("Light " + light->m_key + " already exists.");
                        }
                        m_lightKeys[light->m_key] = static_cast<uint32_t>(m_lights.size());

                        light->m_name = findNextFreeName(m_lightNames, light->m_name);
                        m_lightNames[light->m_name] = static_cast<uint32_t>(m_lights.size());

                        m_lights.push_back(light);
                    });

            if (node) {
                m_nodes.push_back(node);
            }
        }

    private:
        void recordGPUSceneObjectsRecursive(GPUContext *gpuContext, std::vector<RavenSceneObject::GPUSceneObject> &gpuSceneObjects, glm::mat4 globalTransformation, const std::shared_ptr<RavenSceneNode> &node, glm::mat4 transformation) {
            glm::mat4 thisTransformation = transformation;
            if (node->m_transformation.has_value()) {
                thisTransformation = transformation * node->m_transformation.value();
            }
            for (auto &child: node->m_children) {
                recordGPUSceneObjectsRecursive(gpuContext, gpuSceneObjects, globalTransformation, child, thisTransformation);
            }
            if (node->m_object) {
                if (!m_materialKeys.contains(node->m_object->m_material)) {
                    throw std::runtime_error("Material " + node->m_object->m_material + " not found.");
                }
                gpuSceneObjects.push_back(node->m_object->gpu(gpuContext, m_materialKeys[node->m_object->m_material], thisTransformation, globalTransformation));
            }
        }

        void recordLightsRecursive(const std::shared_ptr<RavenSceneNode> &node, uint32_t &objectDescriptorId) {
            for (auto &child: node->m_children) {
                recordLightsRecursive(child, objectDescriptorId);
            }
            if (node->m_object) {
                if (!m_materialKeys.contains(node->m_object->m_material)) {
                    throw std::runtime_error("Material " + node->m_object->m_material + " not found.");
                }
                const auto &material = m_materials[m_materialKeys[node->m_object->m_material]]->m_material;
                if (material.isEmissive()) {
                    for (uint32_t triangleId = 0; triangleId < node->m_object->m_indices.size() / 3; triangleId++) {
                        auto light = std::make_shared<RavenLight>();
                        light->m_key = std::to_string(objectDescriptorId) + "/" + std::to_string(triangleId);
                        light->m_name = light->m_key;
                        light->m_light.objectDescriptorId = objectDescriptorId;
                        light->m_light.triangleId = triangleId;
                        light->m_light.emission = material.emission;

                        if (m_lightKeys.contains(light->m_key)) {
                            throw std::runtime_error("Light " + light->m_key + " already exists.");
                        }
                        m_lightKeys[light->m_key] = static_cast<uint32_t>(m_lights.size());

                        light->m_name = findNextFreeName(m_lightNames, light->m_name);
                        m_lightNames[light->m_name] = static_cast<uint32_t>(m_lights.size());

                        m_lights.push_back(light);
                    }
                }
                objectDescriptorId++;
            }
        }

        static std::string findNextFreeName(const std::map<std::string, uint32_t> &names, std::string &name) {
            if (!names.contains(name)) {
                return name;
            }
            uint32_t i = 0;
            while (names.contains(name + std::to_string(i))) {
                i++;
            }
            return name + std::to_string(i);
        }
    };
} // namespace raven