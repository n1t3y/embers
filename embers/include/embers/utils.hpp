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
