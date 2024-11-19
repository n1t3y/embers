#pragma once
#include <embers/config.hpp>
#include <embers/defines.hpp>
#include <embers/logger.hpp>

#include "error_code.hpp"
#include "vulkan.hpp"
#include "vulkan_debug_messenger.hpp"
#include "window.hpp"

namespace embers {

class Platform {
 public:
  enum class Error : ErrorType {
    kOk      = (ErrorType)embers::Error::kOk,
    kUnknown = (ErrorType)embers::Error::kUnknown,
    // propagate from window::Window
    kInitGLFW = (ErrorType)embers::Error::kWindowInitGLFW,
    // propagate from window::Window
    kCreateWindow = (ErrorType)embers::Error::kWindowCreateWindow,
  };

 private:
  static Error last_error_;
  Window       window_;
  Vulkan       vulkan_;
#ifdef EMBERS_CONFIG_DEBUG
  VulkanDebugMessenger debug_messenger_;
#endif

 public:
  Platform() = delete;
  inline Platform(const config::Platform &config);
  Platform(const Platform &platform) = delete;
  constexpr Platform(Platform &&platform);
  inline ~Platform();

  constexpr explicit                operator bool() const;
  Platform                         &operator=(const Platform &rhs) = delete;
  constexpr Platform               &operator=(Platform &&rhs);
  constexpr bool                    operator==(const Platform &rhs) const;
  constexpr bool                    operator!=(const Platform &rhs) const;
  EMBERS_ALWAYS_INLINE static Error get_last_error();
};

}  // namespace embers

// implementation

namespace embers {

inline Platform::Platform(const config::Platform &config)
    : window_(
          config.resolution.width,
          config.resolution.height,
          config.application_name
      ),
      vulkan_(config)
#ifdef EMBERS_CONFIG_DEBUG
      ,
      debug_messenger_(vulkan_)
#endif
{
  embers::Error err = embers::Error::kOk;

  if (!(bool)window_) {
    err = to_error_code(Window::get_last_error());
    goto platform_create_error;
  }
  if (!(bool)vulkan_) {
    auto err = to_error_code(Vulkan::get_last_error());
    goto platform_create_error;
  }
#ifdef EMBERS_CONFIG_DEBUG
  if (!(bool)debug_messenger_) {
    auto err = to_error_code(VulkanDebugMessenger::get_last_error());
    // goto platform_create_error; // non critical, can continue
  }
#endif
  EMBERS_INFO("Platform initialized");
  return;
platform_create_error:
  EMBERS_FATAL("Unable to init embers platform: {}", err);
  return;
}

constexpr Platform::Platform(Platform &&platform)
    : window_(std::move(platform.window_)),
      vulkan_(std::move(platform.vulkan_)),
      debug_messenger_(std::move(platform.debug_messenger_)) {}

constexpr Platform::operator bool() const {
  return (bool)window_ &&  //
         (bool)vulkan_;
};

inline Platform::~Platform() {
  EMBERS_INFO("Platform terminated");
  return;
}

constexpr Platform &Platform::operator=(Platform &&rhs) {
  window_          = std::move(rhs.window_);
  vulkan_          = std::move(rhs.vulkan_);
  debug_messenger_ = std::move(debug_messenger_);
  return *this;
};

constexpr bool Platform::operator==(const Platform &rhs) const {
  return window_ == rhs.window_ &&  //
         vulkan_ == rhs.vulkan_ &&  //
         true;
}

constexpr bool Platform::operator!=(const Platform &rhs) const {
  return !operator==(rhs);
}

EMBERS_ALWAYS_INLINE Platform::Error Platform::get_last_error() {
  return last_error_;
}

}  // namespace embers