#include "window.hpp"

#include <GLFW/glfw3.h>

#include <embers/logger.hpp>

namespace embers {

Result<Window, Error> Window::create(u32 width, u32 height, const char *title) {
  GLFWwindow *glfw_window = nullptr;
  Error       error       = Error::kUnknown;
  int         glfw_error;

  glfwInitHint(GLFW_WAYLAND_LIBDECOR, GLFW_WAYLAND_PREFER_LIBDECOR);
  if (glfw_inits_ == 0) {
    if (glfwInit() != GLFW_TRUE) {
      error = Error::kInitGLFW;
      goto window_fail_init;
    }
    glfw_inits_++;
    EMBERS_DEBUG("Glfw initialized");
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  glfw_window = glfwCreateWindow(width, height, title, NULL, NULL);

  if (glfw_window == NULL) {
    error = Error::kCreateWindow;
    goto window_fail_window;
  }
  EMBERS_DEBUG("Glfw window created: {}", fmt::ptr(glfw_window));
  return embers::Result<Window, Error>::create_ok(Window(glfw_window));
window_fail_window:
  glfw_inits_--;
  if (glfw_inits_ == 0) {
    glfwTerminate();
  }
window_fail_init:
  glfw_error = glfwGetError(NULL);
  EMBERS_ERROR("Unable to init platform; glfwGetError: {}", glfw_error);
  return embers::Result<Window, Error>::create_err(error);
}

Window::~Window() {
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

u32 embers::window::Window::glfw_inits_ = 0;

}  // namespace embers