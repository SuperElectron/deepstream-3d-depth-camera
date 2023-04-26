################################################
### Build instructions
# mkdir -p /inference/build && cd /inference/build && rm -rf *
################################################
### Build
# cmake -B build -S .
# cmake --build build -j

################################################

################################################
# Main CMake settings for the project
################################################
cmake_minimum_required(VERSION 3.15.2)
enable_language(C CXX)

# set PROJECT_LIB descriptors
set(PROJECT_VERSION 1.2)
set(PROJECT_DESCRIPTION "linux-vms for edge detection")

# set the C++ standard
set(CMAKE_BUILD_TYPE Debug) # Release or Debug
set(CMAKE_CXX_EXTENSIONS OFF) # OFF -> -std=c++14, ON -> -std=gnu++14
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

################################################
# set directories
################################################

string(TIMESTAMP TODAY "%Y-%m-%dT%H-%M-%SZ")
set(CMAKE_CONFIGURATION_TYPES UNIX)
set(CMAKE_MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)
set(CMAKE_LIBRARY_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(PROJECT_MODULES_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/modules)

message("SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR})
message("CMAKE_MODULES_DIR: " ${CMAKE_MODULES_DIR})
message("CMAKE_BINARY_DIR: " ${CMAKE_BINARY_DIR})
message("CMAKE_RUNTIME_OUTPUT_DIRECTORY: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
message("PROJECT_MODULES_DIRECTORY: " ${PROJECT_MODULES_DIRECTORY})
message("CMAKE_LIBRARY_PATH: " ${CMAKE_LIBRARY_PATH})
message("CMAKE_HOST_SYSTEM_PROCESSOR: " ${CMAKE_HOST_SYSTEM_PROCESSOR})

if( ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86_64" )
    set(BUILD_ARCHITECTURE "x64" )
    set(CUDA_VER 11.8)
else(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "aarch64" )
    set(BUILD_ARCHITECTURE "arm64" )
    set(CUDA_VER 11.4)
endif()

################################################
# Add vcpkg build paths
################################################

set("CMAKE_TOOLCHAIN_FILE /start/vcpkg/scripts/buildsystems/vcpkg.cmake")
include(/start/vcpkg/scripts/buildsystems/vcpkg.cmake)
# Azure setup: must have vcpkg.json at project root and call this before `project(name VERSION 0.0.0)`
list(APPEND CMAKE_MODULE_PATH "${CMAKE_MODULES_DIR}")

################################################
# Configure project executable and library (static or shared)
################################################

project(depthCam DESCRIPTION ${PROJECT_DESCRIPTION} VERSION ${PROJECT_VERSION})
add_library(${PROJECT_NAME}_LIB STATIC)
# https://cliutils.gitlab.io/modern-cmake/chapters/features/small.html
set_target_properties(${PROJECT_NAME}_LIB PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(${PROJECT_NAME}_LIB PROPERTIES LINKER_LANGUAGE CXX)

set(SRC_FILE main.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILE} ${HEADER_FILE})

# create the library and add all headers to it
# include this module's directory so other modules can find it
target_include_directories(${PROJECT_NAME}_LIB PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

file(GLOB APPLICATION_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

# create the library
target_sources(${PROJECT_NAME}_LIB PUBLIC ${APPLICATION_HEADERS})

################################################
# Include libraries base cmakes for all modules
################################################

include(cmake/gstreamer.cmake)
include(cmake/nvds.cmake)
include(cmake/uuid.cmake)
include(cmake/yaml.cmake)

################################################
## Build Application
################################################

set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_CXX_STANDARD 20)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_compile_features(${PROJECT_NAME}_LIB PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_LIB ${CMAKE_DL_LIBS})

################################################
# Set additional library properties
################################################

set_target_properties(${PROJECT_NAME}_LIB PROPERTIES
        VERSION ${PROJECT_VERSION}
        LINK_FLAGS "-Wl,--whole-archive"
)
target_include_directories(${PROJECT_NAME}_LIB PUBLIC ${PROJECT_MODULES_DIRECTORY})
target_include_directories(${PROJECT_NAME}_LIB PUBLIC ${PROJECT_MODULES_DIRECTORY}/3d)
include(GNUInstallDirs)
message(CMAKE_INSTALL_LIBDIR: ${CMAKE_INSTALL_LIBDIR})
message(CMAKE_INSTALL_INCLUDEDIR: ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS ${PROJECT_NAME}_LIB
        LIBRARY DESTINATION ${CMAKE_BINARY_DIR}
        DESTINATION ${CMAKE_BINARY_DIR}
)
