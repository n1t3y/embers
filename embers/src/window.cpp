#include "window.hpp"

#include <GLFW/glfw3.h>

#include <embers/logger.hpp>

namespace embers {

Window::Window(u32 width, u32 height, const char *title) {
  Error error = Error::kUnknown;
  int   glfw_error;

  // GLFW
  glfwInitHint(GLFW_WAYLAND_LIBDECOR, GLFW_WAYLAND_PREFER_LIBDECOR);
  if (glfw_inits_ == 0) {
    if (glfwInit() != GLFW_TRUE) {
      error = Error::kWindowInitGLFW;
      goto window_fail_init;
    }
    glfw_inits_++;
    EMBERS_DEBUG("Glfw initialized");
  }

  // Window
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  window_ = glfwCreateWindow(width, height, title, NULL, NULL);
  if (window_ == NULL) {
    error = Error::kWindowCreateWindow;
    goto window_fail_window;
  }
  EMBERS_DEBUG("Glfw window created: {}", fmt::ptr(window_));

  // Everything is fine
  return;
  // if it is not fine:
window_fail_window:
  glfw_inits_--;
  if (glfw_inits_ == 0) {
    glfwTerminate();
  }
window_fail_init:
  glfw_error = glfwGetError(NULL);
  EMBERS_FATAL(
      "Unable to init Embers window; glfwGetError: {}; Zombie window is going "
      "to be created",
      glfw_error
  );

  window_     = nullptr;
  last_error_ = error;
  return;
}

void Window::destroy() {
  EMBERS_DEBUG("Glfw window destroyed: {}", fmt::ptr(window_));
  glfwDestroyWindow(window_);
  glfw_inits_--;
  if (glfw_inits_ == 0) {
    EMBERS_DEBUG("Glfw terminated");
    glfwTerminate();
  }
  EMBERS_INFO("Embers Window terminated");
  return;
}

u32   Window::glfw_inits_ = 0;
Error Window::last_error_ = Error::kUnknown;

}  // namespace embers