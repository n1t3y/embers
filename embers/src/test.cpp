#include <fmt/format.h>

#include <embers/logger.hpp>
#include <embers/test.hpp>
#include <iostream>

#include "engine_config.hpp"

int embers::test::main() {
  EMBERS_INFO(
      "Main called: {} ver. {} built @ " __DATE__ " " __TIME__, embers::config::engine_config.name,
      embers::config::engine_config.version
  );

  return 0;
}