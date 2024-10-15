cmake_minimum_required(VERSION 3.21)

project(embers_sandbox VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(
	embers_sandbox
	src/main.cpp
)

target_compile_definitions(
	embers_sandbox
	PRIVATE
	$<$<CONFIG:Debug>:EMBERS_CONFIG_DEBUG>
	# EMBERS_DLL
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


# add_custom_command(TARGET embers_sandbox POST_BUILD
# 	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:embers_sandbox> $<TARGET_FILE_DIR:embers_sandbox> 
# 	COMMAND_EXPAND_LISTS
# )