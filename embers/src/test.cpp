#include <fmt/format.h>

#include <embers/logger.hpp>
#include <embers/test.hpp>
#include <iostream>
#include <vector>

#include "containers/allocator.hpp"
#include "ecs/entity.hpp"
#include "engine_config.hpp"
#include "error_code.hpp"
#include "platform.hpp"
#include "window.hpp"

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

  // EMBERS_INFO("Window {}", fmt::ptr((GLFWwindow *)window));

  // auto platform = Platform::create(config).unwrap();

  // embers::ecs::Entity entity;

  // entity.index_   = 0xff;
  // entity.counter_ = 0xcc;

  // EMBERS_INFO("Entity: {:#x}", entity.index_and_counter_);

  // std::vector<int, embers::containers::TestAllocator<int>> test_vector =
  //     {1, 2, 3, 5, 6, 7};

  // for (auto &&i : test_vector) {
  //   EMBERS_INFO("- {}", i);
  // }

  // test_vector.push_back(1);

  return 0;
}