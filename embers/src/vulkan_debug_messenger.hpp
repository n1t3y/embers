#pragma once

#include <vulkan/vulkan.h>

#include "error_code.hpp"
#include "vulkan.hpp"

struct VkDebugUtilsMessengerEXT_T;
typedef VkDebugUtilsMessengerEXT_T* VkDebugUtilsMessengerEXT;

namespace embers {

class VulkanDebugMessenger {
 public:
  enum class Error : ErrorType {
    kOk      = (ErrorType)embers::Error::kOk,
    kUnknown = (ErrorType)embers::Error::kUnknown
  };

 private:
  static Error             last_error_;
  VkDebugUtilsMessengerEXT debug_utils_messenger_;  // todo conditional builds
  VkInstance               instance_;               // doesn't own

  void destroy();

  static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
      VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
      VkDebugUtilsMessageTypeFlagsEXT             message_type,
      const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
      void*                                       user_data
  );

 public:
  VulkanDebugMessenger() = delete;
  constexpr VulkanDebugMessenger(const Vulkan& vulkan);
  VulkanDebugMessenger(const VulkanDebugMessenger& other) = delete;
  constexpr VulkanDebugMessenger(VulkanDebugMessenger&& other);
  inline ~VulkanDebugMessenger();

  constexpr explicit    operator bool() const;
  constexpr explicit    operator VkDebugUtilsMessengerEXT() const;
  VulkanDebugMessenger& operator=(const VulkanDebugMessenger& rhs) = delete;
  constexpr VulkanDebugMessenger& operator=(VulkanDebugMessenger&& rhs);
  constexpr bool operator==(const VulkanDebugMessenger& rhs) const;
  constexpr bool operator!=(const VulkanDebugMessenger& rhs) const;
  EMBERS_ALWAYS_INLINE static Error get_last_error();

  static VkDebugUtilsMessengerCreateInfoEXT create_info;
};

}  // namespace embers

namespace embers {

constexpr VulkanDebugMessenger::VulkanDebugMessenger(const Vulkan& vulkan)
    : debug_utils_messenger_(nullptr), instance_((VkInstance)vulkan) {
  PFN_vkCreateDebugUtilsMessengerEXT create_debug_utils_messenger = nullptr;

  create_debug_utils_messenger = (PFN_vkCreateDebugUtilsMessengerEXT
  )vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT");
  // todo do checks

  VkResult result = create_debug_utils_messenger(
      instance_,
      &create_info,
      nullptr,  // todo allocators
      &debug_utils_messenger_
  );
  // todo check result
  return;
}

constexpr VulkanDebugMessenger::VulkanDebugMessenger(
    VulkanDebugMessenger&& other
)
    : debug_utils_messenger_(other.debug_utils_messenger_),
      instance_(other.instance_) {
  other.debug_utils_messenger_ = nullptr;
}

VulkanDebugMessenger::~VulkanDebugMessenger() {
  if (debug_utils_messenger_ == nullptr) {
    return;
  }
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT
  )vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT");
  // todo checks

  func(instance_, debug_utils_messenger_, nullptr);

  return;
}

constexpr VulkanDebugMessenger::operator bool() const {
  return debug_utils_messenger_ != nullptr;
}
constexpr VulkanDebugMessenger::operator VkDebugUtilsMessengerEXT() const {
  return debug_utils_messenger_;
}

constexpr VulkanDebugMessenger& VulkanDebugMessenger::operator=(
    VulkanDebugMessenger&& rhs
) {
  debug_utils_messenger_     = rhs.debug_utils_messenger_;
  rhs.debug_utils_messenger_ = nullptr;
  return *this;
}
constexpr bool VulkanDebugMessenger::operator==(const VulkanDebugMessenger& rhs
) const {
  return debug_utils_messenger_ == rhs.debug_utils_messenger_;
}
constexpr bool VulkanDebugMessenger::operator!=(const VulkanDebugMessenger& rhs
) const {
  return !operator==(rhs);
}
EMBERS_ALWAYS_INLINE VulkanDebugMessenger::Error
                     VulkanDebugMessenger::get_last_error() {
  return last_error_;
}

}  // namespace embers