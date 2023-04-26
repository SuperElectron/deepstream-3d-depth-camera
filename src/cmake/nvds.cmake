#
# To look at the shared object library, try this!
# nm -D /opt/nvidia/deepstream/deepstream-6.1/lib/libnvbufsurface.so

set(MODULE_NAME "nvds.cmake")
message(STATUS ${MODULE_NAME} [start] ----------------------)
SET(NVDS_INSTALL_DIR /opt/nvidia/deepstream/deepstream)

if(CUDA_VER)
    message(STATUS "[custom] Cuda Version: ${CUDA_VER} ")
else()
    set(CUDA_VER 11.6)
    message(STATUS "[default] Cuda Version: ${CUDA_VER} ")
endif()

include_directories(
    ${NVDS_INSTALL_DIR}/sources/includes
    /usr/local/cuda-${CUDA_VER}/include
    )

target_link_directories(${PROJECT_NAME}_LIB PUBLIC
    ${NVDS_INSTALL_DIR}/lib
    /usr/local/cuda-${CUDA_VER}/lib64
)

add_library(nvds SHARED IMPORTED)
set_target_properties(nvds PROPERTIES
    IMPORTED_LOCATION ${NVDS_INSTALL_DIR}/lib/libcudart.so
    IMPORTED_LOCATION ${NVDS_INSTALL_DIR}/lib/libnvdsgst_helper.so
    IMPORTED_LOCATION ${NVDS_INSTALL_DIR}/lib/libnvdsgst_3d_gst.so
    IMPORTED_LOCATION ${NVDS_INSTALL_DIR}/lib/libnvds_meta.so
    IMPORTED_LOCATION ${NVDS_INSTALL_DIR}/lib/libnvdsgst_meta.so

    )

target_link_libraries(${PROJECT_NAME}_LIB PUBLIC
    nvds
    cudart
    nvds_yml_parser
    cuda
    nvdsgst_3d_gst
    nvdsgst_meta
    nvdsgst_3d_gst
    nvds_meta
)

message(STATUS ${MODULE_NAME} [finish] ----------------------)