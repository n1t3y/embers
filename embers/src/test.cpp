#include <GLFW/glfw3.h>
#include <fmt/format.h>

#include <embers/logger.hpp>
#include <embers/result.hpp>
#include <embers/test.hpp>
#include <iostream>

#include "engine_config.hpp"

enum class PlatformError {
  kUnknown,
  kUnableToInitGLFW,
  kUnableToCreateWindow,
};

class Platform {
  static u32  glfw_inits_;
  GLFWwindow *window_;

  Platform(GLFWwindow *window) { window_ = window; };

 public:
  Platform() = delete;

  Platform(const Platform &platform) = delete;
  Platform(Platform &&platform) {
    window_          = platform.window_;
    platform.window_ = nullptr;
  }

  static embers::Result<Platform, PlatformError> create(
      const embers::config::Config &config
  );
  ~Platform();
};

u32 Platform::glfw_inits_ = 0;

embers::Result<Platform, PlatformError> Platform::create(
    const embers::config::Config &config
) {
  PlatformError error = PlatformError::kUnknown;
  GLFWwindow   *window;

  glfwInitHint(GLFW_WAYLAND_LIBDECOR, GLFW_WAYLAND_PREFER_LIBDECOR);
  if (glfw_inits_ == 0) {
    if (glfwInit() != GLFW_TRUE) {
      error = PlatformError::kUnableToInitGLFW;
      goto embersPlatformInit_fail_init;
    }
    glfw_inits_++;
    EMBERS_DEBUG("Glfw initialized");
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(
      config.resolution.width,
      config.resolution.height,
      config.application_name,
      NULL,
      NULL
  );

  if (window == NULL) {
    error = PlatformError::kUnableToCreateWindow;
    goto embersPlatformInit_fail_window;
  }
  EMBERS_DEBUG("Glfw window created: {}", fmt::ptr(window));
  EMBERS_INFO("Platform initialized");
  return std::move(Platform(window));
  // -- embersPlatformInit error handling --
  int glfw_error;

embersPlatformInit_fail:
  glfwDestroyWindow(window);
embersPlatformInit_fail_window:
  glfw_inits_--;
  if (glfw_inits_ == 0) {
    glfwTerminate();
  }
embersPlatformInit_fail_init:

  glfw_error = glfwGetError(NULL);
  EMBERS_ERROR("Unable to init platform; glfwGetError: {}", glfw_error);
  return embers::Result<Platform, PlatformError>::create_err(error
  );  // todo(n1t3): Error handling
}

Platform::~Platform() {
  if (window_ == nullptr) {
    return;
  }
  glfwDestroyWindow(window_);
  glfw_inits_--;
  if (glfw_inits_ == 0) {
    EMBERS_DEBUG("Glfw terminated");
    glfwTerminate();
  }
  EMBERS_INFO("Platform terminated");
}

enum class ErrorCode {
  kPermissionDenied,
  kUnknown,
};

embers::Result<int, ErrorCode> get_a_thing() {
  int a;
  std::cin >> a;
  if (a > 10) {
    return embers::Result<int, ErrorCode>::create_ok(a);
  } else {
    return embers::Result<int, ErrorCode>::create_err(
        ErrorCode::kPermissionDenied
    );
  }
}
#define LOG_INFO EMBERS_INFO
// EMBERS_INFO(
//     "Main called: {} ver. {} built @ " __DATE__ " " __TIME__,
//     embers::config::engine_config.name,
//     embers::config::engine_config.version
// );

// embers::config::Config config;

// auto platform = Platform::create(config);

int embers::test::main() {
  auto test = Result<int, const char *>::create_err("Nope");
  LOG_INFO("Result: {}", test);

  return 0;
}