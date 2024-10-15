#if defined(_WIN32)
#define EMBERS_DLL_IMPORT __declspec(dllimport)
#define EMBERS_DLL_EXPORT __declspec(dllexport)
#elif __GNUC__ >= 4
#define EMBERS_DLL_IMPORT __attribute__((visibility("default")))
#define EMBERS_DLL_EXPORT __attribute__((visibility("default")))
#else
#define EMBERS_DLL_IMPORT
#define EMBERS_DLL_EXPORT
#endif

#if !defined(EMBERS_DLL)
#define EMBERS_API
#elif defined(EMBERS_DLL_EXPORTS)
#define EMBERS_API EMBERS_DLL_EXPORT
#else
#define EMBERS_API EMBERS_DLL_IMPORT
#endif

#define EMBERS_INLINE inline

#define _EMBERS__STRINGIFY_INTERNAL(x) #x
#define EMBERS_STRINGIFY(x)            _EMBERS__STRINGIFY_INTERNAL(x)

// is used by logger, use short names (when possible)
#if defined(__FILE_NAME__)
#define EMBERS_FILENAME __FILE_NAME__
#elif defined(__FILE__)
#define EMBERS_FILENAME __FILE__
#else
#define EMBERS_FILENAME "unknown_file"
#endif

#if defined(__has_builtin) && !defined(__ibmxl__)
#if __has_builtin(__builtin_debugtrap)
#define EMBERS_DEBUGBREAK() __builtin_debugtrap()
#elif __has_builtin(__debugbreak)
#define EMBERS_DEBUGBREAK() __debugbreak()
#endif
#endif
#if !defined(EMBERS_DEBUGBREAK)
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define EMBERS_DEBUGBREAK() __debugbreak()
#endif
#endif