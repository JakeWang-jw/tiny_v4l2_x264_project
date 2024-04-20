#include <errno.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <linux/videodev2.h>

#include "common.h"
#include "encode.h"
#include "capture.h"
#include "parse_config.h"

#ifndef H264_FRAME_NUM
#define H264_FRAME_NUM 100
#endif

static void param_init(VI_PARAM *vi_param) {
    // init vi_param
    memset(vi_param, 0x0, sizeof(*vi_param));
}

static void capture_encode_transfer_thread(void *arg) {
    int ret = 0;
    fd_set fds;
    struct timeval tv;
	struct v4l2_buffer v4l2_buf;
    VI_STRM *vi_strm = (VI_STRM *)arg;
    int cur_encode_frame_num = 0;

    FD_ZERO(&fds);
    FD_SET(vi_strm->v4l2_fd, &fds);

    /* Timeout. */
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    while (1) {
        if (H264_FRAME_NUM < cur_encode_frame_num) {
            break;
        }

		ret = select(vi_strm->v4l2_fd + 1, &fds, NULL, NULL, &tv);
		if (-1 == ret) {
			if (EINTR == errno) {
				continue;
            }
            PRINT_ERROR("select error!");
            goto out;
		}
		if (0 == ret) {
            PRINT_ERROR("select timeout!");
            goto out;
		}

        memset(&v4l2_buf, 0x0, sizeof(v4l2_buf));
        v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4l2_buf.memory = V4L2_MEMORY_MMAP;

        ret = ioctl(vi_strm->v4l2_fd, VIDIOC_DQBUF, &v4l2_buf);
        if (0 > ret) {
            PRINT_ERROR("ioctl VIDIOC_DQBUF failed! errno: %s", strerror(errno));
            goto out;
        }

        PRINT_DEBUG("start to encode, num: %d", cur_encode_frame_num);
        ret = x264_encode_one_frame((unsigned char *)vi_strm->v4l2_buf_ptr[v4l2_buf.index]);
        if (OK != ret) {
            PRINT_ERROR("x264_encode_one_frame failed!");
            goto out;
        }

        ret = ioctl(vi_strm->v4l2_fd, VIDIOC_QBUF, &v4l2_buf);
        if (0 > ret) {
            PRINT_ERROR("ioctl VIDIOC_QBUF failed! errno: %s", strerror(errno));
            goto out;
        }

        cur_encode_frame_num++;
    }

out:
    return;
}

int main(void) {
    int ret = 0;
    pthread_t thread;
    VI_PARAM *vi_param = NULL;

    init_log_level();

    vi_param = malloc(sizeof(*vi_param));
    if (!vi_param) {
        PRINT_ERROR("out of memory!\n");
        goto error;
    }

    param_init(vi_param);
    ret = parse_config(vi_param);
    if (OK != ret) {
        PRINT_ERROR("parse_config error!\n");
        goto error_free1;
    }

    ret = check_config(vi_param);
    if (OK != ret) {
        PRINT_ERROR("stream config is illegal!\n");
        goto error_free1;
    }

    /*
     * 刚使用sensor时，可调用该函数来探测sensor的capability
     * 调用该函数打印的log较长，平时不开启
     */
    ret = enum_video_device_capability();
    if (OK != ret) {
        PRINT_ERROR("enum_video_device_capability failed!");
        goto error_free1;
    }

    ret = capture_init(&vi_param->strm);
    if (OK != ret) {
        PRINT_ERROR("capture init failed!");
        goto error_free1;
    }

    ret = x264_encode_init(&vi_param->strm);
    if (ret < 0) {
        PRINT_ERROR("x264_encode_init failed!");
        goto error_free2;
    }

	if (0 != pthread_create(&thread, NULL, (void *)capture_encode_transfer_thread, (void *)&(vi_param->strm))) {
        PRINT_ERROR("thread create failed!");
        goto error_free3;
	}
	pthread_join(thread, NULL);

    x264_encode_deinit();
    capture_deinit(&vi_param->strm);
    free(vi_param);
    return OK;
error_free3:
    x264_encode_deinit();
error_free2:
    capture_deinit(&vi_param->strm);
error_free1:
    free(vi_param);
error:
    return ERROR;
}
