cmake_minimum_required(VERSION 3.10)

project(embers VERSION 0.0.1 LANGUAGES C CXX)

# set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(CMAKE_CXX_STANDARD 17)

add_library(
	embers
	SHARED
	src/test.cpp
)

target_include_directories(
	embers
	PRIVATE

	PUBLIC
	$<INSTALL_INTERFACE:include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_definitions(
	embers
	PRIVATE
	$<$<CONFIG:Debug>:EMBERS_DEBUG>
	EMBERS_DLL_EXPORTS
	EMBERS_DLL
)

target_link_libraries(
	embers
	PRIVATE
)
