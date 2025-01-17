#pragma once

#include <fmt/base.h>

#include "defines.hpp"

namespace embers::config {

struct Version {
  u32 patch : 12;
  u32 minor : 10;
  u32 major : 10;
};

static_assert(sizeof(Version) == 4, "Size of Version must be 32 bits");

struct Platform {
  template <typename T>
  struct Array {
    T  *array = nullptr;
    u32 size  = 0;
  };

  struct VulkanExtensionsAndLayers {
    struct {
      Array<const char *> optional = {};
      Array<const char *> required = {};
    } extensions;

    struct {
      Array<const char *> optional = {};
      Array<const char *> required = {};
    } layers;
  };

  const char *application_name = "Embers Application";
  struct {
    u32 width  = 1280;
    u32 height = 800;
  } resolution;

  Version version = {1, 0, 0};

  VulkanExtensionsAndLayers instance;
  VulkanExtensionsAndLayers device;
};

}  // namespace embers::config

template <>
class fmt::formatter<embers::config::Version> {
  using Version = embers::config::Version;

 public:
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename Context>
  constexpr auto format(Version const &version, Context &ctx) const {
    return format_to(
        ctx.out(),
        "{}.{}.{}",
        version.major,
        version.minor,
        version.patch
    );
  }
};