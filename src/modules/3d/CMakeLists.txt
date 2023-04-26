cmake_minimum_required(VERSION 3.15.2)

set(MODULE_NAME "3d")
message(STATUS "*** building ${MODULE_NAME} module ***")

add_subdirectory(3dgst)
add_subdirectory(common)
add_subdirectory(hpp)

message(STATUS "*** finished building ${MODULE_NAME} module ***")