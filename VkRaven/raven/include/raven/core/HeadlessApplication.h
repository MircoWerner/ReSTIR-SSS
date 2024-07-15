#pragma once

#include <memory>

#include "Buffer.h"
#include "GPUContext.h"
#include "Renderer.h"
#include "Texture.h"
#include "raven/util/Camera.h"

namespace raven {
    class HeadlessApplication : public GPUContext {
    public:
        struct ApplicationSettings {
            vk::SampleCountFlagBits m_msaaSamples = vk::SampleCountFlagBits::e1;

            /*
             * RayTracing
             * VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME
             */
            std::vector<const char *> m_deviceExtensions = {};

            int32_t m_width = 1920;
            int32_t m_height = 1080;

            uint32_t m_rendererExecutions = 1;

            std::string m_directory;
            std::string m_name;

            std::function<void(GPUContext *)> m_executeAfterInit = [](GPUContext *) {};

            bool m_png;
        };

        HeadlessApplication(std::string &appName, ApplicationSettings appSettings, std::shared_ptr<Renderer> renderer, uint32_t requiredQueueFamilies);

        virtual void run();

        Camera *getCamera() override {
            return m_camera.get();
        }

    protected:
        virtual void getDeviceExtensions(std::vector<const char *> &extensions) const override;

        std::string m_appName;
        ApplicationSettings m_applicationSettings;

        std::shared_ptr<Camera> m_camera;

        std::shared_ptr<Renderer> m_renderer;

        Image *m_image = nullptr;

        void init() override;

        virtual void loop();
        virtual void finish();
        virtual void render();

        void shutdown() override;

        void assertMSAASamplesSupported(vk::SampleCountFlagBits msaaSamples);

        uint32_t m_queueFamilyIndex{};

        uint32_t findQueueFamilyIndex() {
            Queues::QueueFamilyIndices queueFamilyIndices = m_queues->findQueueFamilies(m_physicalDevice);
            return queueFamilyIndices.graphicsFamily.value();
        }

        [[nodiscard]] std::string prefix() const {
            return "[" + m_applicationSettings.m_name + "] ";
        }
    };
} // namespace raven
