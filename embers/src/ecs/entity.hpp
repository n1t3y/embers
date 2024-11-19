#pragma once

#include <embers/defines.hpp>

namespace embers::ecs {

class Entity {
 public:  // todo
  union {
    struct {
      u16 index_;
      u16 counter_;
    };
    u32 index_and_counter_;
  };

 public:
  constexpr Entity(u16 index, u16 counter) : index_(index), counter_(counter) {}
  constexpr Entity() : Entity(0, 0) {}

  constexpr bool valid() { return index_and_counter_ != 0; }

  constexpr bool operator==(const Entity &rhs) {
    return index_and_counter_ == rhs.index_and_counter_;
  }
  constexpr bool operator!=(const Entity &rhs) { return !(*this == rhs); }
};

}  // namespace embers::ecs