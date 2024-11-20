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

constexpr u32 version_to_vk(config::Version version);

}

namespace embers::vulkan {

Instance::Instance(const config::Platform& config) {
  VkResult                            result               = VK_SUCCESS;
  VkApplicationInfo                   app_info             = {};
  VkInstanceCreateInfo                instance_create_info = {};
  const Instance::Vector<const char*> extensions =
      get_extension_list(config);  // todo checks
  const Instance::Vector<const char*> layers =
      get_layer_list(config);  // todo checks

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

Instance::Vector<const char*> Instance::get_extension_list(
    const config::Platform& config
) {
  // (n1t3)todo: i'm unsure about stl set, but it is better to search using it
  using SetOfViews = std::unordered_set<
      std::string_view,
      std::hash<std::string_view>,
      std::equal_to<std::string_view>,
      Allocator<std::string_view>>;

  Vector<const char*>           return_value   = {};
  VkResult                      result         = VK_SUCCESS;
  u32                           existing_count = 0;
  Vector<VkExtensionProperties> existing_vec   = {};
  SetOfViews                    existing_map   = {};

  u32                glfw_extensions_length = 0;
  const char* const* glfw_extensions        = nullptr;

  // trying to predict a potential size
  return_value.reserve(
      5 + config.extensions.required.length + config.extensions.optional.length
  );

  // Get existing
  result = vkEnumerateInstanceExtensionProperties(  //
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

  // Nessesary
  // - GLFW
  glfw_extensions_length = 0;
  glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_length);
  if (glfw_extensions_length == 0) {
    last_error_ = Error::kVulkanEnumerateExtensions;
    return {};
  }
  for (u32 i = 0; i < glfw_extensions_length; i++) {
    if (existing_map.count(glfw_extensions[i]) == 0) {
      EMBERS_FATAL(
          "Unable to init Vulkan; glfw extension was not found: {}",
          glfw_extensions[i]
      );
      last_error_ = Error::kVulkanRequiredExtensionsArentPresent;
      return {};
    }
  }
  return_value.insert(
      return_value.end(),
      glfw_extensions,
      glfw_extensions + glfw_extensions_length
  );
  // - Requested
  for (u32 i = 0; i < config.extensions.required.length; i++) {
    if (existing_map.count(config.extensions.required.array[i]) == 0) {
      EMBERS_FATAL(
          "Unable to init Vulkan; requested extension was not found: {}",
          config.extensions.required.array[i]
      );
      return {};
    }
  }
  return_value.insert(
      return_value.end(),
      config.extensions.required.array,
      config.extensions.required.array + config.extensions.required.length
  );

  // Optional
  // - Requested
  for (u32 i = 0; i < config.extensions.optional.length; i++) {
    if (existing_map.count(config.extensions.optional.array[i]) != 0) {
      return_value.push_back(config.extensions.optional.array[i]);
    } else {
      EMBERS_ERROR(
          "Unable to properly initialize Vulkan; requested extension was "
          "not found: {}, skip",
          config.extensions.optional.array[i]
      );
    }
  }
// - Debug
#ifdef EMBERS_CONFIG_DEBUG
  for (const auto& requested : debug_extensions) {
    if (existing_map.count(requested)) {
      return_value.push_back(requested);
    } else {
      EMBERS_ERROR(
          "Unable to properly initialize Vulkan; debug extension was "
          "not found: {}, skip",
          requested
      );
    }
  }
#endif
  return return_value;
}

Instance::Vector<const char*> Instance::get_layer_list(
    const config::Platform& config
) {
  // (n1t3)todo: i'm unsure about stl set, but it is better to search using it
  using SetOfViews = std::unordered_set<
      std::string_view,
      std::hash<std::string_view>,
      std::equal_to<std::string_view>,
      Allocator<std::string_view>>;

  Vector<const char*>       return_value   = {};
  VkResult                  result         = VK_SUCCESS;
  u32                       existing_count = 0;
  Vector<VkLayerProperties> existing_vec   = {};
  SetOfViews                existing_map   = {};

  // trying to predict a potential size
  return_value.reserve(
      config.layers.required.length + config.layers.optional.length
  );

  // Get existing
  result = vkEnumerateInstanceLayerProperties(  //
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

  // Nessesary
  // - Requested
  for (u32 i = 0; i < config.layers.required.length; i++) {
    if (existing_map.count(config.layers.required.array[i]) == 0) {
      EMBERS_FATAL(
          "Unable to init Vulkan; requested layer was not found: {}",
          config.layers.required.array[i]
      );
      return {};
    }
  }
  return_value.insert(
      return_value.end(),
      config.layers.required.array,
      config.layers.required.array + config.layers.required.length
  );

  // Optional
  // - Requested
  for (u32 i = 0; i < config.layers.optional.length; i++) {
    if (existing_map.count(config.layers.optional.array[i]) != 0) {
      return_value.push_back(config.layers.optional.array[i]);
    } else {
      EMBERS_ERROR(
          "Unable to properly initialize Vulkan; requested layer was "
          "not found: {}, skip",
          config.layers.optional.array[i]
      );
    }
  }
// - Debug
#ifdef EMBERS_CONFIG_DEBUG
  for (const auto& requested : debug_layers) {
    if (existing_map.count(requested)) {
      return_value.push_back(requested);
    } else {
      EMBERS_ERROR(
          "Unable to properly initialize Vulkan; validation layer was "
          "not found: {}, skip",
          requested
      );
    }
  }
#endif
  return return_value;
}

Instance::Vector<VkPhysicalDevice> Instance::get_device_list() {
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

VkPhysicalDevice Instance::pick_device(
    const Instance::Vector<VkPhysicalDevice>& devices
) {
  Instance::Vector<u32>                        rating(devices.size(), 1);
  Instance::Vector<VkPhysicalDeviceProperties> properties(devices.size());
  Instance::Vector<VkPhysicalDeviceFeatures>   features(devices.size());

  for (std::size_t i = 0; i < devices.size(); ++i) {
    vkGetPhysicalDeviceProperties(devices[i], &properties[i]);
    vkGetPhysicalDeviceFeatures(devices[i], &features[i]);
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
  return devices[pos];
}

Error Instance::last_error_ = Error::kUnknown;

constexpr u32 version_to_vk(config::Version version) {
  return VK_MAKE_VERSION(version.major, version.minor, version.patch);
}

}  // namespace embers::vulkan