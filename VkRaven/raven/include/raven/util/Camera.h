#pragma once

#include "Trajectory.h"


#ifdef WIN32
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#else
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif
#include <chrono>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/glm.hpp>

namespace raven {
    class Camera {
    public:
        void update(GLFWwindow *window, const float deltaTime) {
            static const float PI_HALF = glm::pi<float>() / 2.f;

            static const float CAMERA_POS_STEP = 3.f;
            static const float MOUSE_SENSITIVITY = 0.25f;

            bool centerMoved = false;
            glm::vec3 position;
            if (m_trajectory.step(&position)) {
                m_center = position;
                centerMoved = true;
            } else {
                float speed = 1.f;
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
                    speed = m_speedMultiplier;
                }

                float offsetX = 0.f;
                offsetX -= glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ? speed : 0.f;
                offsetX += glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? speed : 0.f;
                float offsetY = 0.f;
                offsetY -= glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS ? speed : 0.f;
                offsetY += glfwGetKey(window, GLFW_KEY_SPACE) || glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS ? speed : 0.f;
                float offsetZ = 0.f;
                offsetZ -= glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ? speed : 0.f;
                offsetZ += glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? speed : 0.f;

                if (offsetX != 0) {
                    offsetX *= CAMERA_POS_STEP * deltaTime;
                    m_center.x -= (float) glm::sin(m_rotation.y - PI_HALF) * offsetX;
                    m_center.z += (float) glm::cos(m_rotation.y - PI_HALF) * offsetX;
                }
                m_center.y += offsetY * CAMERA_POS_STEP * deltaTime;
                if (offsetZ != 0) {
                    offsetZ *= CAMERA_POS_STEP * deltaTime;
                    m_center.x -= (float) glm::sin(m_rotation.y) * offsetZ;
                    m_center.z += (float) glm::cos(m_rotation.y) * offsetZ;
                }

                centerMoved = offsetX != 0.0f || offsetY != 0.0f || offsetZ != 0.0f;
            }

            bool rotation = false;
            {
                glm::vec2 motion{0.f};
                double xpos, ypos;
                double *xposP = &xpos, *yposP = &ypos;
                glfwGetCursorPos(window, xposP, yposP);
                if (xposP != nullptr && yposP != nullptr) {
                    if (m_mousePreviousPosition.x > 0 && m_mousePreviousPosition.y > 0) {
                        motion.x = static_cast<float>(xpos) - m_mousePreviousPosition.x;
                        motion.y = static_cast<float>(ypos) - m_mousePreviousPosition.y;
                    }
                    m_mousePreviousPosition.x = static_cast<float>(xpos);
                    m_mousePreviousPosition.y = static_cast<float>(ypos);
                }
                if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                    move(0.f, -motion.x * MOUSE_SENSITIVITY * deltaTime, motion.y * MOUSE_SENSITIVITY * deltaTime);
                    rotation = motion.x != 0.0f || motion.y != 0.0f;
                }
            }

            update();

            if (centerMoved || rotation) {
                onCameraUpdate();
            }
        }

        void updateTrajectory() {
            bool centerMoved = false;
            glm::vec3 position;
            if (m_trajectory.step(&position)) {
                m_center = position;
                centerMoved = true;
            }

            update();

            if (centerMoved) {
                onCameraUpdate();
            }
        }

        void move(float offsetR, float offsetTheta, float offsetPhi) {
            static const auto PI = glm::pi<float>();
            static const float PI_HALF = glm::pi<float>() / 2.f;
            static const float TWO_PI = glm::pi<float>() * 2.f;

            m_r = glm::max(0.f, m_r + offsetR);
            m_theta = glm::mod(m_theta + offsetTheta + TWO_PI, TWO_PI);
            m_phi = glm::min(PI, glm::max(0.f, m_phi + offsetPhi));
        }

        void moveCenter(glm::vec3 center) {
            m_center = center;
            update();
        }

        void setRotation(float r, float theta, float phi) {
            m_r = r;
            m_theta = theta;
            m_phi = phi;
            update();
        }

        void fromSphericalCoordinates(glm::vec3 center, float r, float theta, float phi) {
            m_center = center;
            m_r = r;
            m_theta = theta;
            m_phi = phi;
            update();
        }

        static glm::vec3 ndcToWorldSpace(float ndcX, float ndcY, glm::mat4 inverseProjection, glm::mat4 inverseView) {
            glm::vec4 clipCoordinates = glm::vec4(ndcX, ndcY, -1.0, 1.0);
            glm::vec4 viewCoordinates = inverseProjection * clipCoordinates;
            viewCoordinates.z = -1.0;
            viewCoordinates.w = 0.0;
            glm::vec4 worldCoordinates = inverseView * viewCoordinates;
            return glm::normalize(glm::vec3(worldCoordinates));
        }

        glm::mat4 getWorldToViewSpace() {
            return glm::rotate(glm::identity<glm::mat4>(), m_rotation.x, glm::vec3(1, 0, 0)) * glm::rotate(glm::identity<glm::mat4>(), m_rotation.y, glm::vec3(0, 1, 0)) * glm::translate(glm::identity<glm::mat4>(), -m_position);
        }

        [[nodiscard]] glm::mat4 getViewToClipSpace() const {
            glm::mat4 proj = glm::perspective(m_fovy, m_aspect, m_zNear, m_zFar);
            proj[1][1] *= -1;
            return proj;
        }

        void setAspect(float aspect) {
            m_aspect = aspect;
        }

        [[nodiscard]] static float extentToAspectRatio(vk::Extent2D extent) {
            return static_cast<float>(extent.width) / static_cast<float>(extent.height);
        }

        glm::vec3 getPosition() {
            return m_position;
        }

        [[nodiscard]] glm::vec3 getSphericalRotation() const {
            return {m_r, m_theta, m_phi};
        }

        void registerCameraUpdateCallback(std::function<void()> callbackFunction) {
            m_cameraUpdateFunction = std::move(callbackFunction);
        }

        void onCameraUpdate() {
            if (m_cameraUpdateFunction) {
                m_cameraUpdateFunction();
            }
        }

        void initTrajectory(glm::vec3 start, glm::vec3 end, uint32_t steps) {
            m_trajectory.init(start, end, steps);
        }

        void setSpeedMultiplier(const float speedMultiplier) {
            m_speedMultiplier = speedMultiplier;
        }

    private:
        glm::vec3 m_center{0};
        float m_r = 0.f;
        float m_theta = 0.f;
        float m_phi = 0.f;

        void update() {
            float phi = m_phi - glm::pi<float>() / 2.f;
            float sinTheta = glm::sin(m_theta);
            float cosTheta = glm::cos(m_theta);
            float sinPhi = glm::sin(phi);
            float cosPhi = glm::cos(phi);
            m_position = m_center + glm::vec3(m_r * sinTheta * cosPhi, m_r * sinPhi, m_r * cosTheta * cosPhi);
            m_rotation.x = phi;
            m_rotation.y = -m_theta;
        }

        glm::vec3 m_position{0};
        glm::vec2 m_rotation{0};

        float m_fovy = 0.33f * glm::pi<float>();
        float m_aspect{};
        float m_zNear = 0.05f;
        float m_zFar = 1000.0f;

        glm::vec2 m_mousePosition = glm::vec2(0.f);
        glm::vec2 m_mousePreviousPosition = glm::vec2(-1.f, -1.f);
        bool m_rightButtonPressed = false;

        std::function<void()> m_cameraUpdateFunction = nullptr;

        Trajectory m_trajectory{};

        float m_speedMultiplier = 10.f;
    };
} // namespace raven