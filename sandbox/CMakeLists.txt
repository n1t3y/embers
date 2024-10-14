cmake_minimum_required(VERSION 3.10)

project(embers_sandbox VERSION 0.0.1 LANGUAGES CXX)

add_executable(
	embers_sandbox
	src/main.cpp
)

target_compile_definitions(
	embers_sandbox
	PRIVATE
	$<$<CONFIG:Debug>:EMBERS_DEBUG>
	EMBERS_DLL
)

target_include_directories(
	embers_sandbox
	PUBLIC
	embers
)

target_link_libraries(
	embers_sandbox
	PRIVATE
	embers
)
