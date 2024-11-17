#include "debug_allocator.hpp"

namespace embers::containers {

static DebugAllocatorInfo debug_allocator_info
    [(int)DebugAllocatorTags::kMax - (int)DebugAllocatorTags::kMin + 1]{};

}  // namespace embers::containers