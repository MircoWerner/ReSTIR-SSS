cmake_minimum_required(VERSION 3.18)
project(raven VERSION 0.1.0 DESCRIPTION "VkRaven - Vulkan Render and Visualization Engine" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(Vulkan REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/find/" PARENT_SCOPE)
include(cmake/setup.cmake)

if (MSVC)
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup" PARENT_SCOPE)
endif ()

add_subdirectory(lib)
add_subdirectory(raven)
