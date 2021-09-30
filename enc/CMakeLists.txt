# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

link_directories("/home/haff/poky_sdk/tmp/sysroots/trustbox/usr/lib/aarch64-poky-linux/9.3.0")
set(TENSORFLOW_LITE_DIR "/home/haff/work/trustbox/openenclave-sdk-builder/packages/oeml/libs/tensorflow")

include(../cmake/add-enclave.cmake)

include_directories(
    ${CMAKE_SOURCE_DIR}/libs
    ${CMAKE_SOURCE_DIR}/libs/gemmlowp
    ${CMAKE_SOURCE_DIR}/libs/ruy
    ${CMAKE_SOURCE_DIR}/enc/inc
    ${TENSORFLOW_LITE_INC}
)

add_custom_command(
  OUTPUT
    oeml_t.h
    oeml_t.c
    oeml_args.h
  DEPENDS ${CMAKE_SOURCE_DIR}/oeml.edl
  COMMAND openenclave::oeedger8r --trusted ${CMAKE_SOURCE_DIR}/oeml.edl --search-path /opt/openenclave-sdk/sdk/optee/ls-ls1012grapeboard/include --search-path /opt/openenclave-sdk/sdk/optee/ls-ls1012grapeboard/include/openenclave/edl --search-path /opt/openenclave-sdk/sdk/optee/ls-ls1012grapeboard/include/openenclave/edl/optee)

#add_library(Tensorflow_lite_micro STATIC ${TENSORFLOW_LITE_SRC})
#set_target_properties(Tensorflow_lite_micro PROPERTIES LINKER_LANGUAGE CXX)

add_enclave(
  TARGET oeml_enclave
  UUID 5d286b7e-ff68-4b4b-b7b8-05f55dbfd0c7
    SOURCES
    enc.cc
    oeml_model_data.cc
    ${CMAKE_CURRENT_BINARY_DIR}/oeml_t.c)

target_compile_options(oeml_enclave
  PRIVATE "-DTF_LITE_STATIC_MEMORY"
)

target_link_libraries(oeml_enclave
	/home/haff/work/trustbox/openenclave-sdk-builder/tflite-micro/tensorflow/lite/micro/tools/make/gen/linux_aarch64_debug/lib/libtensorflow-microlite.a
)

