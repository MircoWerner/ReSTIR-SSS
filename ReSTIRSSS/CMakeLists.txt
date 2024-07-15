cmake_minimum_required(VERSION 3.18)
project(restirsss VERSION 0.1.0 DESCRIPTION "ReSTIR Subsurface Scattering" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(Vulkan REQUIRED)

#set(GCC_COVERAGE_COMPILE_FLAGS "-fpermissive")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

message(STATUS "Compiler Version: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")

# VkRaven Core
if(NOT RAVEN_BASE_DIRECTORY)
    message(${CMAKE_CURRENT_SOURCE_DIR})
    find_path(RAVEN_BASE_DIRECTORY
            NAMES VkRaven/cmake/setup.cmake
            PATHS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../..
            REQUIRED
            DOC "Directory containing VkRaven"
    )
endif()
if(EXISTS ${RAVEN_BASE_DIRECTORY}/VkRaven)
    add_subdirectory(${RAVEN_BASE_DIRECTORY}/VkRaven ${CMAKE_CURRENT_BINARY_DIR}/VkRaven)
else()
    message(FATAL_ERROR "Could not find base directory, please set RAVEN_BASE_DIRECTORY to folder containing VkRaven")
endif()
set(RAVEN_CORE_DIR ${RAVEN_BASE_DIRECTORY}/VkRaven)

SET(ENABLE_OPTIX ON)
if (ENABLE_OPTIX)
    find_package(CUDAToolkit)
    if (CUDAToolkit_FOUND)
        _add_package_Optix()
    else()
        message(STATUS "CudaToolkit: Not found")
        message(STATUS "OptiX: Disabled")
    endif ()
endif ()

set(RAVENPROJECT_HEADERS
        include/restirsss/Raystructs.h include/restirsss/Raydata.h include/restirsss/Rayscenes.h
        include/restirsss/passes/PassTonemapper.h
        include/denoiser/DenoiserOptiX.h
        include/restirsss/ReSTIRSSS.h
        include/restirsss/passes/PassGBuffer.h  include/restirsss/passes/PassDebug.h include/restirsss/passes/PassSurface.h include/restirsss/passes/PassPathtrace.h
        include/restirsss/passes/ReSTIRSSSPassCandidateGeneration.h include/restirsss/passes/ReSTIRSSSPassTemporalReuse.h include/restirsss/passes/ReSTIRSSSPassSpatialReuse.h include/restirsss/passes/ReSTIRSSSPassShade.h
        include/restirsss/ReSTIRSSSEvaluation.h include/restirsss/ReSTIRSSSEvaluationTiming.h include/restirsss/ReSTIRSSSVideo.h
)

set(RAVENPROJECT_SOURCES
        src/bin/main.cpp
        src/denoiser/DenoiserOptiX.cpp
        src/restirsss/ReSTIRSSS.cpp src/restirsss/ReSTIRSSSEvaluation.cpp src/restirsss/ReSTIRSSSEvaluationTiming.cpp src/restirsss/ReSTIRSSSVideo.cpp
)

add_executable(${CMAKE_PROJECT_NAME} ${RAVENPROJECT_HEADERS} ${RAVENPROJECT_SOURCES})

target_link_libraries(${CMAKE_PROJECT_NAME} glm Vulkan::Vulkan imgui stb tinyobjloader tinygltf spirv-reflect ravencore glfw pugixml argparse)
if (CUDAToolkit_FOUND)
    target_link_libraries(${CMAKE_PROJECT_NAME} CUDA::cudart CUDA::cuda_driver)
endif ()

target_include_directories(${CMAKE_PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
if (OPTIX_FOUND)
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${OPTIX_INCLUDE_DIR})
endif ()

target_include_directories(${PROJECT_NAME} PRIVATE ${OPTIX_INCLUDE_DIR})

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE -DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 -DVULKAN_HPP_STORAGE_SHARED=1)

SET(RESOURCE_DIRECTORY_PATH \"${CMAKE_CURRENT_SOURCE_DIR}/resources\")
if (RESOURCE_DIRECTORY_PATH)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE RESOURCE_DIRECTORY_PATH=${RESOURCE_DIRECTORY_PATH})
endif ()

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_BINARY_DIR}/VkRaven/lib"
        "${PROJECT_BINARY_DIR}")
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_BINARY_DIR}/VkRaven/raven"
        "${PROJECT_BINARY_DIR}")