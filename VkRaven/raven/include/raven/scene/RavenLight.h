#pragma once

namespace raven {
    class RavenLight {
    public:
#define LIGHT_TYPE_AREA 0
#define LIGHT_TYPE_POINT 1
        struct Light {
            uint32_t type = LIGHT_TYPE_AREA;

            uint32_t objectDescriptorId = 0;
            uint32_t triangleId = 0;

            glm::vec3 position = glm::vec3(0);
            glm::vec3 emission = glm::vec3(1);
        };

        std::string m_key{};
        std::string m_name = "RavenLight";
        Light m_light{};
    };
} // namespace raven