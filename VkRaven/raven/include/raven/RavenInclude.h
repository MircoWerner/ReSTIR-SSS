#pragma once

#ifdef WIN32
#include <windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <vulkan/vulkan.hpp>
#else
#include <vulkan/vulkan.hpp>
#endif