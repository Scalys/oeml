// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>

#include "stb/stb_image.h"

#include "common.h"
#include "host.h"
#include "capture.h"


const char default_cam_dev[] = "/dev/video0";
int g_help = 0;


static void usage()
{
    printf("Usage: oeml TA [OPTION]... [IMG]\n\
Execute OpenEnclave confidential machine learning sample.\n\
\n\
If IMG is provided the demo will classify the supplied image logo file\n\
between Scalys, Arm and Microsoft and exit.\n\
Note: expected image is %dx%d PNG file.\n\
\n\
If image file is not specified capture and calssify images from a web camera\n\
stream.\n\
\n\
Options:\n\
  -h, --help              show help\n\
  -d, --device=DEVICE     set camera device. Default is %s\n\
  -c, --capture           capture processed frames and store them in currend directory\n\
", FRAME_WIDTH, FRAME_HEIGHT, default_cam_dev);
}

static int init(const char *ta_name)
{
    int ret = create_enclave(ta_name);
    if (ret != 0) {
        fprintf(stderr, "Error: failed to create enclave with result = %d\n",
                ret);
    }

    return ret;
}

static int uninit()
{
    int ret = terminate_enclave();
    if (ret != 0) {
        fprintf(stderr, "Error: failed to terminate enclave with result = %d\n",
                ret);
    }

    return ret;
}

static uint8_t *load_image(const char *path)
{
    int w, h, n;

    uint8_t *img = stbi_load(path, &w, &h, &n, 0);
    if (!img) {
        fprintf(stderr, "Error: .");
        return 0;
    }
    if (w != FRAME_WIDTH || h != FRAME_HEIGHT || n != 3) {
        fprintf(stderr, "Error: invalida image file");

    }

    return img;
}

static int process_image(const char *path)
{
    int ret = 0;
    uint8_t *img;

    img = load_image(path);
    if (!img) {
        fprintf(stderr, "Error: failed to read image file\n");
        return 1;
    }

    ret = call_enclave(img);
    if (ret != 0) {
        fprintf(stderr, "Error: failed to call enclave with result = %d\n", ret);
        free(img);
        terminate_enclave();
        return ret;
    }

    free(img);
    return 0;
}

int main(int argc, char *argv[])
{
    int ret;
    uint8_t *img;
    const char *cam_dev = default_cam_dev;
    const char *img_path = 0;
    const char *ta_name = 0;
    int c;

    while (1) {
        static struct option long_opts[] = {
            {"debug",    no_argument, &g_enclave_debug,    1},
            {"simulate", no_argument, &g_enclave_simulate, 1},
            {"capture",  no_argument, &g_capture_frames,   1},
            {"help",     no_argument, &g_help,             1},
            {"device",   required_argument, 0, 'd'},
            {0, 0, 0, 0}
        };

        int opt_index = 0;
        c = getopt_long(argc, argv, "hcd:", long_opts, &opt_index);
        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            if (long_opts[opt_index].flag != 0)
                break;
            printf("option %s", long_opts[opt_index].name);
            if (optarg)
                printf (" with arg %s", optarg);
            printf ("\n");
            break;
        case 'h':
            usage();
            exit(0);
        case 'c':
            g_capture_frames = true;
            break;
        case 'd':
            cam_dev = optarg;
            break;
        case '?':
            break;
        default:
            abort();
        }
    }

    if (g_help) {
        usage();
        exit(0);
    }

    if (optind >= argc) {
        usage();
        abort();
    }
    ta_name = argv[optind];

    if ((optind + 1) < argc) {
        img_path = argv[optind + 1];
    }

    ret = init(ta_name);
    if (ret)
        return ret;

    if (img_path) {
        process_image(img_path);
    } else {
        capture_loop(cam_dev);
    }

    uninit();

    return ret;
}
