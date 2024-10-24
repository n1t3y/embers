#include <GLFW/glfw3.h>
#include <fmt/format.h>

#include <embers/logger.hpp>
#include <embers/test.hpp>
#include <iostream>

#include "engine_config.hpp"
#include "result.hpp"

using ErrorCodeType = u32;

enum class ErrorCode : ErrorCodeType {
  kOk                 = 0x00000000,
  kUnknown            = 0x00000001,
  kWindowInitGLFW     = 0x00000010,
  kWindowCreateWindow = 0x00000011,
};

enum class PlatformError : ErrorCodeType {
  kOk                 = (ErrorCodeType)ErrorCode::kOk,
  kUnknown            = (ErrorCodeType)ErrorCode::kUnknown,
  kWindowInitGLFW     = (ErrorCodeType)ErrorCode::kWindowInitGLFW,
  kWindowCreateWindow = (ErrorCodeType)ErrorCode::kWindowCreateWindow,
};

enum class WindowError : ErrorCodeType {
  kOk           = (ErrorCodeType)PlatformError::kOk,
  kUnknown      = (ErrorCodeType)PlatformError::kUnknown,
  kInitGLFW     = (ErrorCodeType)PlatformError::kWindowInitGLFW,
  kCreateWindow = (ErrorCodeType)PlatformError::kWindowCreateWindow,
};

class Window {
  static u32  glfw_inits_;
  GLFWwindow *window_;

  Window(GLFWwindow *window) : window_(window) {}

 public:
  Window() = delete;

  static embers::Result<Window, WindowError> create(
      u32 width, u32 height, const char *title
  ) {
    GLFWwindow *glfw_window = nullptr;
    WindowError error       = WindowError::kUnknown;
    int         glfw_error;

    glfwInitHint(GLFW_WAYLAND_LIBDECOR, GLFW_WAYLAND_PREFER_LIBDECOR);
    if (glfw_inits_ == 0) {
      if (glfwInit() != GLFW_TRUE) {
        error = WindowError::kInitGLFW;
        goto window_fail_init;
      }
      glfw_inits_++;
      EMBERS_DEBUG("Glfw initialized");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    glfw_window = glfwCreateWindow(width, height, title, NULL, NULL);

    if (glfw_window == NULL) {
      error = WindowError::kCreateWindow;
      goto window_fail_window;
    }
    EMBERS_DEBUG("Glfw window created: {}", fmt::ptr(glfw_window));
    return embers::Result<Window, WindowError>::create_ok(Window(glfw_window));
  window_fail_window:
    glfw_inits_--;
    if (glfw_inits_ == 0) {
      glfwTerminate();
    }
  window_fail_init:
    glfw_error = glfwGetError(NULL);
    EMBERS_ERROR("Unable to init platform; glfwGetError: {}", glfw_error);
    return embers::Result<Window, WindowError>::create_err(error);
  }

  Window(const Window &window) = delete;
  Window(Window &&window) {
    window_        = window.window_;
    window.window_ = nullptr;
  };
  ~Window() {
    if (window_ == nullptr) {
      return;
    }
    EMBERS_DEBUG("Glfw window destroyed: {}", fmt::ptr(window_));
    glfwDestroyWindow(window_);
    glfw_inits_--;
    if (glfw_inits_ == 0) {
      EMBERS_DEBUG("Glfw terminated");
      glfwTerminate();
    }
    EMBERS_INFO("Window terminated");
  }
};

class Platform {
  Window window_;
  bool   valid_;

  explicit Platform(Window &&window)
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

u32 Window::glfw_inits_ = 0;

embers::Result<Platform, PlatformError> Platform::create(
    const embers::config::Config &config
) {
  return Window::create(
             config.resolution.width,
             config.resolution.height,
             config.application_name
  )
      .map([](Window &&window) { return Platform(std::move(window)); })
      .inspect_err([](const auto &err) {
        EMBERS_ERROR(
            "Unable to init platform: couldn't initialize the window: {}",
            (ErrorCodeType)err
        );
      })
      .map_err([](WindowError &&err) { return (PlatformError)(err); })
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