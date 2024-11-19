#include <fmt/format.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <embers/logger.hpp>
#include <embers/test.hpp>
#include <iostream>
#include <vector>

#include "containers/allocator.hpp"
#include "containers/debug_allocator.hpp"
#include "ecs/entity.hpp"
#include "engine_config.hpp"
#include "error_code.hpp"
#include "platform.hpp"
#include "vulkan.hpp"
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
    auto err = embers::to_error_code(embers::Platform::get_last_error());
    EMBERS_DEBUG("{}", err);
    return 1;
  }

  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(
      (VkInstance)platform.vulkan_,
      &deviceCount,
      nullptr
  );
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(
      (VkInstance)platform.vulkan_,
      &deviceCount,
      devices.data()
  );

  EMBERS_DEBUG("Devices:");
  for (const VkPhysicalDevice &i : devices) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures   deviceFeatures;
    vkGetPhysicalDeviceProperties(i, &deviceProperties);
    vkGetPhysicalDeviceFeatures(i, &deviceFeatures);

    EMBERS_DEBUG(
        "- {}: {} ({}) {}",
        deviceProperties.deviceID,
        deviceProperties.deviceName,
        (int)deviceProperties.deviceType,
        deviceFeatures.geometryShader
    );
  }

  VkPhysicalDevice device = devices.front();

  u32 queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(
      device,
      &queueFamilyCount,
      queueFamilies.data()
  );

  // VK_QUEUE_GRAPHICS_BIT = 0x00000001,                 1
  // VK_QUEUE_COMPUTE_BIT = 0x00000002,                 10
  // VK_QUEUE_TRANSFER_BIT = 0x00000004,               100
  // VK_QUEUE_SPARSE_BINDING_BIT = 0x00000008,        1000
  // VK_QUEUE_PROTECTED_BIT = 0x00000010,            10000
  // VK_QUEUE_VIDEO_DECODE_BIT_KHR = 0x00000020,    100000
  // VK_QUEUE_VIDEO_ENCODE_BIT_KHR = 0x00000040,   1000000
  // VK_QUEUE_OPTICAL_FLOW_BIT_NV = 0x00000100,  100000000

  EMBERS_DEBUG(
      "Flags: OPTICAL_FLOW | ??? | VIDEO_ENCODE | VIDEO_DECODE | PROTECTED | "
      "SPARSE_BINDING | TRANSFER | COMPUTE | GRAPHICS "
  );

  EMBERS_DEBUG("Queue families for first device:");
  EMBERS_DEBUG("                    O?EDPSTCG");
  for (const auto &queueFamily : queueFamilies) {
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
  }

#ifdef EMBERS_CONFIG_DEBUG
  EMBERS_DEBUG("Vulkan: {}", embers::containers::debug_allocator_info[0]);
  EMBERS_DEBUG("Logger: {}", embers::containers::debug_allocator_info[1]);

#endif

  return 0;
}