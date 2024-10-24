#include <fmt/format.h>

#include <embers/logger.hpp>
#include <embers/test.hpp>
#include <iostream>

#include "engine_config.hpp"
#include "error_code.hpp"
#include "result.hpp"
#include "window.hpp"

enum class PlatformError : embers::ErrorCodeType {
  kOk             = (embers::ErrorCodeType)embers::ErrorCode::kOk,
  kUnknown        = (embers::ErrorCodeType)embers::ErrorCode::kUnknown,
  kWindowInitGLFW = (embers::ErrorCodeType)embers::ErrorCode::kWindowInitGLFW,
  kWindowCreateWindow =
      (embers::ErrorCodeType)embers::ErrorCode::kWindowCreateWindow,
};

class Platform {
  embers::window::Window window_;
  bool                   valid_;

  explicit Platform(embers::window::Window &&window)
      : window_(std::move(window)), valid_(true) {};

 public:
  Platform() = delete;

  Platform(const Platform &platform) = delete;
  Platform(Platform &&platform) : window_(std::move(platform.window_)) {
    valid_          = platform.valid_;
    platform.valid_ = false;
  }

  static embers::Result<Platform, PlatformError> create(
      const embers::config::Config &config
  );
  ~Platform();
};

embers::Result<Platform, PlatformError> Platform::create(
    const embers::config::Config &config
) {
  return embers::window::Window::create(
             config.resolution.width,
             config.resolution.height,
             config.application_name
  )
      .map([](embers::window::Window &&window) {
        return Platform(std::move(window));
      })
      .inspect_err([](const auto &err) {
        EMBERS_ERROR(
            "Unable to init platform: couldn't initialize the window: {}",
            (embers::ErrorCodeType)err
        );
      })
      .map_err([](embers::window::Error &&err) { return (PlatformError)(err); })
      .inspect([](const auto &platform) { EMBERS_INFO("Platform initialized"); }
      );
}

Platform::~Platform() {
  if (!valid_) {
    return;
  }

  EMBERS_INFO("Platform terminated");
}

int embers::test::main() {
  EMBERS_INFO(
      "Main called: {} ver. {} built @ " __DATE__ " " __TIME__,
      embers::config::engine_config.name,
      embers::config::engine_config.version
  );

  embers::config::Config config;

  auto platform = Platform::create(config);

  return 0;
}