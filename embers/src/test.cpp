#include <fmt/format.h>

#include <embers/logger.hpp>
#include <embers/test.hpp>
#include <iostream>

int embers::test::main() {
  using embers::logger::Level;
  using embers::logger::log;

  EMBERS_DEBUG("Debug, {}", 42);
  EMBERS_INFO("Info, {}", 42);
  EMBERS_WARN("Warn, {}", 42);
  EMBERS_ERROR("Error, {}", 42);
  EMBERS_FATAL("Fatal, {}", 42);

  return 0;
}