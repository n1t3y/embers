#include "debug_messenger.hpp"

#ifdef EMBERS_CONFIG_DEBUG

#include <embers/logger.hpp>

#define EMBERS__VULKAN_DEBUG_CALLBACK_FORMAT "{}"

namespace embers::vulkan {

VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger::debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
    VkDebugUtilsMessageTypeFlagsEXT             message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void*                                       user_data
) {
  static const char filename[] = "vulkan.txt";

  switch (message_severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
      EMBERS_DEBUG_INTO(
          filename,
          EMBERS__VULKAN_DEBUG_CALLBACK_FORMAT,
          callback_data->pMessage
      );
      break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
      EMBERS_INFO_INTO(
          filename,
          EMBERS__VULKAN_DEBUG_CALLBACK_FORMAT,
          callback_data->pMessage
      );
      break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
      EMBERS_WARN_INTO(
          filename,
          EMBERS__VULKAN_DEBUG_CALLBACK_FORMAT,
          callback_data->pMessage
      );
      break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
      EMBERS_ERROR_INTO(
          filename,
          EMBERS__VULKAN_DEBUG_CALLBACK_FORMAT,
          callback_data->pMessage
      );
      break;
    }
    default: {
      break;
    }
  }

  return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT DebugMessenger::create_info = {
    VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    nullptr,
    {},
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
    debug_callback,
    nullptr
};

Error DebugMessenger::last_error_ = Error::kOk;

}  // namespace embers::vulkan

#endif