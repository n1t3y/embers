#pragma once

namespace embers::vulkan {

#ifdef EMBERS_CONFIG_DEBUG

extern const char* const debug_extensions[1];

extern const char* const debug_layers[1];

#endif

extern const char* required_device_extensions[1];

}  // namespace embers::vulkan