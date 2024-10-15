cmake_minimum_required(VERSION 3.21)

project(embers VERSION 0.0.1 LANGUAGES C CXX)

# set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/fmt)

add_library(
	embers
	# SHARED
	STATIC
	src/test.cpp
	src/logger.cpp
)

target_include_directories(
	embers
	PRIVATE

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
)
