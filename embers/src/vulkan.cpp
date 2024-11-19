#include "vulkan.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

// #include <string_view>
#include <algorithm>
#include <iterator>
#include <unordered_set>

#include "containers/allocator.hpp"
#include "engine_config.hpp"
#include "vulkan_debug_messenger.hpp"

namespace embers {

constexpr u32 version_to_vk(config::Version version);

}

namespace embers {

Vulkan::Vulkan(const config::Platform& config) {
  VkApplicationInfo                 app_info             = {};
  VkInstanceCreateInfo              instance_create_info = {};
  const Vulkan::Vector<const char*> extensions =
      get_extension_list(config);  // todo checks
  const Vulkan::Vector<const char*> layers =
      get_layer_list(config);  // todo checks

  VkResult result = VK_SUCCESS;

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
  instance_create_info.pNext = &VulkanDebugMessenger::create_info;
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

void Vulkan::destroy() {
  vkDestroyInstance(instance_, nullptr);  // (n1t3)todo allocator callbacks
  EMBERS_INFO("Vulkan terminated");

  return;
}

Vulkan::Vector<const char*> Vulkan::get_extension_list(
    const config::Platform& config
) {
  Vector<const char*> return_value = {};
  // trying to predict a potential size
  return_value.reserve(
      5 + config.extensions.required.length + config.extensions.optional.length
  );

  Vector<VkExtensionProperties> existing_vec = {};
  // (n1t3)todo: i'm unsure about stl set, but it is better to search using it
  std::unordered_set<
      std::string_view,
      std::hash<std::string_view>,
      std::equal_to<std::string_view>,
      Allocator<std::string_view>>
           existing_map = {};
  VkResult result       = VK_SUCCESS;

  // Get existing
  u32 existing_count = 0;
  result             = vkEnumerateInstanceExtensionProperties(  //
      nullptr,
      &existing_count,
      nullptr
  );
  existing_vec.resize(existing_count);
  result = result == VK_SUCCESS  //
               ? vkEnumerateInstanceExtensionProperties(
                     nullptr,
                     &existing_count,
                     existing_vec.data()
                 )
               : result;

  if (result != VK_SUCCESS) {
    last_error_ = Error::kEnumerateExtensions;
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
  u32                glfw_extensions_length = 0;
  const char* const* glfw_extensions =
      glfwGetRequiredInstanceExtensions(&glfw_extensions_length);
  if (glfw_extensions_length == 0) {
    last_error_ = Error::kEnumerateExtensions;
    return {};
  }
  for (u32 i = 0; i < glfw_extensions_length; i++) {
    if (existing_map.count(glfw_extensions[i]) == 0) {
      EMBERS_FATAL(
          "Unable to init Vulkan; glfw extension was not found: {}",
          glfw_extensions[i]
      );
      last_error_ = Error::kRequiredExtensionsArentPresent;
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
  static const char* const debug_extensions[] = {
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME
  };

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

Vulkan::Vector<const char*> Vulkan::get_layer_list(
    const config::Platform& config
) {
  Vector<const char*> return_value = {};
  // trying to predict a potential size
  return_value.reserve(
      config.layers.required.length + config.layers.optional.length
  );

  Vector<VkLayerProperties> existing_vec = {};
  // (n1t3)todo: i'm unsure about stl set, but it is better to search using it
  std::unordered_set<
      std::string_view,
      std::hash<std::string_view>,
      std::equal_to<std::string_view>,
      Allocator<std::string_view>>
           existing_map = {};
  VkResult result       = VK_SUCCESS;

  // Get existing
  u32 existing_count = 0;
  result             = vkEnumerateInstanceLayerProperties(  //
      &existing_count,
      nullptr
  );
  existing_vec.resize(existing_count);
  result = result == VK_SUCCESS  //
               ? vkEnumerateInstanceLayerProperties(
                     &existing_count,
                     existing_vec.data()
                 )
               : result;

  if (result != VK_SUCCESS) {
    last_error_ = Error::kEnumerateLayers;
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
  static const char* const debug_layers[] = {"VK_LAYER_KHRONOS_validation"};

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

Vulkan::Error Vulkan::last_error_ = Vulkan::Error::kUnknown;

constexpr u32 version_to_vk(config::Version version) {
  return VK_MAKE_VERSION(version.major, version.minor, version.patch);
}

}  // namespace embers