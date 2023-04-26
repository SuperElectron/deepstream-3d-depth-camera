#
# AVAILABLE VARIABLES
# UUID_INCLUDE_DIRS     # include dir
# UUID_LIBRARIES        # found libraries
#
set(MODULE_NAME "uuid.cmake")
message(STATUS ${MODULE_NAME} [start] ----------------------)

find_package(PkgConfig REQUIRED)

pkg_check_modules(UUID REQUIRED uuid)

target_include_directories(${PROJECT_NAME}_LIB PUBLIC ${UUID_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}_LIB PUBLIC ${UUID_LIBRARIES})

message(STATUS ${MODULE_NAME} [finish] ----------------------)