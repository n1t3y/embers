#include "device.hpp"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <bitset>
#include <cstddef>
#include <functional>
#include <iterator>
#include <unordered_set>

#include "surface.hpp"

namespace embers::vulkan {

Error Device::last_error_ = Error::kUnknown;

Device::Device(
    const Instance&         instance,
    const Surface&          surface,
    const config::Platform& config
) {
  auto             physical_devices = instance.get_device_list();
  VkPhysicalDevice physical_device  = instance.pick_device(physical_devices);

  // gather info about physical device

  u32 device_families_length = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(
      physical_device,
      &device_families_length,
      nullptr
  );

  VkQueueFamilyProperties device_families[device_families_length];
  vkGetPhysicalDeviceQueueFamilyProperties(
      physical_device,
      &device_families_length,
      device_families
  );
  struct {
    u32  used            = 0;
    bool present_support = false;
  } queue_families_additional_info[device_families_length];

  for (u32 i = 0; i < device_families_length; ++i) {
    VkBool32 presentSupport = false;
    VkResult res            = vkGetPhysicalDeviceSurfaceSupportKHR(
        physical_device,
        i,
        (VkSurfaceKHR)surface,
        &presentSupport
    );
    if (res == VK_SUCCESS && presentSupport) {
      queue_families_additional_info[i].present_support = true;
    }
  }

  // graphics teansfer present compute
  struct {
    u32 family = -1;
    u32 index  = -1;
  } queues[4] = {{}, {}, {}, {}};

  // serach graphics

  auto get_best_with = [&](auto condition) -> u32 {
    u32 ratings[device_families_length];
    for (auto& rating : ratings) {
      rating = 0x10000000;
    }

    for (u32 i = 0; i != device_families_length; ++i) {
      bool has_avaliable_queues = device_families[i].queueCount >=
                                  queue_families_additional_info[i].used;
      size_t flags_on =
          std::bitset<sizeof(VkQueueFlags) * 8>(device_families[i].queueFlags)
              .count();

      ratings[i] &= ((u32) !(condition(i) && has_avaliable_queues)) - 1;
      ratings[i] >>= flags_on;
    }
    auto iter_max = std::max_element(ratings, ratings + device_families_length);
    if (*iter_max == 0) {
      return -1;
    }
    return std::distance(ratings, iter_max);
  };

  queues[0].family = get_best_with([&](u32 i) {
    return VK_QUEUE_GRAPHICS_BIT & device_families[i].queueFlags;
  });
  queues[1].family = get_best_with([&](u32 i) {
    return VK_QUEUE_TRANSFER_BIT & device_families[i].queueFlags;
  });
  queues[2].family = get_best_with([&](u32 i) {
    return queue_families_additional_info[i].present_support;
  });
  queues[3].family = get_best_with([&](u32 i) {
    return VK_QUEUE_COMPUTE_BIT & device_families[i].queueFlags;
  });

  // todo error checks (-1)

  // todo replace with array
  std::unordered_map<
      u32,
      u32,
      std::hash<u32>,
      std::equal_to<u32>,
      Allocator<std::pair<const u32, u32>>>
      queue_count_for_family;

  queue_count_for_family.reserve(4);

  for (auto& queue : queues) {
    queue.index = queue_count_for_family[queue.family];
    queue_count_for_family[queue.family] += 1;
  }

  VkDeviceQueueCreateInfo
      device_queue_create_infos[queue_count_for_family.size()];

  float  queue_priority[4] = {1., 1., 1., 1.};  // todo
  size_t i                 = 0;
  for (const auto& iter : queue_count_for_family) {
    VkDeviceQueueCreateInfo device_queue_create_info = {};
    device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    device_queue_create_info.pNext = nullptr;
    device_queue_create_info.flags = {};
    device_queue_create_info.queueFamilyIndex = iter.first;
    device_queue_create_info.queueCount       = iter.second;
    device_queue_create_info.pQueuePriorities = queue_priority;
    device_queue_create_infos[i]              = device_queue_create_info;
    ++i;
  }

  ;
  VkPhysicalDeviceFeatures device_features{};
  auto                     device_extensions =
      instance.get_device_extension_list(physical_device, config);

  VkDeviceCreateInfo device_create_info{};
  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_create_info.queueCreateInfoCount    = queue_count_for_family.size();
  device_create_info.pQueueCreateInfos       = device_queue_create_infos;
  device_create_info.enabledExtensionCount   = device_extensions.size();
  device_create_info.ppEnabledExtensionNames = device_extensions.data();
  device_create_info.pEnabledFeatures        = &device_features;

  VkResult result = vkCreateDevice(  //
      physical_device,
      &device_create_info,
      nullptr,
      &device_
  );

  if (result != VK_SUCCESS) {
    last_error_ = Error::kUnknown;  // todo
    device_     = nullptr;
    return;
  }

  vkGetDeviceQueue(
      device_,
      queues[0].family,
      queues[0].index,
      &queues_.graphics
  );
  vkGetDeviceQueue(
      device_,
      queues[1].family,
      queues[1].index,
      &queues_.transfer
  );
  vkGetDeviceQueue(
      device_,
      queues[2].family,
      queues[2].index,
      &queues_.present
  );
  vkGetDeviceQueue(
      device_,
      queues[3].family,
      queues[3].index,
      &queues_.compute
  );
}

void Device::destroy() { vkDestroyDevice(device_, nullptr); }

}  // namespace embers::vulkan