#pragma once
#include <embers/defines.hpp>

#include "error_code.hpp"

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

  void destroy();

 public:
  Window() = delete;
  Window(u32 width, u32 height, const char *title);
  Window(const Window &window) = delete;
  constexpr Window(Window &&window);
  inline ~Window();

  constexpr explicit                operator bool() const;
  constexpr explicit                operator GLFWwindow *() const;
  Window                           &operator=(const Window &rhs) = delete;
  constexpr Window                 &operator=(Window &&rhs);
  constexpr bool                    operator==(const Window &rhs) const;
  constexpr bool                    operator!=(const Window &rhs) const;
  EMBERS_ALWAYS_INLINE static Error get_last_error();
};
}  // namespace embers

// implementation

namespace embers {

constexpr Window::Window(Window &&window) : window_(window.window_) {
  window.window_ = nullptr;
  return;
}

inline Window::~Window() {
  if (window_ == nullptr) {
    return;
  }
  destroy();
  return;
}

constexpr Window::operator bool() const { return window_ != nullptr; }
constexpr Window::operator GLFWwindow *() const { return window_; }

constexpr Window &Window::operator=(Window &&rhs) {
  window_     = rhs.window_;
  rhs.window_ = nullptr;
  return *this;
}

constexpr bool Window::operator==(const Window &rhs) const {
  return window_ == rhs.window_;
}

constexpr bool Window::operator!=(const Window &rhs) const {
  return !operator==(rhs);
}

EMBERS_ALWAYS_INLINE Window::Error Window::get_last_error() {
  return last_error_;
}

}  // namespace embers
