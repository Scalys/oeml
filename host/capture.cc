#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <cstdint>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <linux/videodev2.h>

#include "capture.h"
#include "host.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

int g_capture_frames = 0;

const size_t frame_width  = 176;
const size_t frame_height = 144;
const char *dev_name;

static volatile int keep_running = 1;
static void exit_loop(int)
{
	keep_running = 0;
}

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define UINT8_SAT(x)							\
	(((x) < 0) ? 0 : ((x) > 255) ? 255 : (x))


struct buffer {
	void   *start;
	size_t  length;
};

static int fd = -1;
struct buffer *buffers;
static unsigned int n_buffers;
static int out_buf;
static int force_format = 1;
static int frame_count = 200;

static void errno_exit(const char *s)
{
	fprintf(stderr, "Error: %s error %d, %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

static int xioctl(int fh, int request, void *arg)
{
	int r;

	do {
		r = ioctl(fh, request, arg);
	} while (-1 == r && EINTR == errno);

	return r;
}

static void capture_frame_png(uint8_t *img_rgb)
{
	static size_t idx = 0;

	char path_png[255];
	sprintf(path_png, "oeml-%03lu.png", idx++);
	fprintf(stderr, "%s: ", path_png);

    stbi_write_png(path_png, frame_width, frame_height, 3, img_rgb, 0);
}

static void image_yuyv422_to_rgb(uint8_t *img_yuyv422, uint8_t *img_rgb)
{
	size_t size_rgb = frame_width * frame_height * 3;
	double r, g, b;
	double y, u, v;

	y = -16.0;
	u = -128.0;
	v = -128.0;
	for (size_t i_rgb = 0, i_yuv = 0; i_rgb < size_rgb;) {
		y = img_yuyv422[i_yuv++] - 16.0;
		u = img_yuyv422[i_yuv++] - 128.0;
		r = 1.164 * y             + 1.596 * v;
		g = 1.164 * y - 0.392 * u - 0.813 * v;
		b = 1.164 * y + 2.017 * u;
		img_rgb[i_rgb++] = UINT8_SAT(r);
		img_rgb[i_rgb++] = UINT8_SAT(g);
		img_rgb[i_rgb++] = UINT8_SAT(b);

		y = img_yuyv422[i_yuv++] - 16.0;
		v = img_yuyv422[i_yuv++] - 128.0;
		r = 1.164 * y             + 1.596 * v;
		g = 1.164 * y - 0.392 * u - 0.813 * v;
		b = 1.164 * y + 2.017 * u;
		img_rgb[i_rgb++] = UINT8_SAT(r);
		img_rgb[i_rgb++] = UINT8_SAT(g);
		img_rgb[i_rgb++] = UINT8_SAT(b);
	}
}

static void image_rgb_to_yuyv422(uint8_t *img_rgb, uint8_t *img_yuyv422)
{
	size_t size_yuyv422 = frame_width * frame_height * 2;
	double r, g, b;
	double y, u, v;

	for (size_t i = 0, j = 0; i < size_yuyv422; i += 4) {
		r = img_rgb[j++];
		g = img_rgb[j++];
		b = img_rgb[j++];
		y =  0.257 * r + 0.504 * g + 0.098 * b +  16;
		u = -0.148 * r - 0.291 * g + 0.439 * b + 128;
		img_yuyv422[i]   = y;
		img_yuyv422[i+1] = u;

		r = img_rgb[j++];
		g = img_rgb[j++];
		b = img_rgb[j++];
		y =  0.257 * r + 0.504 * g + 0.098 * b +  16;
		v =  0.439 * r - 0.368 * g - 0.071 * b + 128;
		img_yuyv422[i+2] = y;
		img_yuyv422[i+3] = v;
	}
}

static void process_frame(uint8_t *img_yuyv422, size_t bytesused)
{
	const size_t size_yuyv422 = frame_width * frame_height * 2;
	const size_t size_rgb     = frame_width * frame_height * 3;
	uint8_t img_rgb[size_rgb];
    int result;

	assert(size_rgb > bytesused);

	image_yuyv422_to_rgb(img_yuyv422, img_rgb);

	if (g_capture_frames)
		capture_frame_png(img_rgb);

    result = call_enclave(img_rgb);
    if (result != 0) {
        fprintf(stderr, "Error: failed to call enclave with result = %d\n",
				result);
        terminate_enclave();
		exit(1);
    }
}

static int read_frame(void)
{
	struct v4l2_buffer buf;

	CLEAR(buf);

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
		switch (errno) {
		case EAGAIN:
			return 0;
		case EIO:
			/* Could ignore EIO, see spec. */
			/* fall through */
		default:
			errno_exit("VIDIOC_DQBUF");
		}
	}
	assert(buf.index < n_buffers);

	process_frame((uint8_t*)buffers[buf.index].start, buf.bytesused);
	if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
		errno_exit("VIDIOC_QBUF");

	return 1;
}

static void mainloop(void)
{
	while (keep_running) {
		for (;;) {
			fd_set fds;
			struct timeval tv;
			int r;

			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			/* Timeout. */
			tv.tv_sec = 2;
			tv.tv_usec = 0;

			r = select(fd + 1, &fds, NULL, NULL, &tv);

			if (-1 == r) {
				if (EINTR == errno)
					continue;
				errno_exit("select");
			}

			if (0 == r) {
				fprintf(stderr, "Error: select timeout\n");
				exit(EXIT_FAILURE);
			}

			if (read_frame())
				break;
			/* EAGAIN - continue select loop. */
		}
	}
	printf("exitting capture loop...\n");
}

static void stop_capturing(void)
{
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
		errno_exit("VIDIOC_STREAMOFF");
}

static void start_capturing(void)
{
	unsigned int i;
	enum v4l2_buf_type type;

	for (i = 0; i < n_buffers; ++i) {
		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");
	}
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
		errno_exit("VIDIOC_STREAMON");
}

static void uninit_device(void)
{
	unsigned int i;

	for (i = 0; i < n_buffers; ++i)
		if (-1 == munmap(buffers[i].start, buffers[i].length))
			errno_exit("munmap");

	free(buffers);
}

static void init_mmap(void)
{
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "Error: %s does not support "
					"memory mappingn", dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2) {
		fprintf(stderr, "Error: insufficient buffer memory on %s\n",
				dev_name);
		exit(EXIT_FAILURE);
	}

	buffers = (buffer*)calloc(req.count, sizeof(*buffers));

	if (!buffers) {
		fprintf(stderr, "Error: out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		struct v4l2_buffer buf;

		CLEAR(buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;

		if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
			errno_exit("VIDIOC_QUERYBUF");

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start =
			mmap(NULL /* start anywhere */,
				 buf.length,
				 PROT_READ | PROT_WRITE /* required */,
				 MAP_SHARED /* recommended */,
				 fd, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start)
			errno_exit("mmap");
	}
}

static void init_device(void)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf(stderr, "Error: %s is no V4L2 device\n",
					dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "Error: %s is no video capture device\n",
				dev_name);
		exit(EXIT_FAILURE);
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		fprintf(stderr, "Error: %s does not support streaming i/o\n",
				dev_name);
		exit(EXIT_FAILURE);
	}

	/* Select video input, video standard and tune here. */

	CLEAR(cropcap);

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */

		if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
			switch (errno) {
			case EINVAL:
				/* Cropping not supported. */
				break;
			default:
				/* Errors ignored. */
				break;
			}
		}
	} else {
		/* Errors ignored. */
	}


	CLEAR(fmt);

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (force_format) {
		fmt.fmt.pix.width       = 176;
		fmt.fmt.pix.height      = 144;
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
		fmt.fmt.pix.field       = V4L2_FIELD_NONE;

		if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
			errno_exit("VIDIOC_S_FMT");

		/* Note VIDIOC_S_FMT may change width and height. */
	} else {
		/* Preserve original settings as set by v4l2-ctl for example */
		if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
			errno_exit("VIDIOC_G_FMT");
	}

	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	init_mmap();
}

static void close_device(void)
{
	if (-1 == close(fd))
		errno_exit("close");

	fd = -1;
}

static void open_device()
{
	struct stat st;

	if (-1 == stat(dev_name, &st)) {
		fprintf(stderr, "Error: cannot identify '%s': %d, %s\n",
				dev_name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (!S_ISCHR(st.st_mode)) {
		fprintf(stderr, "Error: %s is no devicen", dev_name);
		exit(EXIT_FAILURE);
	}

	fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);

	if (-1 == fd) {
		fprintf(stderr, "Error: cannot open '%s': %d, %s\n",
				dev_name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

int capture_loop(const char *cam_dev)
{
	printf("Starting a capture loop on %s. Press Ctrl-C to exit\n", cam_dev);

	dev_name = cam_dev;

	signal(SIGINT, &exit_loop);
	open_device();
	init_device();
	start_capturing();
	mainloop();
	stop_capturing();
	uninit_device();
	close_device();

	return 0;
}
