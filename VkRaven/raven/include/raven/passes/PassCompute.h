#pragma once

#include "Pass.h"

namespace raven {
    class PassCompute final : public Pass {
    public:
        PassCompute(GPUContext *gpuContext, PassSettings settings) : Pass(gpuContext, std::move(settings)) {}

    protected:
        void findQueueFamily() override {
            Queues::QueueFamilyIndices queueFamilyIndices = m_gpuContext->m_queues->findQueueFamilies(m_gpuContext->m_physicalDevice);
            m_queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
            m_queueType = Queues::GRAPHICS;
        }

    private:
    };
} // namespace raven