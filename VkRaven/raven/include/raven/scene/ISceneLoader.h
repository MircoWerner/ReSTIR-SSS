#pragma once
#include "RavenLight.h"
#include "RavenMaterial.h"
#include "RavenSceneNode.h"
#include "RavenTexture.h"


#include <functional>

namespace raven {
    class ISceneLoader {
    protected:
        ~ISceneLoader() = default;

    public:
        virtual std::shared_ptr<RavenSceneNode> loadScene(std::function<void(const std::shared_ptr<RavenMaterial>&)> registerMaterial,
                                                          std::function<void(const std::shared_ptr<RavenTexture>&)> registerTexture,
                                                          std::function<void(const std::shared_ptr<RavenLight>&)> registerLight) = 0;
    };
} // namespace raven