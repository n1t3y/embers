#include <embers/logger.hpp>
#include <embers/test.hpp>
#include <iostream>

int main() {
  EMBERS_DEBUG("Starting Sandbox");
  int var = embers::test::main();
  EMBERS_DEBUG("Main existed with {}", var);
  return 0;
}