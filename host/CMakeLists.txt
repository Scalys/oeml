# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

add_custom_command(
  OUTPUT
    oeml_u.h
    oeml_u.c
    oeml_args.h
  DEPENDS ${CMAKE_SOURCE_DIR}/oeml.edl
  COMMAND openenclave::oeedger8r --untrusted ${CMAKE_SOURCE_DIR}/oeml.edl
  --search-path ${PKG_CONFIG_SYSROOT_DIR}/opt/oe/${OE_OPTEE_PLATFORM}/include
  --search-path ${PKG_CONFIG_SYSROOT_DIR}/opt/oe/${OE_OPTEE_PLATFORM}/include/openenclave/edl
  --search-path ${PKG_CONFIG_SYSROOT_DIR}/opt/oe/${OE_OPTEE_PLATFORM}/include/openenclave/edl/optee)

include_directories(
    ${CMAKE_SOURCE_DIR}/libs
    ${CMAKE_SOURCE_DIR}/common
)

add_executable(oeml
    host.cc
    capture.cc
    main.cc
    ${CMAKE_CURRENT_BINARY_DIR}/oeml_u.c)

target_include_directories(oeml PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(oeml openenclave::oehost)
