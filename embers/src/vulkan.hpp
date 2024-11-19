#pragma once

#include <embers/config.hpp>
#include <vector>

#include "containers/allocator.hpp"
#include "containers/debug_allocator.hpp"
#include "error_code.hpp"

struct VkInstance_T;
struct VkDebugUtilsMessengerEXT_T;

typedef VkInstance_T*               VkInstance;
typedef VkDebugUtilsMessengerEXT_T* VkDebugUtilsMessengerEXT;

namespace embers {
class Vulkan {
#ifdef EMBERS_CONFIG_DEBUG
  template <typename T>
  using Allocator = containers::with<
      containers::DefaultAllocator,
      containers::DebugAllocatorTags::kVulkan>::DebugAllocator<T>;

#else
  template <typename T>
  using Allocator = embers::containers::DefaultAllocator<T>;
#endif

  template <typename T>
  using Vector = std::vector<T, Allocator<T>>;

 public:
  enum class Error : ErrorType {
    kOk                  = (ErrorType)embers::Error::kOk,
    kUnknown             = (ErrorType)embers::Error::kUnknown,
    kInitVulkan          = (ErrorType)embers::Error::kVulkanInitVulkan,
    kEnumerateExtensions = (ErrorType)embers::Error::kVulkanEnumerateExtensions,
    kGLFWGetRequiredExtensions =
        (ErrorType)embers::Error::kVulkanGLFWGetRequiredExtensions,
    kRequiredExtensionsArentPresent =
        (ErrorType)embers::Error::kVulkanRequiredExtensionsArentPresent,

    kEnumerateLayers = (ErrorType)embers::Error::kVulkanEnumerateLayers,
    kRequiredLayersArentPresent =
        (ErrorType)embers::Error::kVulkanRequiredLayersArentPresent,
  };

 private:
  static Error             last_error_;
  VkInstance               instance_;
  VkDebugUtilsMessengerEXT debug_utils_messenger_;  // todo conditional builds

  void destroy();

 public:
  Vulkan() = delete;
  Vulkan(const config::Platform& config);
  Vulkan(const Vulkan&) = delete;
  constexpr Vulkan(Vulkan&& other);
  inline ~Vulkan();

  constexpr explicit                operator bool() const;
  constexpr explicit                operator VkInstance() const;
  Vulkan&                           operator=(const Vulkan& rhs) = delete;
  constexpr Vulkan&                 operator=(Vulkan&& rhs);
  constexpr bool                    operator==(const Vulkan& rhs) const;
  constexpr bool                    operator!=(const Vulkan& rhs) const;
  EMBERS_ALWAYS_INLINE static Error get_last_error();

 private:
  static Vector<const char*> get_extension_list(const config::Platform& config);
  static Vector<const char*> get_layer_list(const config::Platform& config);
};

}  // namespace embers

namespace embers {

constexpr Vulkan::Vulkan(Vulkan&& other)
    : instance_(other.instance_),
      debug_utils_messenger_(other.debug_utils_messenger_) {
  other.instance_              = nullptr;
  other.debug_utils_messenger_ = nullptr;
}

inline Vulkan::~Vulkan() {
  if (instance_ == nullptr) {
    return;
  }
  destroy();
  return;
}

constexpr Vulkan::operator bool() const { return instance_ != nullptr; }
constexpr Vulkan::operator VkInstance() const { return instance_; }

constexpr Vulkan& Vulkan::operator=(Vulkan&& rhs) {
  instance_     = rhs.instance_;
  rhs.instance_ = nullptr;
  return *this;
}
constexpr bool Vulkan::operator==(const Vulkan& rhs) const {
  return instance_ == rhs.instance_;
}
constexpr bool Vulkan::operator!=(const Vulkan& rhs) const {
  return !operator==(rhs);
}
EMBERS_ALWAYS_INLINE Vulkan::Error Vulkan::get_last_error() {
  return last_error_;
}

}  // namespace embers