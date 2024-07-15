#pragma once
#include <glm/vec3.hpp>

namespace raven {
    class Trajectory {
    public:
        Trajectory() = default;

        void init(glm::vec3 start, glm::vec3 end, uint32_t steps) {
            m_start = start;
            m_end = end;
            m_steps = steps;
            m_stepCount = 0;
        }

        bool step(glm::vec3 *position) {
            if (m_stepCount >= m_steps) {
                return false;
            }

            m_stepCount++;

            float t = static_cast<float>(m_stepCount) / static_cast<float>(m_steps);
            *position = m_start + t * (m_end - m_start);

            return true;
        }

    private:
        glm::vec3 m_start{};
        glm::vec3 m_end{};
        uint32_t m_steps{};

        uint32_t m_stepCount = 0;
    };
}