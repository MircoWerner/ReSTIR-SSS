#pragma once

#include <glm/glm.hpp>
#include <string>

namespace raven {
    class RavenMaterial {
    public:
        struct Material {
            glm::vec3 baseColorFactor = glm::vec4(1.f);
            int32_t baseColorTexture = -1;
            float metallicFactor = 1.f;
            float roughnessFactor = 1.f;
            int32_t metallicRoughnessTexture = -1;

            int32_t normalTexture = -1;

            glm::vec3 emission = glm::vec3(0.f);

            float subsurface = 0.f; // [0,1], 0 no sss, 1 only sss
            glm::vec3 meanFreePath = glm::vec3(0.f); // l

            [[nodiscard]] bool isEmissive() const {
                return glm::any(glm::greaterThan(emission, glm::vec3(0.f)));
            }

            void mfpFromVolumetricCoefficients(glm::vec3 scatteringCoefficient, glm::vec3 absorptionCoefficient) {
                glm::vec3 extinctionCoefficient = scatteringCoefficient + absorptionCoefficient;
                meanFreePath = 1.f / extinctionCoefficient;
            }

            [[nodiscard]] glm::vec3 scatteringScaling(glm::vec3 surfaceAlbedo) const {
                glm::vec3 a = surfaceAlbedo - glm::vec3(0.8f);
                return 1.9f - surfaceAlbedo + 3.5f * a * a;
            }

            [[nodiscard]] glm::vec3 scatteringDistance(glm::vec3 surfaceAlbedo) const {
                return subsurface * meanFreePath / scatteringScaling(surfaceAlbedo);
            }
        };

        std::string m_key{};
        std::string m_name = "RavenMaterial";
        Material m_material{};
        glm::vec3 m_emissiveFactor = glm::vec3(0.f);
        float m_emissiveStrength = 0.f;

        void setEmission(const glm::vec3 emissiveFactor, const float emissiveStrength) {
            m_emissiveFactor = emissiveFactor;
            m_emissiveStrength = emissiveStrength;
            updateEmission();
        }
        void setEmission(const glm::vec3 emissiveFactor) {
            m_emissiveFactor = emissiveFactor;
            updateEmission();
        }
        void setEmission(const float emissiveStrength) {
            m_emissiveStrength = emissiveStrength;
            updateEmission();
        }
        void updateEmission() {
            m_material.emission = m_emissiveFactor * m_emissiveStrength;
        }

        std
        ::string m_baseColorTexture{};
        std::string m_metallicRoughnessTexture{};
        std::string m_normalTexture{};

        void updateTextureIds(const std::function<int32_t(const std::string &name)> &textureIdFromName) {
            m_material.baseColorTexture = textureIdFromName(m_baseColorTexture);
            m_material.metallicRoughnessTexture = textureIdFromName(m_metallicRoughnessTexture);
            m_material.normalTexture = textureIdFromName(m_normalTexture);
        }
    };
} // namespace RavenMaterial