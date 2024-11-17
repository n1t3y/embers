#include <fmt/format.h>

#include <embers/logger.hpp>
#include <embers/test.hpp>
#include <iostream>
#include <vector>

#include "containers/allocator.hpp"
#include "containers/debug_allocator.hpp"
#include "ecs/entity.hpp"
#include "engine_config.hpp"
#include "error_code.hpp"
#include "platform.hpp"
#include "window.hpp"

using embers::containers::TestAllocator;

using namespace embers;

template <typename T>
using TestAlloc = containers::with<          //
    std::allocator,                          //
    containers::DebugAllocatorTags::kVulkan  //
    >::DebugAllocator<T>;                    //


int embers::test::main() {
  EMBERS_INFO(
      "Main called: {} ver. {} built @ " __DATE__ " " __TIME__,
      embers::config::engine.name,
      embers::config::engine.version
  );

  embers::config::Platform config;

  auto platform = embers::Platform(config);

  if (!(bool)platform) {
    auto err = embers::to_error_code(embers::Platform::get_last_error());
    EMBERS_DEBUG("{}", err);
    return 1;
  }

  EMBERS_DEBUG("Size: {}", embers::containers::debug_allocator_info[0].size);
  EMBERS_DEBUG(
      "Allocations: {}",
      embers::containers::debug_allocator_info[0].allocations
  );
  EMBERS_DEBUG(
      "Deallocations: {}",
      embers::containers::debug_allocator_info[0].deallocations
  );

  return 0;
}