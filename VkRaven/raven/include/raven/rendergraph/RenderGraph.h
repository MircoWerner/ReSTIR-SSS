#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "raven/rendergraph/RenderGraphPass.h"
#include "raven/rendergraph/RenderGraphResource.h"

namespace raven {
    class RenderGraph {
    public:
        std::string m_name;

        std::vector<std::vector<RenderGraphPass>> m_executionOrder{};

        static RenderGraph create(const std::string &name) {
            return RenderGraph(name);
        }

        RenderGraph &addRenderPass(const RenderGraphPass &renderPass) {
            if (m_renderPasses.contains(renderPass.m_name)) {
                throw std::runtime_error("Render pass with name " + renderPass.m_name + " already exists.");
            }
            m_renderPasses.insert({renderPass.m_name, renderPass});
            return *this;
        }

        RenderGraphResource &addRenderResource(const std::string &name) {
            if (m_renderResources.contains(name)) {
                throw std::runtime_error("Render resource with name " + name + " already exists.");
            }
            m_renderResources.insert({name, RenderGraphResource(name)});
            return m_renderResources.at(name);
        }

        void compile() {
            deadStrip();
            topologySort();
        }

        void execute() {
            bool first = true;
            std::cout << "Execute..." << std::endl;
            for (const auto &renderPasses: m_executionOrder) {
                if (!first) {
                    std::cout << "      <<barrier>>" << std::endl;
                }
                first = false;
                for (const auto &renderPass: renderPasses) {
                    std::cout << "      Pass " << renderPass.m_name << "... ";
                    renderPass.execute();
                }
            }
            std::cout << "Execute finished..." << std::endl;
        }

        friend std::ostream &operator<<(std::ostream &stream, const RenderGraph &object) {
            stream << "RenderGraph{ name=" << object.m_name << "\n passes={\n";
            for (const auto &renderPass: object.m_renderPasses) {
                stream << "    " << renderPass.second << "\n";
            }
            stream << " resources={\n";
            for (const auto &renderResource: object.m_renderResources) {
                stream << "    " << renderResource.second << "\n";
            }
            stream << "}";
            return stream;
        }

    private:
        explicit RenderGraph(std::string name) : m_name(std::move(name)) {}

        std::unordered_map<std::string, RenderGraphPass> m_renderPasses{};

        std::unordered_map<std::string, RenderGraphResource> m_renderResources{};

        void deadStrip() {
            std::ranges::for_each(std::views::values(m_renderPasses), [](RenderGraphPass &renderPass) {
                renderPass.m_visited = false;
            });

            std::ranges::for_each(std::views::values(m_renderPasses), [&](RenderGraphPass &renderPass) {
                if (renderPass.m_root && !renderPass.m_visited) {
                    deadStripRecursive(renderPass);
                }
            });

            size_t erased = std::erase_if(m_renderPasses, [](const auto &renderPass) {
                return !renderPass.second.m_visited;
            });
            std::cout << "Erased " << erased << " render passes." << std::endl;

            // === erase resources ===
            std::ranges::for_each(std::views::values(m_renderResources), [&](RenderGraphResource &renderResource) {
                std::erase_if(renderResource.m_dependencies, [&](const auto &dependency) {
                    return !m_renderPasses.contains(dependency.m_name);
                });
            });

            erased = std::erase_if(m_renderResources, [&](const auto &renderResourcePair) {
                const auto &[name, renderResource] = renderResourcePair;
                if (renderResource.m_dependencies.empty()) {
                    return true;
                }
                bool onlyNonRootWrites = true;
                for (const auto &dependency: renderResource.m_dependencies) {
                    if (dependency.m_type == RenderGraphResourceDependency::READ || m_renderPasses.at(dependency.m_name).m_root) {
                        onlyNonRootWrites = false;
                        break;
                    }
                }
                return onlyNonRootWrites;
            });
            std::cout << "Erased " << erased << " render resources." << std::endl;

            // === clean up pass dependencies ===
            std::ranges::for_each(std::views::values(m_renderPasses), [&](RenderGraphPass &renderPass) {
                std::erase_if(renderPass.m_dependencies, [&](const auto &dependency) {
                    return !m_renderResources.contains(dependency.m_name);
                });
            });
        }

        void deadStripRecursive(RenderGraphPass &renderPass) {
            if (renderPass.m_visited) {
                return;
            }
            renderPass.m_visited = true;

            for (const auto &resourceDependency: renderPass.m_dependencies) {
                if (resourceDependency.m_type == RenderGraphResourceDependency::WRITE) {
                    continue;
                }

                for (const auto &renderPassDependency: m_renderResources.at(resourceDependency.m_name).m_dependencies) {
                    if (renderPassDependency.m_type == RenderGraphResourceDependency::READ || resourceDependency.m_version == 0 || resourceDependency.m_version - renderPassDependency.m_version != 1) {
                        continue;
                    }

                    deadStripRecursive(m_renderPasses.at(renderPassDependency.m_name));
                }
            }
        }

        void topologySort() {
            std::unordered_map<std::string, RenderGraphPass> renderPasses;
            std::ranges::for_each(std::views::values(m_renderPasses), [&](const RenderGraphPass &renderPass) {
                renderPasses.insert({renderPass.m_name, renderPass});
            });

            const uint32_t maxIterations = renderPasses.size();
            uint32_t iteration = 0;

            std::unordered_map<std::string, uint32_t> resourceVersions{};
            for (const auto &renderResource: std::views::values(m_renderResources)) {
                resourceVersions.insert({renderResource.m_name, 0});
            }

            while (!renderPasses.empty()) {
                if (iteration++ >= maxIterations) {
                    throw std::runtime_error("Too many iterations, something is wrong with the render graph.");
                }

                // query next possible passes that have no read dependencies
                std::set<std::string> nextPasses{};
                for (const auto &renderPass: std::views::values(renderPasses)) {
                    if (!renderPass.hasReadDependencies(resourceVersions)) {
                        nextPasses.insert(renderPass.m_name);
                    }
                }

                std::unordered_set<std::string> erasePasses{};
                while (!nextPasses.empty()) {
                    std::string nextPass;
                    for (const auto &pass: nextPasses) {
                        // check if pass removal contradicts any other pass
                        std::unordered_map<std::string, uint32_t> resourceVersionsCopy = resourceVersions;
                        for (const auto &resourceDependency: renderPasses.at(pass).m_dependencies) {
                            if (resourceDependency.m_type == RenderGraphResourceDependency::WRITE) {
                                resourceVersionsCopy.at(resourceDependency.m_name) = std::max(resourceVersionsCopy.at(resourceDependency.m_name), resourceDependency.m_version + 1);
                            }
                        }
                        bool contradicts = false;
                        for (const auto &otherPass : nextPasses) {
                            if (otherPass == pass) {
                                continue;
                            }
                            if (!renderPasses.at(otherPass).isFutureExecutionPossible(resourceVersionsCopy)) {
                                contradicts = true;
                                break;
                            }
                        }
                        if (contradicts) {
                            continue;
                        }
                        for (const auto &otherPass : erasePasses) {
                            if (otherPass == pass) {
                                continue;
                            }
                            if (!renderPasses.at(otherPass).isFutureExecutionPossible(resourceVersionsCopy)) {
                                contradicts = true;
                                break;
                            }
                        }
                        if (contradicts) {
                            continue;
                        }

                        nextPass = pass;
                        break;
                    }
                    if (nextPass.empty()) {
                        break;
                    }
                    nextPasses.erase(nextPass);
                    erasePasses.insert(nextPass);
                }

                if (erasePasses.empty()) {
                    throw std::runtime_error("Invalid render graph, no render pass can be executed next.");
                }

                m_executionOrder.emplace_back();
                for (const auto &pass : erasePasses) {
                    m_executionOrder.back().push_back(renderPasses.at(pass));

                    for (const auto &resourceDependency: renderPasses.at(pass).m_dependencies) {
                        if (resourceDependency.m_type == RenderGraphResourceDependency::WRITE) {
                            resourceVersions.at(resourceDependency.m_name) = std::max(resourceVersions.at(resourceDependency.m_name), resourceDependency.m_version + 1);
                        }
                    }
                }

                std::erase_if(renderPasses, [&](const auto &renderPass) {
                    return erasePasses.contains(renderPass.first);
                });
            }
        }
    };
} // namespace raven