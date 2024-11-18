#pragma once

#include <GLFW/glfw3.h>
#include <fmt/base.h>

#include <embers/defines.hpp>

namespace embers {

using ErrorType = u32;

enum class Error : ErrorType {
  kOk                                   = 0x00000000,
  kUnknown                              = 0x00000001,
  kWindowInitGLFW                       = 0x00000010,
  kWindowCreateWindow                   = 0x00000011,
  kVulkanInitVulkan                     = 0x00000020,
  kVulkanEnumerateExtensions            = 0x00000021,
  kVulkanGLFWGetRequiredExtensions      = 0x00000022,
  kVulkanRequiredExtensionsArentPresent = 0x00000023,
};

template <typename E>
constexpr Error to_error_code(E e) noexcept {
  static_assert("Error code conversion is not defined");
  return Error::kUnknown;
}

}  // namespace embers

template <>
class fmt::formatter<embers::Error> {
  using Error = embers::Error;

 public:
  constexpr static const char *get_error_description(Error err);
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename Context>
  constexpr auto format(Error const &error_code, Context &ctx) const {
    return format_to(
        ctx.out(),
        "<Error: {}>",
        get_error_description(error_code)
    );
  }
};

constexpr const char *fmt::formatter<embers::Error>::get_error_description(
    embers::Error err
) {
  using Error = embers::Error;

  switch (err) {
    case Error::kOk:
      return "Ok";
    case Error::kUnknown:
      return "Unknown";
    case Error::kWindowInitGLFW:
      return "Unable to init GLFW";
    case Error::kWindowCreateWindow:
      return "Unable to create a window";
    default:
      break;
  }
  return "Error without description";
}