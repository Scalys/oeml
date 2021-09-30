# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

add_custom_command(
  OUTPUT
    oeml_u.h
    oeml_u.c
    oeml_args.h
  DEPENDS ${CMAKE_SOURCE_DIR}/oeml.edl
  COMMAND openenclave::oeedger8r --untrusted ${CMAKE_SOURCE_DIR}/oeml.edl  --search-path /opt/openenclave-sdk/sdk/optee/ls-ls1012grapeboard/include --search-path /opt/openenclave-sdk/sdk/optee/ls-ls1012grapeboard/include/openenclave/edl/optee --search-path /opt/openenclave-sdk/sdk/optee/ls-ls1012grapeboard/include/openenclave/edl)

include_directories(
    ${CMAKE_SOURCE_DIR}/libs
)

add_executable(oeml
    host.cc
    main.cc
    ${CMAKE_CURRENT_BINARY_DIR}/oeml_u.c)

target_include_directories(oeml PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(oeml openenclave::oehost)
