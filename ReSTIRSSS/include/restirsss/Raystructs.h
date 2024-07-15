#pragma once

#include <cstdint>

namespace raven {
    struct ObjectDescriptor {
        uint64_t vertexAddress{};
        uint64_t indexAddress{};
        uint32_t materialId = 0;

        glm::mat4 modelMatrix = glm::mat4(1.f);
        glm::mat4 objectToWorldNormal = glm::mat4(1.f);
    };
} // namespace raven