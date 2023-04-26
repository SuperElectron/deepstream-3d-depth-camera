set(MODULE_NAME "yaml.cmake")
message(STATUS ${MODULE_NAME} [start] ----------------------)

find_package(yaml-cpp CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}_LIB PRIVATE ${YAML_CPP_LIBRARIES})
target_include_directories(${PROJECT_NAME}_LIB PUBLIC
    ${YAML_CPP_INCLUDE_DIRS}
)

message(STATUS ${MODULE_NAME} [finish] ----------------------)



