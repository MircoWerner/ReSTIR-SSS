cmake_minimum_required(VERSION 3.18)
project(ravencore VERSION 0.1.0 DESCRIPTION "VkRaven Core - Vulkan Render and Visualization Engine Core" LANGUAGES CXX)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(Vulkan REQUIRED)

set(RAVENCORE_HEADERS
        include/raven/RavenInclude.h
        include/raven/core/Renderer.h
        include/raven/core/Application.h include/raven/core/HeadlessApplication.h include/raven/core/TimingHeadlessApplication.h include/raven/core/VideoHeadlessApplication.h
        include/raven/core/GPUContext.h include/raven/core/Queues.h include/raven/core/Shader.h
        include/raven/core/RavenVkDynamicLoader.h
        include/raven/util/AABB.h include/raven/util/Paths.h include/raven/core/Buffer.h include/raven/core/Image.h include/raven/core/Texture.h include/raven/passes/ImGuiPass.h include/raven/core/Uniform.h
        include/raven/core/SwapChain.h include/raven/util/Camera.h include/raven/core/VkDebugUtils.h include/raven/core/AccelerationStructure.h
        include/raven/scene/RavenScene.h include/raven/scene/RavenSceneNode.h  include/raven/scene/RavenSceneObject.h include/raven/scene/RavenMaterial.h include/raven/scene/RavenLight.h include/raven/scene/RavenTexture.h include/raven/scene/ISceneLoader.h include/raven/scene/loader/GLTFLoader.h
        include/raven/rendergraph/RenderGraph.h
        include/raven/rendergraph/RenderGraphPass.h
        include/raven/rendergraph/RenderGraphResource.h
        include/raven/util/sampling/AliasTable.h
        include/raven/passes/PassShader.h include/raven/passes/PassShaderCompute.h include/raven/passes/PassShaderRayTracing.h
        include/raven/passes/Pass.h include/raven/passes/PassCompute.h
        include/raven/util/Trajectory.h
        include/raven/util/ImagePFM.h
        include/raven/util/animation/BSpline.h
        include/raven/RavenInclude.h
)

set(RAVENCORE_SOURCES
        src/raven/core/Application.cpp src/raven/core/HeadlessApplication.cpp src/raven/core/TimingHeadlessApplication.cpp src/raven/core/VideoHeadlessApplication.cpp
        src/raven/core/GPUContext.cpp src/raven/core/Queues.cpp src/raven/core/RavenVkDynamicLoader.cpp
)

add_library(ravencore SHARED ${RAVENCORE_HEADERS} ${RAVENCORE_SOURCES})
add_library(ravencore::ravencore ALIAS ravencore)

set_target_properties(ravencore PROPERTIES LINKER_LANGUAGE CXX)

target_compile_definitions(ravencore PRIVATE -DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)
target_compile_definitions(ravencore PRIVATE -DVULKAN_HPP_STORAGE_SHARED_EXPORT=1)
target_compile_definitions(ravencore PRIVATE -DVULKAN_HPP_STORAGE_SHARED=1)

target_include_directories(ravencore
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../lib>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../lib/glfw/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../lib/glm>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

target_link_libraries(ravencore Vulkan::Vulkan glfw imgui stb)

install(TARGETS ravencore LIBRARY DESTINATION raven)