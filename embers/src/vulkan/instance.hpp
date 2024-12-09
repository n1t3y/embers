#pragma once

#include <embers/config.hpp>
#include <vector>

#include "../error_code.hpp"
#include "common.hpp"


struct VkInstance_T;
struct VkPhysicalDevice_T;

typedef VkInstance_T*       VkInstance;
typedef VkPhysicalDevice_T* VkPhysicalDevice;

namespace embers::vulkan {

class Instance {
  static Error last_error_;
  VkInstance   instance_;

  void destroy();

 public:
  Instance() = delete;
  Instance(const config::Platform& config);
  Instance(const Instance&) = delete;
  constexpr Instance(Instance&& other);
  inline ~Instance();

  constexpr explicit                operator bool() const;
  constexpr explicit                operator VkInstance() const;
  Instance&                         operator=(const Instance& rhs) = delete;
  constexpr Instance&               operator=(Instance&& rhs);
  constexpr bool                    operator==(const Instance& rhs) const;
  constexpr bool                    operator!=(const Instance& rhs) const;
  EMBERS_ALWAYS_INLINE static Error get_last_error();

 private:
 public:  // todo
  static Vector<const char*> get_extension_list(const config::Platform& config);
  static Vector<const char*> get_layer_list(const config::Platform& config);
  static Vector<const char*> get_device_extension_list(
      VkPhysicalDevice device, const config::Platform& config
  );
  // static Vector<const char*> get_device_layer_list(
  //     VkPhysicalDevice device, const config::Platform& config
  // );

  Vector<VkPhysicalDevice> get_device_list() const;
  static VkPhysicalDevice  pick_device(const Vector<VkPhysicalDevice>& devices);
};

}  // namespace embers::vulkan

namespace embers::vulkan {

constexpr Instance::Instance(Instance&& other) : instance_(other.instance_) {
  other.instance_ = nullptr;
}

inline Instance::~Instance() {
  if (instance_ == nullptr) {
    return;
  }
  destroy();
  return;
}

constexpr Instance::operator bool() const { return instance_ != nullptr; }
constexpr Instance::operator VkInstance() const { return instance_; }

constexpr Instance& Instance::operator=(Instance&& rhs) {
  instance_     = rhs.instance_;
  rhs.instance_ = nullptr;
  return *this;
}
constexpr bool Instance::operator==(const Instance& rhs) const {
  return instance_ == rhs.instance_;
}
constexpr bool Instance::operator!=(const Instance& rhs) const {
  return !operator==(rhs);
}
EMBERS_ALWAYS_INLINE Error Instance::get_last_error() { return last_error_; }

}  // namespace embers::vulkan