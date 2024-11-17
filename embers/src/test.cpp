#include <fmt/format.h>
#include <vulkan/vulkan.h>

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
#include "window.hpp"

using embers::containers::TestAllocator;

using namespace embers;

template <typename T>
using TestAlloc = containers::with<          //
    std::allocator,                          //
    containers::DebugAllocatorTags::kVulkan  //
    >::DebugAllocator<T>;                    //

constexpr u32 version_to_vk(embers::config::Version version) {
  return VK_MAKE_VERSION(version.major, version.minor, version.patch);
}

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

  VkInstance instance;
  VkResult   result;
  {
    using allocator = embers::containers::with<
        embers::containers::DefaultAllocator,
        embers::containers::DebugAllocatorTags::kVulkan>::
        DebugAllocator<VkExtensionProperties>;

    std::vector<VkExtensionProperties, allocator> existing_extensions;

    {
      u32 extension_count = 0;
      vkEnumerateInstanceExtensionProperties(
          nullptr,
          &extension_count,
          nullptr
      );
      existing_extensions.resize(extension_count);
      vkEnumerateInstanceExtensionProperties(
          nullptr,
          &extension_count,
          existing_extensions.data()
      );
    }

    EMBERS_DEBUG("Avaliable extensions: ");
    for (const auto& i : existing_extensions) {
      EMBERS_DEBUG("- {} ({})", i.extensionName, i.specVersion);
    }

    VkApplicationInfo app_info;
    {
      app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      app_info.pApplicationName   = config.application_name;
      app_info.applicationVersion = version_to_vk(config.version);
      app_info.pEngineName        = embers::config::engine.name;
      app_info.engineVersion = version_to_vk(embers::config::engine.version);
      app_info.apiVersion    = VK_API_VERSION_1_0;
    }

    VkInstanceCreateInfo instance_create_info{};
    {
      u32          glfw_extension_count = 0;
      const char** extensions =
          glfwGetRequiredInstanceExtensions(&glfw_extension_count);

      EMBERS_DEBUG("Required extensions: ");
      for (size_t i = 0; i < glfw_extension_count; i++) {
        EMBERS_DEBUG("- {}", extensions[i]);
      }

      for (size_t i = 0; i < glfw_extension_count; i++) {
        for (const auto& j : existing_extensions) {
          if (strcmp(  //
                  extensions[i],
                  j.extensionName
              ) == 0) {
            goto continue_outer_loop;
          }
        }
        EMBERS_DEBUG("Unable to find extension {}", extensions[i]);
      continue_outer_loop:;
      }

      instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      instance_create_info.pApplicationInfo        = &app_info;
      instance_create_info.enabledLayerCount       = 0;
      instance_create_info.enabledExtensionCount   = glfw_extension_count;
      instance_create_info.ppEnabledExtensionNames = extensions;
    }

    result = vkCreateInstance(&instance_create_info, nullptr, &instance);
  }
  EMBERS_DEBUG("Result: {:x}", (int)result);

  vkDestroyInstance(instance, nullptr);

  EMBERS_DEBUG("Size: {}", embers::containers::debug_allocator_info[0].size);
  EMBERS_DEBUG(
      "Allocations: {}",
      embers::containers::debug_allocator_info[0].allocations
  );
  EMBERS_DEBUG(
      "Deallocations: {}",
      embers::containers::debug_allocator_info[0].deallocations
  );

  return 0;
}