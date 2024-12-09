#include "surface.hpp"

// clang-format off
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
// clang-format on

namespace embers::vulkan {

Error Surface::last_error_ = Error::kUnknown;

void Surface::destroy() {
  vkDestroySurfaceKHR(instance_, surface_, nullptr);
  return;
}

Surface::Surface(const Instance& instance, const Window& window) {
  instance_ = (VkInstance)instance;

  VkResult err = glfwCreateWindowSurface(  //
      instance_,
      (GLFWwindow*)window,
      NULL,
      &surface_
  );

  if (err != VK_SUCCESS) {
    last_error_ = Error::kVulkanCreateSurface;
    surface_    = nullptr;
    EMBERS_FATAL("Unable to create surface: {}", (int)err);
    return;
  }
}

}  // namespace embers::vulkan