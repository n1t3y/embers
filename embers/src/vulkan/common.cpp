#include "common.hpp"

#include <vulkan/vulkan_core.h>

namespace embers::vulkan {

#ifdef EMBERS_CONFIG_DEBUG

const char* const debug_extensions[1] = {  //
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

const char* const debug_layers[1] = {  //
    "VK_LAYER_KHRONOS_validation"
};

#endif

const char* required_device_extensions[1] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

}  // namespace embers::vulkan