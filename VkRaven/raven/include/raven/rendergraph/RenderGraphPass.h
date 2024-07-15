#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>
#include <memory>

#include "raven/rendergraph/RenderGraphResource.h"

namespace raven {
    enum RenderGraphPassType {
        GRAPHICS,
        COMPUTE,
        TRANSFER
    };

    class RenderGraphPass {
    public:
        void execute() const {
            m_executeFunction();
        }

        friend std::ostream &operator<<(std::ostream &stream, const RenderGraphPass &object) {
            stream << "RenderGraphPass{ name=" << object.m_name << " type=" << object.m_type << " root=" << object.m_root << "\n     dependencies={ \n";
            for (const auto &dependency: object.m_dependencies) {
                stream << "         " << dependency << "\n";
            }
            stream << "     }"
                   << "\n    }";
            return stream;
        }

    private:
        friend class RenderGraphPassBuilder;
        friend class RenderGraph;

        RenderGraphPass(std::string name, const RenderGraphPassType type, const bool root) : m_name(std::move(name)), m_type(type), m_root(root) {}

        std::string m_name;
        RenderGraphPassType m_type;
        bool m_root = false;

        bool m_visited = false;
        std::vector<RenderGraphResourceDependency> m_dependencies{};

        std::function<void()> m_executeFunction = []() { throw std::runtime_error("RenderGraphPass::m_executeFunction not set."); };

        [[nodiscard]] bool hasReadDependencies(const std::unordered_map<std::string, uint32_t> &resourceVersions) const {
            bool hasReadDependencies = false;
            for (const auto &dependency: m_dependencies) {
                if (dependency.m_type == RenderGraphResourceDependency::READ) {
                    const auto resourceVersion = resourceVersions.at(dependency.m_name);
                    if (resourceVersion > dependency.m_version) {
                        throw std::runtime_error("This render pass should have been executed already.");
                    }
                    if (resourceVersion < dependency.m_version) {
                        hasReadDependencies = true;
                        break;
                    }
                }
            }
            return hasReadDependencies;
        }

        [[nodiscard]] bool isFutureExecutionPossible(const std::unordered_map<std::string, uint32_t> &resourceVersions) const {
            for (const auto &dependency: m_dependencies) {
                if (dependency.m_type == RenderGraphResourceDependency::READ) {
                    const auto resourceVersion = resourceVersions.at(dependency.m_name);
                    if (resourceVersion > dependency.m_version) {
                        return false;
                    }
                }
            }
            return true;
        }
    };

    class RenderGraphPassBuilder {
    public:
        RenderGraphPassBuilder(const std::string &name, const RenderGraphPassType type, bool root = false) : m_renderGraphPass{new RenderGraphPass(name, type, root)} {
        }

        [[nodiscard]] RenderGraphPass build() const {
            return std::move(*m_renderGraphPass);
        }

        RenderGraphPassBuilder &readResource(RenderGraphResource &renderResource) {
            m_renderGraphPass->m_dependencies.emplace_back(renderResource.m_name, renderResource.m_version, RenderGraphResourceDependency::READ);
            renderResource.m_dependencies.emplace_back(m_renderGraphPass->m_name, renderResource.m_version, RenderGraphResourceDependency::READ);
            return *this;
        }

        RenderGraphPassBuilder &writeResource(RenderGraphResource &renderResource) {
            m_renderGraphPass->m_dependencies.emplace_back(renderResource.m_name, renderResource.m_version, RenderGraphResourceDependency::WRITE);
            renderResource.m_dependencies.emplace_back(m_renderGraphPass->m_name, renderResource.m_version, RenderGraphResourceDependency::WRITE);
            renderResource.m_version++;
            return *this;
        }

        RenderGraphPassBuilder &execute(std::function<void()> executeFunction) {
            m_renderGraphPass->m_executeFunction = std::move(executeFunction);
            return *this;
        }

    private:
        std::unique_ptr<RenderGraphPass> m_renderGraphPass;
    };
} // namespace raven