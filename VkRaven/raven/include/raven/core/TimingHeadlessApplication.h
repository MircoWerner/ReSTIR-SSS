#pragma once

#include <memory>

#include "Buffer.h"
#include "GPUContext.h"
#include "HeadlessApplication.h"
#include "Renderer.h"
#include "Texture.h"
#include "raven/util/Camera.h"

namespace raven {
    class TimingHeadlessApplication : public HeadlessApplication {
    public:
        struct TimingApplicationSettings {
            uint32_t m_timingExecutions = 1;

            uint32_t m_startupExecutions = 64;

            std::function<void(GPUContext *)> m_executeAfterStartupExecutions = [](GPUContext *) {};
            std::function<void(std::ofstream &stream)> m_executeQueryTiming = [](std::ofstream &) {};
            std::function<void(uint32_t)> m_executeEachFrame = [](uint32_t) {};
        };

        TimingHeadlessApplication(std::string &appName, ApplicationSettings appSettings, TimingApplicationSettings timingAppSettings, std::shared_ptr<Renderer> renderer, uint32_t requiredQueueFamilies);

        void run() override;

    protected:
        TimingApplicationSettings m_timingApplicationSettings;

        void loop() override;
        void finish() override;
        void render() override;

    private:
        uint32_t m_timingExecution = 0;
    };
} // namespace raven
