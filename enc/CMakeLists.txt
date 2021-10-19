# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

include(../cmake/add-enclave.cmake)

include_directories(
    ${CMAKE_SOURCE_DIR}/libs
    ${CMAKE_SOURCE_DIR}/libs/gemmlowp
    ${CMAKE_SOURCE_DIR}/libs/ruy
    ${CMAKE_SOURCE_DIR}/enc/inc
    ${CMAKE_SOURCE_DIR}/common
    ${PKG_CONFIG_SYSROOT_DIR}/opt/tflite-micro/third_party/flatbuffers/include
    ${PKG_CONFIG_SYSROOT_DIR}/opt/tflite-micro/third_party/gemmlowp
    ${PKG_CONFIG_SYSROOT_DIR}/opt/tflite-micro/third_party/kissfft
    ${PKG_CONFIG_SYSROOT_DIR}/opt/tflite-micro/third_party/pigweed
    ${PKG_CONFIG_SYSROOT_DIR}/opt/tflite-micro/third_party/ruy
    ${PKG_CONFIG_SYSROOT_DIR}/opt/tflite-micro
#    ${TENSORFLOW_LITE_INC}
)

add_custom_command(
  OUTPUT
    oeml_t.h
    oeml_t.c
    oeml_args.h
  DEPENDS ${CMAKE_SOURCE_DIR}/oeml.edl
  COMMAND openenclave::oeedger8r --trusted ${CMAKE_SOURCE_DIR}/oeml.edl
  --search-path ${PKG_CONFIG_SYSROOT_DIR}/opt/oe/${OE_OPTEE_PLATFORM}/include
  --search-path ${PKG_CONFIG_SYSROOT_DIR}/opt/oe/${OE_OPTEE_PLATFORM}/include/openenclave/edl
  --search-path ${PKG_CONFIG_SYSROOT_DIR}/opt/oe/${OE_OPTEE_PLATFORM}/include/openenclave/edl/optee)

add_enclave(
  TARGET oeml_enclave
  UUID 1f574668-6c89-41b5-b313-4b2d85d63c9d
    SOURCES
    enc.cc
    oeml_model_data.cc
    ${CMAKE_CURRENT_BINARY_DIR}/oeml_t.c)

target_compile_options(oeml_enclave
  PRIVATE "-DTF_LITE_STATIC_MEMORY"
)

target_link_libraries(oeml_enclave
  ${PKG_CONFIG_SYSROOT_DIR}/opt/tflite-micro/tensorflow/lite/micro/tools/make/gen/linux_aarch64_debug/lib/libtensorflow-microlite.a
)

target_link_libraries(oeml_enclave openenclave::oeenclave openenclave::oecryptombedtls)
