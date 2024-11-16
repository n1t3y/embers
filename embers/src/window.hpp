#pragma once
#include <embers/defines.hpp>
#include "error_code.hpp"
#include "result.hpp"

typedef struct GLFWwindow GLFWwindow;

namespace embers {

class Window {
 public:
  enum class Error : ErrorType {
    kOk           = (ErrorType)embers::Error::kOk,
    kUnknown      = (ErrorType)embers::Error::kUnknown,
    kInitGLFW     = (ErrorType)embers::Error::kWindowInitGLFW,
    kCreateWindow = (ErrorType)embers::Error::kWindowCreateWindow,
  };

 private:
  static Error last_error_;
  static u32   glfw_inits_;
  GLFWwindow  *window_;

  constexpr Window(GLFWwindow *window) : window_(window) {}

 public:
  Window()                     = delete;
  Window(const Window &window) = delete;
  constexpr Window(Window &&window);
  ~Window();

  static Result<Window, Error> create(u32 width, u32 height, const char *title);
};
}  // namespace embers::window

namespace embers::window {
constexpr Window::Window(Window &&window) : window_(window.window_) {
  window_ = nullptr;
}
}  // namespace embers::window