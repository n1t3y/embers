#include "instance.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <iterator>
#include <unordered_set>

#include "../engine_config.hpp"
#include "common.hpp"
#include "debug_messenger.hpp"

namespace embers::vulkan {

struct Range {
  const char* const* first;
  const char* const* last;
};

constexpr u32 version_to_vk(config::Version version);
}  // namespace embers::vulkan

namespace embers::vulkan {

Instance::Instance(const config::Platform& config) {
  VkResult                  result               = VK_SUCCESS;
  VkApplicationInfo         app_info             = {};
  VkInstanceCreateInfo      instance_create_info = {};
  const Vector<const char*> extensions =
      get_extension_list(config);                             // todo checks
  const Vector<const char*> layers = get_layer_list(config);  // todo checks

  EMBERS_DEBUG("Enabled extensions: ");
  for (const auto& i : extensions) {
    EMBERS_DEBUG("- {}", i);
  }
  EMBERS_DEBUG("Enabled layers: ");
  for (const auto& i : layers) {
    EMBERS_DEBUG("- {}", i);
  }

  app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName   = config.application_name;
  app_info.applicationVersion = version_to_vk(config.version);
  app_info.pEngineName        = embers::config::engine.name;
  app_info.engineVersion      = version_to_vk(embers::config::engine.version);
  app_info.apiVersion         = VK_API_VERSION_1_0;

  instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_create_info.pApplicationInfo        = &app_info;
  instance_create_info.enabledExtensionCount   = extensions.size();
  instance_create_info.ppEnabledExtensionNames = extensions.data();
  instance_create_info.enabledLayerCount       = layers.size();
  instance_create_info.ppEnabledLayerNames     = layers.data();

#ifdef EMBERS_CONFIG_DEBUG
  instance_create_info.pNext = &DebugMessenger::create_info;
#else
  instance_create_info.pNext = nullptr;
#endif

  result = vkCreateInstance(
      &instance_create_info,
      nullptr,
      &instance_
  );  // (n1t3)todo allocator callbacks

  if (result != VK_SUCCESS) {
    EMBERS_FATAL(
        "Unable to init Vulkan: vkCreateInstance returned {}",
        (u32)result
    );
    instance_ = nullptr;
    return;
  }
  EMBERS_INFO("Vulkan initialized");

  return;
}

void Instance::destroy() {
  vkDestroyInstance(instance_, nullptr);  // (n1t3)todo allocator callbacks
  EMBERS_INFO("Vulkan terminated");

  return;
}

Vector<const char*> Instance::get_extension_list(const config::Platform& config
) {
  // (n1t3)todo: i'm unsure about stl set, but it is better to search using it
  using SetOfViews = std::unordered_set<
      std::string_view,
      std::hash<std::string_view>,
      std::equal_to<std::string_view>,
      Allocator<std::string_view>>;

  Vector<const char*>           return_value   = {};
  Vector<VkExtensionProperties> existing_vec   = {};
  SetOfViews                    existing_map   = {};
  const auto                    present_in_map =  //
      [&existing_map](const char* x) constexpr -> bool {
    return existing_map.count(x) != 0;
  };

  return_value.reserve(
      5 + config.instance.extensions.required.size +
      config.instance.extensions.optional.size
  );

  Range required_extensions[] = {
      {},
      {config.instance.extensions.required.array,
       config.instance.extensions.required.array +
           config.instance.extensions.required.size}
  };
  Range optional_extensions[] = {
      {config.instance.extensions.optional.array,
       config.instance.extensions.optional.array +
           config.instance.extensions.optional.size},
#ifdef EMBERS_CONFIG_DEBUG
      {debug_extensions,
       debug_extensions + sizeof(debug_extensions) / sizeof(const char*)}
#endif
  };
  // fill required extension range (glfw)
  {
    u32 glfw_size = 0;
    required_extensions[0].first =
        glfwGetRequiredInstanceExtensions(&glfw_size);
    required_extensions[0].last = required_extensions[0].first + glfw_size;
    if (glfw_size == 0) {
      last_error_ = Error::kVulkanEnumerateExtensions;
      return {};
    }
  }

  // Pack existing extensions into the map
  {
    u32 existing_count = 0;

    VkResult result = vkEnumerateInstanceExtensionProperties(  //
        nullptr,
        &existing_count,
        nullptr
    );
    if (result != VK_SUCCESS) {
      last_error_ = Error::kVulkanEnumerateExtensions;
      return {};
    }
    existing_vec.resize(existing_count);
    result = vkEnumerateInstanceExtensionProperties(
        nullptr,
        &existing_count,
        existing_vec.data()
    );
    if (result != VK_SUCCESS) {
      last_error_ = Error::kVulkanEnumerateExtensions;
      return {};
    }
    existing_map.reserve(existing_vec.size());
    std::transform(  // move the strings into set to search faster
        existing_vec.cbegin(),
        existing_vec.cend(),
        std::inserter(existing_map, existing_map.begin()),
        [](const VkExtensionProperties& properties) {
          return std::string_view(properties.extensionName);
        }
    );
  }

  for (const Range& range : required_extensions) {
    const char* const* missing =
        std::find_if_not(range.first, range.last, present_in_map);
    if (missing != range.last) {
      EMBERS_FATAL("Required extension wasn't found: {}", *missing);
      last_error_ = Error::kVulkanRequiredExtensionsArentPresent;
      return {};
    }
    return_value.insert(return_value.end(), range.first, range.last);
  }

  for (const Range& range : optional_extensions) {
    std::copy_if(
        range.first,
        range.last,
        std::back_inserter(return_value),
        [&present_in_map](const char* extension) constexpr -> bool {
          bool present = present_in_map(extension);
          if (!present) {
            EMBERS_ERROR("Optional extension wasn't found: {}", extension);
          }
          return present;
        }
    );
  }

  return return_value;
}

Vector<const char*> Instance::get_layer_list(const config::Platform& config) {
  // (n1t3)todo: i'm unsure about stl set, but it is better to search using it
  using SetOfViews = std::unordered_set<
      std::string_view,
      std::hash<std::string_view>,
      std::equal_to<std::string_view>,
      Allocator<std::string_view>>;

  Vector<const char*>       return_value   = {};
  Vector<VkLayerProperties> existing_vec   = {};
  SetOfViews                existing_map   = {};
  const auto                present_in_map =  //
      [&existing_map](const char* x) constexpr -> bool {
    return existing_map.count(x) != 0;
  };

  return_value.reserve(
      config.instance.layers.required.size +
      config.instance.layers.optional.size
  );

  Range required_layers[] = {
      {config.instance.layers.required.array,
       config.instance.layers.required.array +
           config.instance.layers.required.size}
  };
  Range optional_layers[] = {
      {config.instance.layers.optional.array,
       config.instance.layers.optional.array +
           config.instance.layers.optional.size},
#ifdef EMBERS_CONFIG_DEBUG
      {debug_layers, debug_layers + sizeof(debug_layers) / sizeof(const char*)}
#endif
  };

  // Pack existing layers into the map
  {
    u32 existing_count = 0;

    VkResult result = vkEnumerateInstanceLayerProperties(  //
        &existing_count,
        nullptr
    );
    if (result != VK_SUCCESS) {
      last_error_ = Error::kVulkanEnumerateLayers;
      return {};
    }
    existing_vec.resize(existing_count);
    result = vkEnumerateInstanceLayerProperties(  //
        &existing_count,
        existing_vec.data()
    );
    if (result != VK_SUCCESS) {
      last_error_ = Error::kVulkanEnumerateLayers;
      return {};
    }
    existing_map.reserve(existing_vec.size());
    std::transform(  // move the strings into set to search faster
        existing_vec.cbegin(),
        existing_vec.cend(),
        std::inserter(existing_map, existing_map.begin()),
        [](const VkLayerProperties& properties) {
          return std::string_view(properties.layerName);
        }
    );
  }

  for (const Range& range : required_layers) {
    const char* const* missing =
        std::find_if_not(range.first, range.last, present_in_map);
    if (missing != range.last) {
      EMBERS_FATAL("Required layer wasn't found: {}", *missing);
      last_error_ = Error::kVulkanRequiredLayersArentPresent;
      return {};
    }
    return_value.insert(return_value.end(), range.first, range.last);
  }

  for (const Range& range : optional_layers) {
    std::copy_if(
        range.first,
        range.last,
        std::back_inserter(return_value),
        [&present_in_map](const char* layer) constexpr -> bool {
          bool present = present_in_map(layer);
          if (!present) {
            EMBERS_ERROR("Optional layer wasn't found: {}", layer);
          }
          return present;
        }
    );
  }

  return return_value;
}

Vector<const char*> Instance::get_device_extension_list(
    VkPhysicalDevice device, const config::Platform& config
) {
  // (n1t3)todo: i'm unsure about stl set, but it is better to search using it
  using SetOfViews = std::unordered_set<
      std::string_view,
      std::hash<std::string_view>,
      std::equal_to<std::string_view>,
      Allocator<std::string_view>>;

  Vector<const char*>           return_value   = {};
  Vector<VkExtensionProperties> existing_vec   = {};
  SetOfViews                    existing_map   = {};
  const auto                    present_in_map =  //
      [&existing_map](const char* x) constexpr -> bool {
    return existing_map.count(x) != 0;
  };

  return_value.reserve(
      config.device.extensions.required.size +
      config.device.extensions.optional.size
  );

  Range required_extensions[] = {
      {required_device_extensions,
       required_device_extensions +
           sizeof(required_device_extensions) / sizeof(const char*)},
      {config.instance.extensions.required.array,
       config.instance.extensions.required.array +
           config.instance.extensions.required.size}
  };
  Range optional_extensions[] = {
      {config.instance.extensions.optional.array,
       config.instance.extensions.optional.array +
           config.instance.extensions.optional.size},
  };

  // Pack existing extensions into the map
  {
    u32 existing_count = 0;

    VkResult result = vkEnumerateDeviceExtensionProperties(  //
        device,
        nullptr,
        &existing_count,
        nullptr
    );
    if (result != VK_SUCCESS) {
      last_error_ = Error::kVulkanEnumerateDeviceExtensions;
      return {};
    }
    existing_vec.resize(existing_count);
    result = vkEnumerateDeviceExtensionProperties(
        device,
        nullptr,
        &existing_count,
        existing_vec.data()
    );
    if (result != VK_SUCCESS) {
      last_error_ = Error::kVulkanEnumerateDeviceExtensions;
      return {};
    }
    existing_map.reserve(existing_vec.size());
    std::transform(  // move the strings into set to search faster
        existing_vec.cbegin(),
        existing_vec.cend(),
        std::inserter(existing_map, existing_map.begin()),
        [](const VkExtensionProperties& properties) {
          return std::string_view(properties.extensionName);
        }
    );
  }

  for (const Range& range : required_extensions) {
    const char* const* missing =
        std::find_if_not(range.first, range.last, present_in_map);
    if (missing != range.last) {
      EMBERS_FATAL("Required device extension wasn't found: {}", *missing);
      last_error_ = Error::kVulkanRequiredDeviceExtensionsArentPresent;
      return {};
    }
    return_value.insert(return_value.end(), range.first, range.last);
  }

  for (const Range& range : optional_extensions) {
    std::copy_if(
        range.first,
        range.last,
        std::back_inserter(return_value),
        [&present_in_map](const char* extension) constexpr -> bool {
          bool present = present_in_map(extension);
          if (!present) {
            EMBERS_ERROR(
                "Optional device extension wasn't found: {}",
                extension
            );
          }
          return present;
        }
    );
  }

  return return_value;
}

// Instance::Vector<const char*> Instance::get_device_layer_list(
//     VkPhysicalDevice device, const config::Platform& config
// ) {
//   // (n1t3)todo: i'm unsure about stl set, but it is better to search using
//   it using SetOfViews = std::unordered_set<
//       std::string_view,
//       std::hash<std::string_view>,
//       std::equal_to<std::string_view>,
//       Allocator<std::string_view>>;

//   Vector<const char*>       return_value   = {};
//   Vector<VkLayerProperties> existing_vec   = {};
//   SetOfViews                existing_map   = {};
//   const auto                present_in_map =  //
//       [&existing_map](const char* x) constexpr -> bool {
//     return existing_map.count(x) != 0;
//   };

//   return_value.reserve(
//       config.device.layers.required.size + config.device.layers.optional.size
//   );

//   Range required_layers[] = {
//       {config.instance.layers.required.array,
//        config.instance.layers.required.array +
//            config.instance.layers.required.size}
//   };
//   Range optional_layers[] = {
//       {config.instance.layers.optional.array,
//        config.instance.layers.optional.array +
//            config.instance.layers.optional.size},
// #ifdef EMBERS_CONFIG_DEBUG
//       {debug_layers, debug_layers + sizeof(debug_layers) / sizeof(const
//       char*)}
// #endif

//   };

//   // Pack existing layers into the map
//   {
//     u32 existing_count = 0;

//     VkResult result = vkEnumerateDeviceLayerProperties(  //
//         device,
//         &existing_count,
//         nullptr
//     );
//     if (result != VK_SUCCESS) {
//       last_error_ = Error::kVulkanEnumerateDeviceLayers;
//       return {};
//     }
//     existing_vec.resize(existing_count);
//     result = vkEnumerateDeviceLayerProperties(
//         device,
//         &existing_count,
//         existing_vec.data()
//     );
//     if (result != VK_SUCCESS) {
//       last_error_ = Error::kVulkanEnumerateDeviceLayers;
//       return {};
//     }
//     existing_map.reserve(existing_vec.size());
//     std::transform(  // move the strings into set to search faster
//         existing_vec.cbegin(),
//         existing_vec.cend(),
//         std::inserter(existing_map, existing_map.begin()),
//         [](const VkLayerProperties& properties) {
//           return std::string_view(properties.layerName);
//         }
//     );
//   }

//   for (const Range& range : required_layers) {
//     const char* const* missing =
//         std::find_if_not(range.first, range.last, present_in_map);
//     if (missing != range.last) {
//       EMBERS_FATAL("Required device layer wasn't found: {}", *missing);
//       last_error_ = Error::kVulkanRequiredDeviceLayersArentPresent;
//       return {};
//     }
//     return_value.insert(return_value.end(), range.first, range.last);
//   }

//   for (const Range& range : optional_layers) {
//     std::copy_if(
//         range.first,
//         range.last,
//         std::back_inserter(return_value),
//         [&present_in_map](const char* layer) constexpr -> bool {
//           bool present = present_in_map(layer);
//           if (!present) {
//             EMBERS_ERROR("Optional device layer wasn't found: {}", layer);
//           }
//           return present;
//         }
//     );
//   }

//   return return_value;
// }

Vector<VkPhysicalDevice> Instance::get_device_list() const {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(  //
      instance_,
      &deviceCount,
      nullptr
  );
  Vector<VkPhysicalDevice> devices(deviceCount);

  vkEnumeratePhysicalDevices(  //
      instance_,
      &deviceCount,
      devices.data()
  );
  return devices;
}

VkPhysicalDevice Instance::pick_device(const Vector<VkPhysicalDevice>& devices
) {
  Vector<u32>                        rating(devices.size(), 1);
  Vector<VkPhysicalDeviceProperties> properties(devices.size());
  Vector<VkPhysicalDeviceFeatures>   features(devices.size());

  for (std::size_t i = 0; i < devices.size(); ++i) {
    vkGetPhysicalDeviceProperties(devices[i], &properties[i]);
    vkGetPhysicalDeviceFeatures(devices[i], &features[i]);
  }

  for (std::size_t i = 0; i < devices.size(); ++i) {
    u32 extension_count = 0;
    vkEnumerateDeviceExtensionProperties(
        devices[i],
        nullptr,
        &extension_count,
        nullptr
    );
    Vector<VkExtensionProperties> extension_properties(extension_count);
    vkEnumerateDeviceExtensionProperties(
        devices[i],
        nullptr,
        &extension_count,
        extension_properties.data()
    );

    for (const char* required_extension : required_device_extensions) {
      for (const VkExtensionProperties& existing : extension_properties) {
        if (strcmp(existing.extensionName, required_extension) == 0) {
          goto continue_loop;
        }
      }

      EMBERS_DEBUG(
          "Unable to find device extension {}, skip device",
          required_extension
      );
      rating[i] = 0;
      break;
    continue_loop:;
    }
  }

  for (std::size_t i = 0; i < devices.size(); ++i) {
    const static u32 shifts[] = {
        0,  // 0: VK_PHYSICAL_DEVICE_TYPE_OTHER
        1,  // 1: VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
        2,  // 2: VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
        1,  // 3: VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU
        0,  // 4: VK_PHYSICAL_DEVICE_TYPE_CPU
    };
    rating[i] <<= shifts[properties[i].deviceType];
  }
  const auto iter = std::max_element(rating.cbegin(), rating.cend());
  const auto pos  = std::distance(rating.cbegin(), iter);
  // todo check not 0;
  EMBERS_DEBUG("Picked device: {}", properties[pos].deviceName);
  return devices[pos];
}

Error Instance::last_error_ = Error::kUnknown;

constexpr u32 version_to_vk(config::Version version) {
  return VK_MAKE_VERSION(version.major, version.minor, version.patch);
}

}  // namespace embers::vulkan