#pragma once

#include "instance.hpp"
#include "surface.hpp"

typedef struct VkDevice_T* VkDevice;
typedef struct VkQueue_T*  VkQueue;

namespace embers::vulkan {

class Device {
  static Error last_error_;
  VkDevice     device_;
  struct {
    VkQueue graphics;
    VkQueue transfer;
    VkQueue present;
    VkQueue compute;
  } queues_;

  void destroy();

 public:
  Device() = delete;
  Device(
      const Instance&         instance,
      const Surface&          surface,
      const config::Platform& config
  );
  Device(const Device& other) = delete;
  constexpr Device(Device&& other);
  inline ~Device();

  constexpr explicit                operator bool() const;
  constexpr explicit                operator VkDevice() const;
  Device&                           operator=(const Device& rhs) = delete;
  constexpr Device&                 operator=(Device&& rhs);
  constexpr bool                    operator==(const Device& rhs) const;
  constexpr bool                    operator!=(const Device& rhs) const;
  EMBERS_ALWAYS_INLINE static Error get_last_error();
};

}  // namespace embers::vulkan

namespace embers::vulkan {

constexpr Device::Device(Device&& other)
    : device_(other.device_), queues_(other.queues_) {
  other.device_ = nullptr;
  return;
}

Device::~Device() {
  if (device_ == nullptr) {
    return;
  }
  destroy();
  return;
}

constexpr Device::operator bool() const { return device_ != nullptr; }

constexpr Device::operator VkDevice() const { return device_; }

constexpr Device& Device::operator=(Device&& rhs) {
  device_     = rhs.device_;
  rhs.device_ = nullptr;
  return *this;
}

constexpr bool Device::operator==(const Device& rhs) const {
  return device_ == rhs.device_;
}
constexpr bool Device::operator!=(const Device& rhs) const {
  return !operator==(rhs);
}
EMBERS_ALWAYS_INLINE Error Device::get_last_error() { return last_error_; }

}  // namespace embers::vulkan