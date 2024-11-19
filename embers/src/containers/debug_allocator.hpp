#pragma once

#ifdef EMBERS_CONFIG_DEBUG

#include <fmt/base.h>

#include <embers/defines.hpp>
#include <memory>

namespace embers::containers {

enum class DebugAllocatorTags {
  kMin    = 0,
  kVulkan = 0,
  kLogger = 1,
  kMax    = 1,
};

struct DebugAllocatorInfo {
  size_t size            = 0;
  size_t max_size        = 0;
  size_t max_size_single = 0;
  float  avg_mean        = 0;
  u32    allocations     = 0;
  u32    deallocations   = 0;
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
      debug_allocator_info[(int)tag].size += sizeof(T) * n;
      debug_allocator_info[(int)tag].max_size = std::max(
          debug_allocator_info[(int)tag].max_size,
          debug_allocator_info[(int)tag].size
      );
      debug_allocator_info[(int)tag].max_size_single = std::max(
          debug_allocator_info[(int)tag].max_size_single,
          sizeof(T) * n
      );
      debug_allocator_info[(int)tag].avg_mean =
          (debug_allocator_info[(int)tag].avg_mean *
               debug_allocator_info[(int)tag].allocations +
           sizeof(T) * n) /
          (debug_allocator_info[(int)tag].allocations + 1);
      debug_allocator_info[(int)tag].allocations++;

      return p;
    }
    constexpr void deallocate(T *p, std::size_t n) noexcept {
      allocator_traits::deallocate(inner, p, n);
      debug_allocator_info[(int)tag].deallocations++;
      debug_allocator_info[(int)tag].size -= sizeof(T) * n;
      return;
    }
  };
};

}  // namespace embers::containers

template <>
class fmt::formatter<embers::containers::DebugAllocatorInfo> {
  using DebugAllocatorInfo = embers::containers::DebugAllocatorInfo;

 public:
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename Context>
  constexpr auto format(DebugAllocatorInfo const &info, Context &ctx) const {
    return format_to(
        ctx.out(),
        "<Now: {} bytes; "
        "Max total/single: {}/{}; "
        "Allocs/Dealllocs: {}/{}; "
        "Average allocation: {:.2f}>",
        info.size,
        info.max_size,
        info.max_size_single,
        info.allocations,
        info.deallocations,
        info.avg_mean
    );
  }
};

#endif