#include <fmt/format.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <embers/logger.hpp>
#include <embers/test.hpp>
#include <unordered_set>
#include <vector>

#include "containers/debug_allocator.hpp"
#include "ecs/entity.hpp"
#include "engine_config.hpp"
#include "error_code.hpp"
#include "platform.hpp"
#include "vulkan/device.hpp"
#include "vulkan/instance.hpp"
#include "vulkan/surface.hpp"
#include "window.hpp"

using namespace embers;

int embers::test::main() {
  EMBERS_INFO(
      "Main called: {} ver. {} built @ " __DATE__ " " __TIME__,
      embers::config::engine.name,
      embers::config::engine.version
  );

  embers::config::Platform config;

  auto platform = embers::Platform(config);

  if (!(bool)platform) {
    auto err = embers::Platform::get_last_error();
    EMBERS_DEBUG("{}", err);
    return 1;
  }

#ifdef EMBERS_CONFIG_DEBUG
  EMBERS_DEBUG("Vulkan: {}", embers::containers::debug_allocator_info[0]);
  EMBERS_DEBUG("Logger: {}", embers::containers::debug_allocator_info[1]);

#endif

  return 0;
}