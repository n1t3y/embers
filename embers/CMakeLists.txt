cmake_minimum_required(VERSION 3.21)

project(embers VERSION 0.0.1 LANGUAGES C CXX)

# set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/fmt)


set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

add_library(
	embers
	# SHARED
	STATIC
	src/test.cpp
	src/logger.cpp
	src/engine_config.cpp
	src/window.cpp
	src/platform.cpp
	src/containers/debug_allocator.cpp
)

target_include_directories(
	embers
	PRIVATE
	$<TARGET_PROPERTY:glfw,INTERFACE_INCLUDE_DIRECTORIES>

	PUBLIC
	$<INSTALL_INTERFACE:include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<TARGET_PROPERTY:fmt,INTERFACE_INCLUDE_DIRECTORIES>
)



target_compile_definitions(
	embers
	PRIVATE
	$<$<CONFIG:Debug>:EMBERS_CONFIG_DEBUG>
	# EMBERS_DLL_EXPORTS
	# EMBERS_DLL
)


target_link_libraries(
	embers
	PRIVATE
	fmt::fmt
	glfw
)

if(MSVC) 
	add_compile_options(/GR- /EHsc)
else()
	add_compile_options(-fno-rtti -fno-exceptions)
endif(MSVC) 