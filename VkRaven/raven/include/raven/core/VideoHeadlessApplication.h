#pragma once

#include <memory>

#include "Buffer.h"
#include "GPUContext.h"
#include "HeadlessApplication.h"
#include "Renderer.h"
#include "Texture.h"
#include "raven/util/Camera.h"

namespace raven {
    class VideoHeadlessApplication : public HeadlessApplication {
    public:
        struct VideoApplicationSettings {
            std::function<void(GPUContext *)> m_executeAfterStartupExecutions = [](GPUContext *) {};
            std::function<void(uint32_t)> m_executeEachFrame = [](uint32_t) {};

            uint32_t m_framePerExecution = 1;
        };

        VideoHeadlessApplication(std::string &appName, ApplicationSettings appSettings, VideoApplicationSettings videoAppSettings, std::shared_ptr<Renderer> renderer, uint32_t requiredQueueFamilies);

        void run() override;

    protected:
        VideoApplicationSettings m_videoApplicationSettings;

        void loop() override;
        void finish() override;
        void render() override;

    private:
        void storeImage(uint32_t execution);
    };
} // namespace raven
