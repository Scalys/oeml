// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <stdio.h>
#include <openenclave/host.h>

#include "common.h"

#include "oeml_u.h"


int g_enclave_debug = 0;
int g_enclave_simulate = 0;
static oe_enclave_t *g_enclave = NULL;

bool check_simulate_opt(int *argc, char *argv[])
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

bool check_debug_opt(int* argc, char *argv[])
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

int ocall_class_result(int logo, double certainty)
{
    switch (logo) {
    case LOGO_SCALYS : printf("scalys\n");    break ;
    case LOGO_ARM    : printf("arm\n");       break ;
    case LOGO_MSFT   : printf("microsoft\n"); break ;
    default : printf(" -\n");                 break ;
    }

    return 0;
}

int create_enclave(const char *ta_name)
{
    oe_result_t result = OE_OK;
    uint32_t flags = 0;

    if (g_enclave_debug)
        flags |= OE_ENCLAVE_FLAG_DEBUG;
    if (g_enclave_simulate)
        flags |= OE_ENCLAVE_FLAG_SIMULATE;

    result = oe_create_oeml_enclave(ta_name, OE_ENCLAVE_TYPE_AUTO, flags, NULL, 0, &g_enclave);
    if (result != OE_OK) {
        fprintf(stderr, "Error: oe_create_enclave(): result=%u (%s)\n", result,
                oe_result_str(result));
        return 1;
    }

    return 0;
}

int call_enclave(uint8_t *img)
{
    oe_result_t result = OE_OK;
    size_t w = FRAME_WIDTH, h = FRAME_HEIGHT, n = 3;
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
