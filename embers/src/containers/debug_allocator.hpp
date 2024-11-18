#pragma once

#include <embers/defines.hpp>
#include <memory>

namespace embers::containers {

enum class DebugAllocatorTags {
  kMin    = 0,
  kVulkan = 0,
  kMax    = 0,
};

struct DebugAllocatorInfo {
  size_t size          = 0;
  size_t max_size      = 0;
  u32    allocations   = 0;
  u32    deallocations = 0;
};

extern DebugAllocatorInfo debug_allocator_info
    [(int)DebugAllocatorTags::kMax - (int)DebugAllocatorTags::kMin + 1];

template <template <typename> typename InnerAllocator, DebugAllocatorTags tag>
class with {
 public:
  template <typename T>
  class DebugAllocator {
    using allocator_traits = std::allocator_traits<InnerAllocator<T>>;

   public:
    InnerAllocator<T> inner;

    using value_type = T;

    DebugAllocator() noexcept = default;

    template <typename U>
    constexpr DebugAllocator(const DebugAllocator<U> &other) noexcept {
      inner = other.inner;
      return;
    }

    constexpr T *allocate(std::size_t n) {
      T *p = allocator_traits::allocate(inner, n);
      debug_allocator_info[(int)tag].allocations++;
      debug_allocator_info[(int)tag].size += sizeof(T) * n;
      debug_allocator_info[(int)tag].max_size = std::max(
          debug_allocator_info[(int)tag].max_size,
          debug_allocator_info[(int)tag].size
      );
      return p;
    }
    constexpr void deallocate(T *p, std::size_t n) noexcept {
      allocator_traits::deallocate(inner, p, n);
      debug_allocator_info[(int)tag].deallocations++;
      debug_allocator_info[(int)tag].size -= sizeof(T) * n;
      return;
    }

    template <typename U, typename... Args>
    constexpr void construct(U *p, Args &&...args) {
      allocator_traits::construct(inner, p, args...);
      return;
    }

    template <typename U>
    constexpr void destroy(U *p) noexcept {
      allocator_traits::destroy(inner, p);
      return;
    }
  };
};

}  // namespace embers::containers