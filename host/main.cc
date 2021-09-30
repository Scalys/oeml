// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "host.h"

const char *g_path;


void usage()
{
    puts("Usage: oeml TA IMG");
    puts("Run ML demo on the supplied image file IMG.");
}

int parse_arguments(int argc, const char* argv[])
{
    if (argc < 3) {
        usage();
        return 1;
    }

    g_path = argv[2];
    if (!*g_path) {
        usage();
        return 1;
    }

    return 0;
}

uint8_t *load_image(size_t *width_out, size_t *height_out, size_t *n_out)
{
    int w, h, n;

    uint8_t *img = stbi_load(g_path, &w, &h, &n, 0);
    *width_out = w;
    *height_out = h;
    *n_out = n;

    return img;
}

int main(int argc, const char* argv[])
{
    int result;
    uint8_t *img;
    size_t w, h, n;

    if (parse_arguments(argc, argv))
        return 1;

    img = load_image(&w, &h, &n);
    if (!img) {
        fprintf(stderr, "Error: failed to read image file\n");
        return 1;
    }

    result = create_enclave(argc, argv);
    if (result != 0) {
        fprintf(stderr, "Error: failed to create enclave with result = %d\n", result);
        return result;
    }

    result = call_enclave(img, w, h, n);
    if (result != 0) {
        fprintf(stderr, "Error: failed to call enclave with result = %d\n", result);
        terminate_enclave();
        return result;
    }

    result = terminate_enclave();
    if (result != 0) {
        fprintf(stderr, "Error: failed to terminate enclave with result = %d\n", result);
    }

    free(img);

    return result;
}
