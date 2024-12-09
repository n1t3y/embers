#pragma once

#include <vector>

#include "../containers/allocator.hpp"
#include "../containers/debug_allocator.hpp"


namespace embers::vulkan {

#ifdef EMBERS_CONFIG_DEBUG

extern const char* const debug_extensions[1];

extern const char* const debug_layers[1];

#endif

extern const char* required_device_extensions[1];

#ifdef EMBERS_CONFIG_DEBUG
template <typename T>
using Allocator = containers::with<
    containers::DefaultAllocator,
    containers::DebugAllocatorTags::kVulkan>::DebugAllocator<T>;

#else
template <typename T>
using Allocator = embers::containers::DefaultAllocator<T>;
#endif

template <typename T>
using Vector = std::vector<T, Allocator<T>>;

}  // namespace embers::vulkan