#pragma once

#include <GLFW/glfw3.h>

#include <embers/defines/types.hpp>

namespace embers {
using ErrorCodeType = u32;

enum class ErrorCode : ErrorCodeType {
  kOk                 = 0x00000000,
  kUnknown            = 0x00000001,
  kWindowInitGLFW     = 0x00000010,
  kWindowCreateWindow = 0x00000011,
};
}  // namespace embers
