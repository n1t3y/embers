#include <fmt/format.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <embers/logger.hpp>
#include <embers/test.hpp>
#include <iostream>
#include <unordered_set>
#include <vector>

#include "GLFW/glfw3.h"
#include "containers/allocator.hpp"
#include "containers/debug_allocator.hpp"
#include "ecs/entity.hpp"
#include "engine_config.hpp"
#include "error_code.hpp"
#include "platform.hpp"
#include "vulkan/common.hpp"
#include "vulkan/instance.hpp"
#include "window.hpp"

using embers::containers::TestAllocator;

using namespace embers;

int embers::test::main() {
  EMBERS_INFO(
      "Main called: {} ver. {} built @ " __DATE__ " " __TIME__,
      embers::config::engine.name,
      embers::config::engine.version
  );

  embers::config::Platform config;

  auto platform = embers::Platform(config);

  if (!(bool)platform) {
    auto err = embers::Platform::get_last_error();
    EMBERS_DEBUG("{}", err);
    return 1;
  }

  const auto       devices = platform.vulkan_.get_device_list();
  VkPhysicalDevice device  = platform.vulkan_.pick_device(devices);

  VkSurfaceKHR surface;
  VkResult     err = glfwCreateWindowSurface(
      (VkInstance)platform.vulkan_,
      (GLFWwindow*)platform.window_,
      NULL,
      &surface
  );
  if (err != VK_SUCCESS) {
    EMBERS_ERROR("Stuff happened, bad stuff: {}", (int)err);
  }

  u32 queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(
      device,
      &queueFamilyCount,
      queueFamilies.data()
  );

  EMBERS_DEBUG(
      "Flags: OPTICAL_FLOW | ??? | VIDEO_ENCODE | VIDEO_DECODE | PROTECTED | "
      "SPARSE_BINDING | TRANSFER | COMPUTE | GRAPHICS "
  );

  struct {
    u32 graphics;
    u32 present;
    struct {
      bool graphics : 1;
      bool present  : 1;
    } flags = {false, false};
  } indices;

  EMBERS_DEBUG("Queue families for first device:");
  EMBERS_DEBUG("                    O?EDPSTCG");
  u32 i = 0;
  for (const auto& queueFamily : queueFamilies) {
    EMBERS_DEBUG(
        "- Count: {:2}; Flags: {:0>9b}; Timestamp Valid Bits: {}; Min Image "
        "Transfer Granularity: {}x{}x{}",
        queueFamily.queueCount,
        queueFamily.queueFlags,
        queueFamily.timestampValidBits,
        queueFamily.minImageTransferGranularity.width,
        queueFamily.minImageTransferGranularity.height,
        queueFamily.minImageTransferGranularity.depth
    );
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics       = i;
      indices.flags.graphics = true;
    }
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(  //
        device,
        i,
        surface,
        &presentSupport
    );
    if (presentSupport) {
      indices.present       = i;
      indices.flags.present = true;
    }
    i++;
  }
  EMBERS_DEBUG("Index graphics: {}", indices.graphics);
  EMBERS_DEBUG("Index present: {}", indices.present);

  // got the thing

  std::vector<VkDeviceQueueCreateInfo> device_queue_create_infos;
  device_queue_create_infos.reserve(2);

  std::unordered_set<u32> queue_families = {indices.graphics, indices.present};
  float                   queue_priority = 1.f;

  for (const u32 i : queue_families) {
    VkDeviceQueueCreateInfo device_queue_create_info = {};
    device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    device_queue_create_info.pNext = nullptr;
    device_queue_create_info.flags = {};
    device_queue_create_info.queueFamilyIndex = i;
    device_queue_create_info.queueCount       = 1;
    device_queue_create_info.pQueuePriorities = &queue_priority;
    device_queue_create_infos.push_back(device_queue_create_info);
  }

  VkPhysicalDeviceFeatures device_features{};
  auto                     device_extensions =
      platform.vulkan_.get_device_extension_list(device, config);

  VkDeviceCreateInfo device_create_info{};
  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_create_info.pNext = nullptr;
  device_create_info.flags = {};
  device_create_info.queueCreateInfoCount    = device_queue_create_infos.size();
  device_create_info.pQueueCreateInfos       = device_queue_create_infos.data();
  device_create_info.enabledExtensionCount   = device_extensions.size();
  device_create_info.ppEnabledExtensionNames = device_extensions.data();
  device_create_info.pEnabledFeatures        = &device_features;

  VkDevice virtual_device;
  VkResult resu =
      vkCreateDevice(device, &device_create_info, nullptr, &virtual_device);

  EMBERS_DEBUG("Device: {} ({})", fmt::ptr(virtual_device), (int)resu);

  VkQueue graphics;
  VkQueue present;

  vkGetDeviceQueue(virtual_device, indices.graphics, 0, &graphics);
  vkGetDeviceQueue(virtual_device, indices.present, 0, &present);

#ifdef EMBERS_CONFIG_DEBUG
  EMBERS_DEBUG("Vulkan: {}", embers::containers::debug_allocator_info[0]);
  EMBERS_DEBUG("Logger: {}", embers::containers::debug_allocator_info[1]);

#endif

  vkDestroyDevice(virtual_device, nullptr);

  vkDestroySurfaceKHR((VkInstance)platform.vulkan_, surface, nullptr);

  return 0;
}