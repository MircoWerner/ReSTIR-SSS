#pragma once

#ifdef WIN32
#include <windows.h>
#endif

#include "raven/scene/ISceneLoader.h"

#include <glm/glm.hpp>
#include <iostream>
#include <tinygltf/tiny_gltf.h>

namespace raven {
    class GLTFLoader : public ISceneLoader {
    public:
        GLTFLoader(std::string folder, std::string file, glm::mat4 transformation, uint32_t instance = 0) : m_folder(std::move(folder)), m_file(std::move(file)), m_transformation(transformation), m_instance(instance) {
        }

        virtual ~GLTFLoader() = default;

        std::shared_ptr<RavenSceneNode> loadScene(std::function<void(const std::shared_ptr<RavenMaterial> &)> registerMaterial, std::function<void(const std::shared_ptr<RavenTexture> &)> registerTexture, std::function<void(const std::shared_ptr<RavenLight> &)> registerLight) override {
            tinygltf::Model model;
            tinygltf::TinyGLTF loader;
            std::string err;
            std::string warn;

            const bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, m_folder + "/" + m_file);
            //        bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, MODEL_PATH); // for binary glTF(.glb)

            if (!warn.empty()) {
                printf("Warn: %s\n", warn.c_str());
                return nullptr;
            }

            if (!err.empty()) {
                printf("Err: %s\n", err.c_str());
                return nullptr;
            }

            if (!ret) {
                printf("Failed to parse glTF\n");
                return nullptr;
            }

            if (model.scenes.empty()) {
                std::cout << "No scene found." << std::endl;
                return nullptr;
            }

            std::vector<std::shared_ptr<RavenMaterial>> materials;

            const auto node = loadScene(model, model.scenes[0]);

            std::string keyPrefix = m_folder + "/" + m_file + "/" + std::to_string(m_instance) + "/";

            // materials
            {
                for (uint32_t i = 0; i < model.materials.size(); i++) {
                    const tinygltf::Material &material = model.materials[i];

                    auto rMaterial = std::make_shared<RavenMaterial>();

                    rMaterial->m_material.baseColorFactor = glm::vec4(material.pbrMetallicRoughness.baseColorFactor[0], material.pbrMetallicRoughness.baseColorFactor[1], material.pbrMetallicRoughness.baseColorFactor[2], material.pbrMetallicRoughness.baseColorFactor[3]);
                    rMaterial->m_baseColorTexture = m_folder + "/" + m_file + "/" + std::to_string(m_instance) + "/" + std::to_string(material.pbrMetallicRoughness.baseColorTexture.index);
                    rMaterial->m_material.metallicFactor = static_cast<float>(material.pbrMetallicRoughness.metallicFactor);
                    rMaterial->m_material.roughnessFactor = static_cast<float>(material.pbrMetallicRoughness.roughnessFactor);
                    rMaterial->m_metallicRoughnessTexture = m_folder + "/" + m_file + "/" + std::to_string(m_instance) + "/" + std::to_string(material.pbrMetallicRoughness.metallicRoughnessTexture.index);

                    rMaterial->m_normalTexture = m_folder + "/" + m_file + "/" + std::to_string(m_instance) + "/" + std::to_string(material.normalTexture.index);

                    rMaterial->m_emissiveFactor = glm::vec3(material.emissiveFactor[0], material.emissiveFactor[1], material.emissiveFactor[2]);

                    for (const auto &ex: material.extensions) {
                        if (strcmp(ex.first.c_str(), "KHR_materials_emissive_strength") == 0) {
                            rMaterial->m_emissiveStrength = static_cast<float>(ex.second.Get("emissiveStrength").GetNumberAsDouble());
                        }
                    }

                    rMaterial->updateEmission();

                    rMaterial->m_key = keyPrefix + std::to_string(i);
                    rMaterial->m_name = material.name;

                    registerMaterial(rMaterial);
                }
            }

            // textures
            for (uint32_t i = 0; i < model.textures.size(); i++) {
                registerTexture(std::make_shared<RavenTexture>(m_folder + "/" + m_file + "/" + std::to_string(m_instance) + "/" + std::to_string(i), "RavenTexture", m_folder, model.images[model.textures[i].source].uri));
            }

            return node;
        }

    private:
        std::string m_folder;
        std::string m_file;
        glm::mat4 m_transformation;
        uint32_t m_instance;

        glm::mat4 createTransformationMatrix(const std::vector<double> &rotation,    // length must be 0 or 4
                                             const std::vector<double> &scale,       // length must be 0 or 3
                                             const std::vector<double> &translation, // length must be 0 or 3)
                                             const std::vector<double> &matrix) {    // length must be 0 or 16)
            if (!matrix.empty()) {
                glm::mat4 transformationMatrix(1.f);
                for (int i = 0; i < 16; i++) {
                    transformationMatrix[i / 4][i % 4] = static_cast<float>(matrix[i]);
                }
                return transformationMatrix;
            }

            glm::mat4 transformationMatrix(1.f);
            // scale
            if (scale.size() == 3) {
                transformationMatrix = glm::scale(glm::identity<glm::mat4>(), glm::vec3(scale[0], scale[1], scale[2]));
            } else {
                transformationMatrix = glm::scale(glm::identity<glm::mat4>(), glm::vec3(1));
            }
            // translation
            if (translation.size() == 3) {
                transformationMatrix = glm::translate(glm::identity<glm::mat4>(), glm::vec3(translation[0], translation[1], translation[2])) * transformationMatrix;
            }
            // rotation
            if (rotation.size() == 4) {
                float angle = 2 * static_cast<float>(glm::acos(rotation[3]));
                glm::vec3 axis = static_cast<float>(2 / glm::sin(angle)) * glm::vec3(rotation[0], rotation[1], rotation[2]);
                transformationMatrix = transformationMatrix * glm::rotate(glm::identity<glm::mat4>(), angle, axis);
            }
            return transformationMatrix;
        }

        std::shared_ptr<RavenSceneNode> loadScene(const tinygltf::Model &model, const tinygltf::Scene &scene) {
            auto sceneNode = std::make_shared<RavenSceneNode>();

            if (!scene.name.empty()) {
                sceneNode->m_name += "(" + scene.name + ")";
            }

            sceneNode->m_transformation = m_transformation;

            for (const auto &n: scene.nodes) {
                const auto &node = model.nodes[n];
                if (node.mesh < 0 && node.children.empty() || node.mesh >= 0 && !node.children.empty()) {
                    // skip empty nodes
                    continue;
                }
                sceneNode->m_children.push_back(loadNode(model, node));
            }

            return sceneNode;
        }

        std::shared_ptr<RavenSceneNode> loadNode(const tinygltf::Model &model, const tinygltf::Node &node) {
            if (node.mesh < 0 && node.children.empty()) {
                throw std::runtime_error("GLTF node is neither mesh node nor mesh collection node!");
            }
            if (node.mesh >= 0 && !node.children.empty()) {
                throw std::runtime_error("GLTF node is both mesh node and mesh collection node!");
            }

            auto sceneNode = std::make_shared<RavenSceneNode>();

            if (!node.name.empty()) {
                sceneNode->m_name += "(" + node.name + ")";
            }

            sceneNode->m_transformation = createTransformationMatrix(node.rotation, node.scale, node.translation, node.matrix);

            if (node.mesh >= 0) {
                const auto &mesh = model.meshes[node.mesh];
                if (mesh.primitives.size() == 1) {
                    sceneNode->m_object = loadPrimitive(model, mesh.primitives[0], mesh.name);
                } else {
                    for (const auto &primitive: mesh.primitives) {
                        auto child = std::make_shared<RavenSceneNode>();
                        child->m_name += "(" + mesh.name + ")";
                        child->m_object = loadPrimitive(model, primitive, mesh.name);
                        sceneNode->m_children.push_back(child);
                    }
                }
            }

            for (const auto &child: node.children) {
                const auto &childNode = model.nodes[child];
                if (childNode.mesh < 0 && childNode.children.empty() || childNode.mesh >= 0 && !childNode.children.empty()) {
                    continue;
                }
                sceneNode->m_children.push_back(loadNode(model, childNode));
            }

            return sceneNode;
        }

        std::shared_ptr<RavenSceneObject> loadPrimitive(const tinygltf::Model &model, const tinygltf::Primitive &primitive, const std::string &name) {
            auto sceneObject = std::make_shared<RavenSceneObject>();

            sceneObject->m_name += "(" + name + ")";

            // vertices
            bool tangents = false;
            bool textures = false;
            {
                uint32_t vertexCount = 0;
                tinygltf::BufferView bufferViewPosition, bufferViewNormal, bufferViewTangent, bufferViewTexCoord;
                for (const auto &attrib: primitive.attributes) {
                    std::string attribute = attrib.first;
                    int accessorIndex = attrib.second;
                    tinygltf::Accessor accessor;
                    if (strcmp(attribute.c_str(), "POSITION") == 0) {
                        accessor = model.accessors[accessorIndex];
                        bufferViewPosition = model.bufferViews[accessor.bufferView];
                        if (vertexCount > 0 && accessor.count != vertexCount) {
                            throw std::runtime_error("Count of vertices not the same for all attributes!");
                        }
                        vertexCount = accessor.count;
                    } else if (strcmp(attribute.c_str(), "NORMAL") == 0) {
                        accessor = model.accessors[accessorIndex];
                        bufferViewNormal = model.bufferViews[accessor.bufferView];
                        if (vertexCount > 0 && accessor.count != vertexCount) {
                            throw std::runtime_error("Count of vertices not the same for all attributes!");
                        }
                        vertexCount = accessor.count;
                    } else if (strcmp(attribute.c_str(), "TANGENT") == 0) {
                        accessor = model.accessors[accessorIndex];
                        bufferViewTangent = model.bufferViews[accessor.bufferView];
                        if (vertexCount > 0 && accessor.count != vertexCount) {
                            throw std::runtime_error("Count of vertices not the same for all attributes!");
                        }
                        vertexCount = accessor.count;
                        tangents = true;
                    } else if (strcmp(attribute.c_str(), "TEXCOORD_0") == 0) {
                        accessor = model.accessors[accessorIndex];
                        bufferViewTexCoord = model.bufferViews[accessor.bufferView];
                        if (vertexCount > 0 && accessor.count != vertexCount) {
                            throw std::runtime_error("Count of vertices not the same for all attributes!");
                        }
                        vertexCount = accessor.count;
                        textures = true;
                    } else if (strcmp(attribute.c_str(), "TEXCOORD_1") == 0) {
                        std::cout << "[WARNING] Unsupported TEXCOORD_1." << std::endl;
                    } else if (strcmp(attribute.c_str(), "COLOR_0") == 0) {
                        std::cout << "[WARNING] Unsupported COLOR_0." << std::endl;
                    } else {
                        throw std::runtime_error("Unknown attribute!");
                    }
                }
                sceneObject->m_vertices.resize(vertexCount);
                {
                    const auto &bufferView = bufferViewPosition;
                    const auto &buffer = model.buffers[bufferView.buffer];
                    const auto *data = reinterpret_cast<const glm::vec3 *>(buffer.data.data() + bufferView.byteOffset);
                    for (uint32_t i = 0; i < vertexCount; i++) {
                        sceneObject->m_vertices[i].position_x = data[i].x;
                        sceneObject->m_vertices[i].position_y = data[i].y;
                        sceneObject->m_vertices[i].position_z = data[i].z;
                        sceneObject->m_aabb.expand(data[i]);
                    }
                }
                {
                    const auto &bufferView = bufferViewNormal;
                    const auto &buffer = model.buffers[bufferView.buffer];
                    const auto *data = reinterpret_cast<const glm::vec3 *>(buffer.data.data() + bufferView.byteOffset);
                    for (uint32_t i = 0; i < vertexCount; i++) {
                        sceneObject->m_vertices[i].normal_x = data[i].x;
                        sceneObject->m_vertices[i].normal_y = data[i].y;
                        sceneObject->m_vertices[i].normal_z = data[i].z;
                    }
                }
                if (tangents) {
                    const auto &bufferView = bufferViewTangent;
                    const auto &buffer = model.buffers[bufferView.buffer];
                    const auto *data = reinterpret_cast<const glm::vec4 *>(buffer.data.data() + bufferView.byteOffset);
                    for (uint32_t i = 0; i < vertexCount; i++) {
                        sceneObject->m_vertices[i].tangent_x = data[i].x;
                        sceneObject->m_vertices[i].tangent_y = data[i].y;
                        sceneObject->m_vertices[i].tangent_z = data[i].z;
                        sceneObject->m_vertices[i].tangent_w = data[i].w;
                    }
                }
                if (textures) {
                    const auto &bufferView = bufferViewTexCoord;
                    const auto &buffer = model.buffers[bufferView.buffer];
                    const auto *data = reinterpret_cast<const glm::vec2 *>(buffer.data.data() + bufferView.byteOffset);
                    for (uint32_t i = 0; i < vertexCount; i++) {
                        sceneObject->m_vertices[i].texCoord_u = data[i].x;
                        sceneObject->m_vertices[i].texCoord_v = data[i].y;
                    }
                }
            }

            // indices
            {
                const auto &accessor = model.accessors[primitive.indices];
                assert(accessor.type == TINYGLTF_TYPE_SCALAR);
                const auto &bufferView = model.bufferViews[accessor.bufferView];
                uint32_t indexCount = accessor.count;
                sceneObject->m_indices.resize(indexCount);
                const auto &buffer = model.buffers[bufferView.buffer];
                if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    const auto *data = reinterpret_cast<const uint16_t *>(buffer.data.data() + accessor.byteOffset + bufferView.byteOffset);
                    for (uint32_t i = 0; i < indexCount; i++) {
                        sceneObject->m_indices[i] = data[i];
                    }
                } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    const auto *data = reinterpret_cast<const uint32_t *>(buffer.data.data() + accessor.byteOffset + bufferView.byteOffset);
                    for (uint32_t i = 0; i < indexCount; i++) {
                        sceneObject->m_indices[i] = data[i];
                    }
                } else {
                    throw std::runtime_error("Unsupported scalar type!");
                }
            }

            // material
            if (primitive.material >= 0) {
                sceneObject->m_material = m_folder + "/" + m_file + "/" + std::to_string(m_instance) + "/" + std::to_string(primitive.material);
            }

            // tangents
            if (!tangents) {
                for (uint32_t i = 0; i < sceneObject->m_indices.size(); i += 3) {
                    // for each vertex
                    auto &v0 = sceneObject->m_vertices[sceneObject->m_indices[i + 0]];
                    auto &v1 = sceneObject->m_vertices[sceneObject->m_indices[i + 1]];
                    auto &v2 = sceneObject->m_vertices[sceneObject->m_indices[i + 2]];

                    glm::vec3 p0 = glm::vec3(v0.position_x, v0.position_y, v0.position_z);
                    glm::vec3 p1 = glm::vec3(v1.position_x, v1.position_y, v1.position_z);
                    glm::vec3 p2 = glm::vec3(v2.position_x, v2.position_y, v2.position_z);

                    glm::vec3 n0 = glm::vec3(v0.normal_x, v0.normal_y, v0.normal_z);
                    glm::vec3 n1 = glm::vec3(v1.normal_x, v1.normal_y, v1.normal_z);
                    glm::vec3 n2 = glm::vec3(v2.normal_x, v2.normal_y, v2.normal_z);
                    glm::vec3 n = glm::normalize(n0 + n1 + n2);

                    glm::vec2 uv0 = glm::vec2(v0.texCoord_u, v0.texCoord_v);
                    glm::vec2 uv1 = glm::vec2(v1.texCoord_u, v1.texCoord_v);
                    glm::vec2 uv2 = glm::vec2(v2.texCoord_u, v2.texCoord_v);

                    // calculate tangent for the triangle
                    glm::vec3 deltaPos1 = p1 - p0;
                    glm::vec3 deltaPos2 = p2 - p0;

                    glm::vec2 deltaUV1 = uv1 - uv0;
                    glm::vec2 deltaUV2 = uv2 - uv0;

                    float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
                    glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;

                    // add the calculated tangent to all three vertices (if a vertex is shared by multiple triangles, the tangents will be averaged while larger triangles, i.e. larger tangents, contribute more)
                    v0.tangent_x += tangent.x;
                    v0.tangent_y += tangent.y;
                    v0.tangent_z += tangent.z;
                    v1.tangent_x += tangent.x;
                    v1.tangent_y += tangent.y;
                    v1.tangent_z += tangent.z;
                    v2.tangent_x += tangent.x;
                    v2.tangent_y += tangent.y;
                    v2.tangent_z += tangent.z;
                }
                for (auto &vertex: sceneObject->m_vertices) {
                    vertex.tangent_w = 1.f;
                }
            }

            return sceneObject;
        }
    };
} // namespace raven