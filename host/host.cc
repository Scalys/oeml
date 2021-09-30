// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <stdio.h>
#include <openenclave/host.h>

#include "oeml_u.h"


static oe_enclave_t *g_enclave = NULL;

bool check_simulate_opt(int *argc, const char *argv[])
{
    for (int i = 0; i < *argc; i++) {
        if (strcmp(argv[i], "--simulate") == 0) {
            printf("Note: running in simulation mode\n");
            memmove(&argv[i], &argv[i + 1], (*argc - i) * sizeof(char*));
            (*argc)--;
            return true;
        }
    }

    return false;
}

bool check_debug_opt(int* argc, const char* argv[])
{
    for (int i = 0; i < *argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            printf("Note: running in debug mode\n");
            memmove(&argv[i], &argv[i + 1], (*argc - i) * sizeof(char*));
            (*argc)--;
            return true;
        }
    }

    return false;
}

int ocall_log(char *msg)
{
    if (printf("%s\n", msg) < 0)
        return 1;
    return 0;
}

int ocall_class_result(int cl, double certainty)
{
    switch (cl) {
        case 0 : printf(" -\n"); break ;
        case 1 : printf("arm\n"); break ;
        case 2 : printf("microsoft\n"); break ;
        case 3 : printf("scalys\n"); break ;
        default : printf(" UNKNOWN\n"); break ;
    }

    return 0;
}

int create_enclave(int argc, const char* argv[])
{
    oe_result_t result = OE_OK;
    uint32_t flags = 0;

    if (check_debug_opt(&argc, argv))
        flags |= OE_ENCLAVE_FLAG_DEBUG;
    if (check_simulate_opt(&argc, argv))
        flags |= OE_ENCLAVE_FLAG_SIMULATE;

    result = oe_create_oeml_enclave(argv[1], OE_ENCLAVE_TYPE_AUTO, flags, NULL, 0, &g_enclave);
    if (result != OE_OK) {
        fprintf(stderr, "Error: oe_create_enclave(): result=%u (%s)\n", result,
            oe_result_str(result));
        return 1;
    }

    return 0;
}

int call_enclave(uint8_t *img, size_t w, size_t h, size_t n)
{
    oe_result_t result = OE_OK;
    size_t size = w * h * n;
    int hostResult;

    result = ecall_nn(g_enclave, &hostResult, img, size, w, h, n);
    if (result != OE_OK) {
        fprintf(stderr, "Error: failed to run ecall: result=%u (%s)\n", result,
            oe_result_str(result));
        return result;
    } else if (hostResult != 0) {
        fprintf(stderr, "ecall_fibonacci failed: result=%u\n", hostResult);
        return result;
    }

    return result;
}

int terminate_enclave()
{
    oe_result_t result = OE_OK;

    if (g_enclave) {
        result = oe_terminate_enclave(g_enclave);
        if (result != OE_OK) {
            fprintf(stderr, "Error: calling into oe_terminate_enclave failed: result=%u (%s)\n",
                result,
                oe_result_str(result));
            return 1;
        }
    }

    return 0;
}
