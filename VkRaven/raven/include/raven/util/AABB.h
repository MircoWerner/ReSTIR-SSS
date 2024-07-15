#pragma once

#include <glm/glm.hpp>

namespace raven {

    struct AABB {
        glm::vec3 m_min = glm::vec3(FLT_MAX);
        glm::vec3 m_max = glm::vec3(-FLT_MAX);

        void expand(const glm::vec3 vec) {
            if (m_min.x > vec.x) {
                m_min.x = vec.x;
            }
            if (m_min.y > vec.y) {
                m_min.y = vec.y;
            }
            if (m_min.z > vec.z) {
                m_min.z = vec.z;
            }

            if (m_max.x < vec.x) {
                m_max.x = vec.x;
            }
            if (m_max.y < vec.y) {
                m_max.y = vec.y;
            }
            if (m_max.z < vec.z) {
                m_max.z = vec.z;
            }
        }

        [[nodiscard]] float calculateVolume() const {
            if (m_min.x >= m_max.x || m_min.y >= m_max.y || m_min.z >= m_max.z) {
                return 0.f;
            }
            return (m_max.x - m_min.x) * (m_max.y - m_min.y) * (m_max.z - m_min.z);
        }

        [[nodiscard]] float m_maxExtent() const { return glm::max(0.f, glm::max(m_max.x - m_min.x, glm::max(m_max.y - m_min.y, m_max.z - m_min.z))); };

        [[nodiscard]] float m_maxExtentAxis() const {
            const float xExtent = m_max.x - m_min.x;
            const float yExtent = m_max.y - m_min.y;
            const float zExtent = m_max.z - m_min.z;
            if (xExtent > yExtent && xExtent > zExtent) {
                return 0.f;
            }
            return yExtent > zExtent ? 1 : 2;
        }

        [[nodiscard]] float maxElement() const { return glm::max(m_max.x, glm::max(m_max.y, m_max.z)); }

        [[nodiscard]] float minElement() const { return glm::min(m_min.x, glm::min(m_min.y, m_min.z)); }

        [[nodiscard]] glm::vec3 center() const {
            return .5f * (m_min + m_max);
        }

        [[nodiscard]] glm::vec3 halfSize() const {
            return .5f * (m_max - m_min);
        }

        friend std::ostream &operator<<(std::ostream &stream, const AABB &aabb) {
            stream << "AABB{ m_min=(" << aabb.m_min.x << "," << aabb.m_min.y << "," << aabb.m_min.z << "), m_max=(" << aabb.m_max.x << "," << aabb.m_max.y << "," << aabb.m_max.z << ") }";
            return stream;
        }
    };

    struct iAABB {
        glm::ivec3 m_min = glm::ivec3(INT32_MAX);
        glm::ivec3 m_max = glm::ivec3(INT32_MIN);

        void expand(const glm::ivec3 vec) {
            if (m_min.x > vec.x) {
                m_min.x = vec.x;
            }
            if (m_min.y > vec.y) {
                m_min.y = vec.y;
            }
            if (m_min.z > vec.z) {
                m_min.z = vec.z;
            }

            if (m_max.x < vec.x) {
                m_max.x = vec.x;
            }
            if (m_max.y < vec.y) {
                m_max.y = vec.y;
            }
            if (m_max.z < vec.z) {
                m_max.z = vec.z;
            }
        }

        [[nodiscard]] int32_t calculateVolume() const {
            if (m_min.x >= m_max.x || m_min.y >= m_max.y || m_min.z >= m_max.z) {
                return 0;
            }
            return (m_max.x - m_min.x) * (m_max.y - m_min.y) * (m_max.z - m_min.z);
        }

        [[nodiscard]] uint32_t maxExtent() const { return glm::max(0, glm::max(m_max.x - m_min.x, glm::max(m_max.y - m_min.y, m_max.z - m_min.z))); };

        [[nodiscard]] int32_t maxExtentAxis() const {
            const int32_t xExtent = m_max.x - m_min.x;
            const int32_t yExtent = m_max.y - m_min.y;
            const int32_t zExtent = m_max.z - m_min.z;
            if (xExtent > yExtent && xExtent > zExtent) {
                return 0;
            }
            return yExtent > zExtent ? 1 : 2;
        }

        [[nodiscard]] int32_t maxElement() const { return glm::max(m_max.x, glm::max(m_max.y, m_max.z)); }

        [[nodiscard]] int32_t minElement() const { return glm::min(m_min.x, glm::min(m_min.y, m_min.z)); }

        [[nodiscard]] glm::vec3 center() const {
            return .5f * glm::vec3(m_min + m_max);
        }

        [[nodiscard]] glm::vec3 halfSize() const {
            return .5f * glm::vec3(m_max - m_min);
        }

        friend std::ostream &operator<<(std::ostream &stream, const iAABB &aabb) {
            stream << "iAABB{ m_min=(" << aabb.m_min.x << "," << aabb.m_min.y << "," << aabb.m_min.z << "), m_max=(" << aabb.m_max.x << "," << aabb.m_max.y << "," << aabb.m_max.z << ") }";
            return stream;
        }
    };

} // namespace raven