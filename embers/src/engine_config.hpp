#pragma once

#include <embers/config.hpp>

namespace embers::config {

struct Engine {
  const char *name;
  Version     version;
};

extern Engine engine;

}  // namespace embers::config