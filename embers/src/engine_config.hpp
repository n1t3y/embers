#pragma once

#include <embers/config.hpp>

namespace embers::config {
struct EngineConfig {
  const char *name;
  Version     version;
};

extern EngineConfig engine_config;

}  // namespace embers::config
