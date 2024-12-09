#pragma once
#include <embers/config.hpp>
#include <embers/defines.hpp>
#include <embers/logger.hpp>

#include "error_code.hpp"
#include "vulkan/debug_messenger.hpp"
#include "vulkan/device.hpp"
#include "vulkan/surface.hpp"
#include "window.hpp"

namespace embers {

class Platform {
 public:  // todo remove public
  static Error     last_error_;
  Window           window_;
  vulkan::Instance vulkan_;
  vulkan::Surface  surface_;
  vulkan::Device   device_;
#ifdef EMBERS_CONFIG_DEBUG
  vulkan::DebugMessenger debug_messenger_;
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
      vulkan_(config),
      surface_(vulkan_, window_),
      device_(vulkan_, surface_, config)
#ifdef EMBERS_CONFIG_DEBUG
      ,
      debug_messenger_(vulkan_)
#endif
{
  // embers::Error err = embers::Error::kOk;

  if (!(bool)window_) {
    last_error_ = Window::get_last_error();
    goto platform_create_error;
  }
  if (!(bool)vulkan_) {
    last_error_ = vulkan::Instance::get_last_error();
    goto platform_create_error;
  }
  if (!(bool)surface_) {
    last_error_ = vulkan::Surface::get_last_error();
    goto platform_create_error;
  }

  EMBERS_INFO("Platform initialized");
  return;
platform_create_error:
  EMBERS_FATAL("Unable to init embers platform: {}", last_error_);
  return;
}

constexpr Platform::Platform(Platform &&platform)
    : window_(std::move(platform.window_)),
      vulkan_(std::move(platform.vulkan_)),
      surface_(std::move(platform.surface_)),
      device_(std::move(platform.device_))
#ifdef EMBERS_CONFIG_DEBUG
      ,
      debug_messenger_(std::move(platform.debug_messenger_))
#endif

{
}

constexpr Platform::operator bool() const {
  return (bool)window_ &&  //
         (bool)vulkan_;
};

inline Platform::~Platform() {
  EMBERS_INFO("Platform terminated");
  return;
}

constexpr Platform &Platform::operator=(Platform &&rhs) {
  window_ = std::move(rhs.window_);
  vulkan_ = std::move(rhs.vulkan_);
#ifdef EMBERS_CONFIG_DEBUG
  debug_messenger_ = std::move(debug_messenger_);
#endif
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

EMBERS_ALWAYS_INLINE Error Platform::get_last_error() { return last_error_; }

}  // namespace embers