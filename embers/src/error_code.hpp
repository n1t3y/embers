#pragma once

#include <GLFW/glfw3.h>

#include <embers/defines.hpp>

namespace embers {

using ErrorType = u32;

enum class Error : ErrorType {
  kOk                 = 0x00000000,
  kUnknown            = 0x00000001,
  kWindowInitGLFW     = 0x00000010,
  kWindowCreateWindow = 0x00000011,
};
}  // namespace embers
