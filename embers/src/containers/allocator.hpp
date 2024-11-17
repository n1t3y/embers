#pragma once
#include <embers/logger.hpp>
#include <memory>

namespace embers::containers {

template <typename T>
using DefaultAllocator = std::allocator<T>;

template <typename T>
class TestAllocator {
 public:
  using value_type = T;
  using pointer    = T *;

  TestAllocator() noexcept = default;

  template <typename U>
  constexpr TestAllocator(const TestAllocator<U> &) noexcept {}

  constexpr T *allocate(std::size_t n) {
    T *p = (T *)std::malloc(n * sizeof(T));
    return p;
  }
  constexpr void deallocate(T *p, std::size_t n) noexcept { std::free(p); }

  template <typename U, typename... Args>
  constexpr void construct(U *p, Args &&...args) {
    new (p) U(std::forward<Args>(args)...);
  }

  template <typename U>
  constexpr void destroy(U *p) noexcept {
    p->~U();
  }
};

}  // namespace embers::containers