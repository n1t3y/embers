#include <GLFW/glfw3.h>
#include <fmt/format.h>

#include <embers/logger.hpp>
#include <embers/test.hpp>
#include <iostream>

#include "engine_config.hpp"

class Platform {
  static u32  glfw_inits_;
  GLFWwindow *window_;

  Platform(GLFWwindow *window) { window_ = window; };

 public:
  Platform() = delete;

  static Platform create(const embers::config::Config &config);
  ~Platform();
};

u32 Platform::glfw_inits_ = 0;

Platform Platform::create(const embers::config::Config &config) {
  GLFWwindow *window;

  glfwInitHint(GLFW_WAYLAND_LIBDECOR, GLFW_WAYLAND_PREFER_LIBDECOR);
  if (glfw_inits_ == 0) {
    if (glfwInit() != GLFW_TRUE) {
      goto embersPlatformInit_fail_init;
    }
    glfw_inits_++;
    EMBERS_DEBUG("Glfw initialized");
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(config.resolution.width, config.resolution.height, config.application_name, NULL, NULL);

  if (window == NULL) {
    goto embersPlatformInit_fail_window;
  }
  EMBERS_DEBUG("Glfw window created: {}", fmt::ptr(window));
  EMBERS_INFO("Platform initialized");
  return Platform(window);
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
  return false;  // todo(n1t3): Error handling
}

Platform::~Platform() {
  glfwDestroyWindow(window_);
  glfw_inits_--;
  if (glfw_inits_ == 0) {
    EMBERS_DEBUG("Glfw terminated");
    glfwTerminate();
  }
  EMBERS_INFO("Platform terminated");
}

int embers::test::main() {
  EMBERS_INFO(
      "Main called: {} ver. {} built @ " __DATE__ " " __TIME__, embers::config::engine_config.name,
      embers::config::engine_config.version
  );

  embers::config::Config config;

  Platform platform = Platform::create(config);

  return 0;
}