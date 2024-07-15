#pragma once
#include "RavenSceneObject.h"


#include <memory>
#include <optional>

namespace raven {
    class RavenSceneNode {
    public:
        std::string m_name = "RavenSceneNode";

        std::optional<glm::mat4> m_transformation;

        std::vector<std::shared_ptr<RavenSceneNode>> m_children;
        std::shared_ptr<RavenSceneObject> m_object = nullptr;


    };
} // namespace raven