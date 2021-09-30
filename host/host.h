// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#define ENCLAVE_MESSAGE_SIZE 512

int create_enclave(int argc, const char* argv[]);
int terminate_enclave();
int call_enclave(uint8_t *img, size_t w, size_t h, size_t n);
