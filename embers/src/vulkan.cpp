#include "vulkan.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "containers/allocator.hpp"
#include "engine_config.hpp"

constexpr u32 version_to_vk(embers::config::Version version) {
  return VK_MAKE_VERSION(version.major, version.minor, version.patch);
}

namespace embers {

Vulkan::Vulkan(const config::Platform& config) {
  // (n1t3)todo: should be a map of string views
  Vector<VkExtensionProperties> existing_extension_properties = {};
  // (n1t3)todo: consider using a set
  Vector<const char*>  extensions           = {};
  VkApplicationInfo    app_info             = {};
  VkInstanceCreateInfo instance_create_info = {};
  VkResult             result               = VK_SUCCESS;

  extensions.reserve(
      5 + config.required_extensions.length + config.optional_extensions.length
  );

  u32 extension_count = 0;
  result              = vkEnumerateInstanceExtensionProperties(  //
      nullptr,
      &extension_count,
      nullptr
  );
  existing_extension_properties.resize(extension_count);
  result = result == VK_SUCCESS  //
               ? vkEnumerateInstanceExtensionProperties(
                     nullptr,
                     &extension_count,
                     existing_extension_properties.data()
                 )
               : result;

  if (result != VK_SUCCESS) {
    last_error_ = Error::kEnumerateExtensions;
    goto vulkan_create_error;
  }

  // nessesary
  {
    u32                glfw_extensions_length = 0;
    const char* const* glfw_extensions =
        glfwGetRequiredInstanceExtensions(&glfw_extensions_length);

    if (glfw_extensions_length == 0) {
      last_error_ = Error::kEnumerateExtensions;
      goto vulkan_create_error;
    }

    for (u32 i = 0; i < glfw_extensions_length; i++) {
      for (const auto& existing_extension : existing_extension_properties) {
        if (strcmp(  //
                glfw_extensions[i],
                existing_extension.extensionName
            ) == 0) {
          goto continue_outer_loop_1;  // found, next
        }
      }
      EMBERS_FATAL(
          "Unable to init Vulkan; glfw extension was not found: {}",
          glfw_extensions[i]
      );
      last_error_ = Error::kRequiredExtensionsArentPresent;
      goto vulkan_create_error;
    continue_outer_loop_1:;
    }
    for (u32 i = 0; i < config.required_extensions.length; i++) {
      for (const auto& existing_extension : existing_extension_properties) {
        if (strcmp(  //
                config.required_extensions.array[i],
                existing_extension.extensionName
            ) == 0) {
          goto continue_outer_loop_2;  // found, next
        }
      }
      EMBERS_FATAL(
          "Unable to init Vulkan; requested extension was not found: {}",
          config.required_extensions.array[i]
      );
      goto vulkan_create_error;
    continue_outer_loop_2:;
    }
    extensions.insert(
        extensions.end(),
        glfw_extensions,
        glfw_extensions + glfw_extensions_length
    );
    extensions.insert(
        extensions.end(),
        config.required_extensions.array,
        config.required_extensions.array + config.required_extensions.length
    );
  }

  // optional
  {
    for (u32 i = 0; i < config.optional_extensions.length; i++) {
      for (const auto& existing_extension : existing_extension_properties) {
        if (strcmp(  //
                config.optional_extensions.array[i],
                existing_extension.extensionName
            ) == 0) {
          extensions.push_back(config.optional_extensions.array[i]);
          goto continue_outer_loop_3;  // found, next
        }
      }
      EMBERS_ERROR(
          "Unable to properly initialize Vulkan; requested extension was not"
          " found: {}, skip ",
          config.optional_extensions.array[i]
      );
    continue_outer_loop_3:;
    }
  }

#ifdef EMBERS_CONFIG_DEBUG
  // debug
  {
    static const char* const debug_extensions[] = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };
    const u32 len = sizeof(debug_extensions) / sizeof(const char*);

    for (u32 i = 0; i < len; i++) {
      for (const auto& existing_extension : existing_extension_properties) {
        if (strcmp(  //
                debug_extensions[i],
                existing_extension.extensionName
            ) == 0) {
          extensions.push_back(debug_extensions[i]);
          goto continue_outer_loop_4;  // found, next
        }
      }
      EMBERS_ERROR(
          "Unable to properly initialize Vulkan; debug extension was not "
          "found: {}, skip",
          debug_extensions[i]
      );
    continue_outer_loop_4:;
    }
  }
#endif

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
    goto vulkan_create_error;
  }
  EMBERS_INFO("Vulkan initialized");
  return;

vulkan_create_error:
  instance_ = nullptr;
  return;
}

void Vulkan::destroy() {
  vkDestroyInstance(instance_, nullptr);  // (n1t3)todo allocator callbacks
  EMBERS_INFO("Vulkan terminated");
  return;
}

Vulkan::Error Vulkan::last_error_ = Vulkan::Error::kUnknown;

}  // namespace embers