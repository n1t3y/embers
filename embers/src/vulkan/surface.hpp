#pragma once

#include "../window.hpp"
#include "instance.hpp"

struct VkSurfaceKHR_T;
typedef VkSurfaceKHR_T* VkSurfaceKHR;

namespace embers::vulkan {

class Surface {
  static Error last_error_;
  VkSurfaceKHR surface_;
  VkInstance   instance_;  // doesn't own

  void destroy();

 public:
  Surface() = delete;
  Surface(const Instance& instance, const Window& window);
  Surface(const Surface& other) = delete;
  constexpr Surface(Surface&& other);
  inline ~Surface();

  constexpr explicit                operator bool() const;
  constexpr explicit                operator VkSurfaceKHR() const;
  Surface&                          operator=(const Surface& rhs) = delete;
  constexpr Surface&                operator=(Surface&& rhs);
  constexpr bool                    operator==(const Surface& rhs) const;
  constexpr bool                    operator!=(const Surface& rhs) const;
  EMBERS_ALWAYS_INLINE static Error get_last_error();
};

}  // namespace embers::vulkan

namespace embers::vulkan {

constexpr Surface::Surface(Surface&& other)
    : surface_(other.surface_), instance_(other.instance_) {
  other.surface_ = nullptr;
  return;
}

Surface::~Surface() {
  if (surface_ == nullptr) {
    return;
  }
  destroy();
  return;
}

constexpr Surface::operator bool() const { return surface_ != nullptr; }

constexpr Surface::operator VkSurfaceKHR() const { return surface_; }

constexpr Surface& Surface::operator=(Surface&& rhs) {
  surface_     = rhs.surface_;
  rhs.surface_ = nullptr;
  return *this;
}

constexpr bool Surface::operator==(const Surface& rhs) const {
  return surface_ == rhs.surface_;
}
constexpr bool Surface::operator!=(const Surface& rhs) const {
  return !operator==(rhs);
}
EMBERS_ALWAYS_INLINE Error Surface::get_last_error() { return last_error_; }

}  // namespace embers::vulkan