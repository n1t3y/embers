#pragma once

#ifdef EMBERS_CONFIG_DEBUG

#include <vulkan/vulkan.h>

#include "../error_code.hpp"
#include "instance.hpp"

struct VkDebugUtilsMessengerEXT_T;
typedef VkDebugUtilsMessengerEXT_T* VkDebugUtilsMessengerEXT;

namespace embers::vulkan {

class DebugMessenger {
  static Error             last_error_;
  VkDebugUtilsMessengerEXT debug_utils_messenger_;
  VkInstance               instance_;  // doesn't own

  void destroy();

  static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
      VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
      VkDebugUtilsMessageTypeFlagsEXT             message_type,
      const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
      void*                                       user_data
  );

 public:
  DebugMessenger() = delete;
  constexpr DebugMessenger(const Instance& instance);
  DebugMessenger(const DebugMessenger& other) = delete;
  constexpr DebugMessenger(DebugMessenger&& other);
  inline ~DebugMessenger();

  constexpr explicit        operator bool() const;
  constexpr explicit        operator VkDebugUtilsMessengerEXT() const;
  DebugMessenger&           operator=(const DebugMessenger& rhs) = delete;
  constexpr DebugMessenger& operator=(DebugMessenger&& rhs);
  constexpr bool            operator==(const DebugMessenger& rhs) const;
  constexpr bool            operator!=(const DebugMessenger& rhs) const;
  EMBERS_ALWAYS_INLINE static Error get_last_error();

  static VkDebugUtilsMessengerCreateInfoEXT create_info;
};

}  // namespace embers::vulkan

namespace embers::vulkan {

constexpr DebugMessenger::DebugMessenger(const Instance& vulkan)
    : debug_utils_messenger_(nullptr), instance_((VkInstance)vulkan) {
  if (!(bool)vulkan) {
    EMBERS_FATAL(
        "Can't init debug messenger; Vulkan instance must be valid in order to "
        "construct debug messenger"
    );
    debug_utils_messenger_ = nullptr;
    return;
  }

  PFN_vkCreateDebugUtilsMessengerEXT create_debug_utils_messenger = nullptr;

  create_debug_utils_messenger = (PFN_vkCreateDebugUtilsMessengerEXT
  )vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT");

  if (create_debug_utils_messenger == nullptr) {
    debug_utils_messenger_ = nullptr;
    EMBERS_FATAL(
        "Unable to properly initialize Vulkan debug messenger; "
        "vkCreateDebugUtilsMessengerEXT procedure was not found"
    );
    return;
  }

  VkResult result = create_debug_utils_messenger(
      instance_,
      &create_info,
      nullptr,  // todo allocators
      &debug_utils_messenger_
  );

  if (result != VK_SUCCESS) {
    debug_utils_messenger_ = nullptr;
    EMBERS_FATAL(
        "Unable to properly initialize Vulkan debug messenger; "
        "vkCreateDebugUtilsMessengerEXT failed with result {}",
        (int)result
    );
    return;
  }

  return;
}

constexpr DebugMessenger::DebugMessenger(DebugMessenger&& other)
    : debug_utils_messenger_(other.debug_utils_messenger_),
      instance_(other.instance_) {
  other.debug_utils_messenger_ = nullptr;
}

DebugMessenger::~DebugMessenger() {
  if (debug_utils_messenger_ == nullptr) {
    return;
  }
  auto destroy_debug_utils_messenger = (PFN_vkDestroyDebugUtilsMessengerEXT
  )vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT");

  if (destroy_debug_utils_messenger != nullptr) {
    destroy_debug_utils_messenger(instance_, debug_utils_messenger_, nullptr);
  } else {
    EMBERS_ERROR(
        "Unable to properly destroy Vulkan debug messenger; "
        "vkDestroyDebugUtilsMessengerEXT procedure was not found"
    );
    last_error_ = Error::kVulkanGetInstanceProcAddr;
  }

  return;
}

constexpr DebugMessenger::operator bool() const {
  return debug_utils_messenger_ != nullptr;
}
constexpr DebugMessenger::operator VkDebugUtilsMessengerEXT() const {
  return debug_utils_messenger_;
}

constexpr DebugMessenger& DebugMessenger::operator=(DebugMessenger&& rhs) {
  debug_utils_messenger_     = rhs.debug_utils_messenger_;
  rhs.debug_utils_messenger_ = nullptr;
  return *this;
}
constexpr bool DebugMessenger::operator==(const DebugMessenger& rhs) const {
  return debug_utils_messenger_ == rhs.debug_utils_messenger_;
}
constexpr bool DebugMessenger::operator!=(const DebugMessenger& rhs) const {
  return !operator==(rhs);
}
EMBERS_ALWAYS_INLINE Error DebugMessenger::get_last_error() {
  return last_error_;
}

}  // namespace embers::vulkan

#endif