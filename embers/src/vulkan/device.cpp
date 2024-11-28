#include "device.hpp"

#include <vulkan/vulkan_core.h>

#include <functional>
#include <unordered_set>

namespace embers::vulkan {

Device::Device(const Instance& instance, const config::Platform& config) {
  //   auto             physical_devices = instance.get_device_list();
  //   VkPhysicalDevice physical_device  =
  //   instance.pick_device(physical_devices);
  //   ;
  //   Vector<VkDeviceQueueCreateInfo> device_queue_create_infos;
  //   device_queue_create_infos.reserve(2);

  //   std::unordered_set<u32, std::hash<u32>, std::equal_to<u32>,
  //   Allocator<u32>>
  //       queue_families = {indices.graphics, indices.present};

  //   float queue_priority = 1.f;
  //   for (const u32 i : queue_families) {
  //     VkDeviceQueueCreateInfo device_queue_create_info = {};
  //     device_queue_create_info.sType =
  //     VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  //     device_queue_create_info.pNext = nullptr;
  //     device_queue_create_info.flags = {};
  //     device_queue_create_info.queueFamilyIndex = i;
  //     device_queue_create_info.queueCount       = 1;
  //     device_queue_create_info.pQueuePriorities = &queue_priority;
  //     device_queue_create_infos.push_back(device_queue_create_info);
  //   }

  //   ;
  //   VkPhysicalDeviceFeatures device_features{};
  //   auto                     device_extensions =
  //       instance.get_device_extension_list(physical_device, config);

  //   VkDeviceCreateInfo device_create_info{};
  //   device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  //   device_create_info.queueCreateInfoCount    =
  //   device_queue_create_infos.size(); device_create_info.pQueueCreateInfos =
  //   device_queue_create_infos.data();
  //   device_create_info.enabledExtensionCount   = device_extensions.size();
  //   device_create_info.ppEnabledExtensionNames = device_extensions.data();
  //   device_create_info.pEnabledFeatures        = &device_features;

  //   VkResult result = vkCreateDevice(  //
  //       physical_device,
  //       &device_create_info,
  //       nullptr,
  //       &device_
  //   );
}

void Device::destroy() { vkDestroyDevice(device_, nullptr); }

}  // namespace embers::vulkan