#include "debug_allocator.hpp"

#ifdef EMBERS_CONFIG_DEBUG

namespace embers::containers {

DebugAllocatorInfo debug_allocator_info
    [(int)DebugAllocatorTags::kMax - (int)DebugAllocatorTags::kMin + 1] = {};

}  // namespace embers::containers

#endif