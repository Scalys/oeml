# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-poky-linux-gcc)
set(CMAKE_CXX_COMPILER aarch64-poky-linux-g++)
set(CMAKE_C_FLAGS "--sysroot=/home/haff/poky_sdk/tmp/sysroots/trustbox")
set(CMAKE_CXX_FLAGS "--sysroot=/home/haff/poky_sdk/tmp/sysroots/trustbox")
set(CMAKE_C_COMPILER_ID GNU)
SET(CMAKE_FIND_ROOT_PATH "/home/haff/poky_sdk/tmp/sysroots/trustbox")

