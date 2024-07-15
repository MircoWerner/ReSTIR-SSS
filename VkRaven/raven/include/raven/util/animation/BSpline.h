#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace raven {
    class BSpline {
    public:
        [[nodiscard]] glm::vec3 evaluate(float u) const {
            const int interval = static_cast<int>(glm::floor(u));
            const float t = glm::fract(u);

            assert(interval >= 0 && interval < m_controlPoints.size() - 3);

            const glm::vec3 p0 = m_controlPoints[interval];
            const glm::vec3 p1 = m_controlPoints[interval + 1];
            const glm::vec3 p2 = m_controlPoints[interval + 2];
            const glm::vec3 p3 = m_controlPoints[interval + 3];

            const float t2 = t * t;
            const float t3 = t2 * t;

            return 1.f / 6.f * ((-p0 + 3.f * p1 - 3.f * p2 + p3) * t3 + (3.f * p0 - 6.f * p1 + 3.f * p2) * t2 + (-3.f * p0 + 3.f * p2) * t + (p0 + 4.f * p1 + p2));
        }

        [[nodiscard]] float getMaxU() const {
            return static_cast<float>(static_cast<int>(m_controlPoints.size()) - 3);
        }

        std::vector<glm::vec3> m_controlPoints;
    };
} // namespace raven