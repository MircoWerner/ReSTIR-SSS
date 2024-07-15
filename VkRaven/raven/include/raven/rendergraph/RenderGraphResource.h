#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

namespace raven {
    struct RenderGraphResourceDependency {
        enum RenderGraphResourceDependencyType {
            READ,
            WRITE
        };

        std::string m_name; // either name of related resource or render pass
        uint32_t m_version;
        RenderGraphResourceDependencyType m_type;

        friend std::ostream &operator<<(std::ostream &stream, const RenderGraphResourceDependency &object) {
            stream << "RenderGraphResourceDependency{ name=" << object.m_name << " version=" << object.m_version << " type=" << (object.m_type == READ ? "R" : "W") << " }";
            return stream;
        }
    };

    class RenderGraphResource {
    public:
        explicit RenderGraphResource(std::string name) : m_name(std::move(name)) {}

    private:
        std::string m_name;
        uint32_t m_version = 0;

        std::vector<RenderGraphResourceDependency> m_dependencies{};

        friend std::ostream &operator<<(std::ostream &stream, const RenderGraphResource &object) {
            stream << "RenderGraphResource{ name=" << object.m_name << " nextVersion=" << object.m_version << "\n     dependencies={ \n";
            for (const auto &dependency: object.m_dependencies) {
                stream << "         " << dependency << "\n";
            }
            stream << "     }"
                   << "\n    }";
            return stream;
        }

        friend class RenderGraphPass;
        friend class RenderGraphPassBuilder;
        friend class RenderGraph;
    };
} // namespace raven
