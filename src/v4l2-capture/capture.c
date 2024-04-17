#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include "common.h"
#include "capture.h"

int enum_video_device_capability(void) {
    int fd = -1;
    struct v4l2_capability vcap;
    struct v4l2_fmtdesc fmtdesc;
    struct v4l2_frmsizeenum frmsize;
    struct v4l2_frmivalenum frmival;

    // 1. 打开设备
    fd = open("/dev/video0", O_RDWR);
    if (0 > fd) {
        PRINT_ERROR("Open %s error: %s", "dev/video0", strerror(errno));
        goto error;
    }

    // 2. 查询设备的属性/能力/功能
    memset(&vcap, 0x0, sizeof(vcap));
    ioctl(fd, VIDIOC_QUERYCAP, &vcap);

    // 判断是否是视频采集设备
    if (!(V4L2_CAP_VIDEO_CAPTURE & vcap.capabilities)) {
        PRINT_ERROR("VI_CAP: 0x%x", vcap.capabilities);
        PRINT_ERROR("Video device is not a capture device!");
        goto error_close;
    }

    if (!(V4L2_CAP_STREAMING & vcap.capabilities)) {
        PRINT_INFO("Video device doesn't support streaming I/O");
        goto error_close;
    }

    // 枚举出摄像头所支持的所有格式以及描述信息
    memset(&fmtdesc, 0x0, sizeof(fmtdesc));
    memset(&frmsize, 0x0, sizeof(frmsize));
    memset(&frmival, 0x0, sizeof(frmival));
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while (0 == ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc)) {
        PRINT_INFO("fmt: %s <0x%x>", fmtdesc.description, fmtdesc.pixelformat);
        // 根据像素格式枚举出摄像头所支持的所有视频帧大小
        frmsize.index = 0;
        frmsize.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        frmsize.pixel_format = fmtdesc.pixelformat;
        while (0 == ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize)) {
            PRINT_INFO("frame_size<%d*%d>", frmsize.discrete.width, frmsize.discrete.height);
            frmival.index = 0;
            frmival.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            frmival.pixel_format = frmsize.pixel_format;
            frmival.width = frmsize.discrete.width;
            frmival.height = frmsize.discrete.height;
            while (0 == ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival)) {
                PRINT_INFO("Frame interval<%ffps>",
                    frmival.discrete.denominator / frmival.discrete.numerator);
                frmival.index++;
            }
            frmsize.index++;
        }
        fmtdesc.index++;
    }

    close(fd);
    return OK;
error_close:
    close(fd);
error:
    return ERROR;
}

static int set_format(VI_STRM *vi_strm, int fd) {
    struct v4l2_format fmt;

    memset(&fmt, 0x0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = vi_strm->res.width;
    fmt.fmt.pix.height = vi_strm->res.height;
    fmt.fmt.pix.pixelformat = vi_strm->pix_fmt;
    if (0 > ioctl(fd, VIDIOC_S_FMT, &fmt)) {
        PRINT_ERROR("ioctl VIDIOC_S_FMT error!");
        goto error;
    }

    // 再检查一遍，确认参数生效了
    if (vi_strm->res.width != fmt.fmt.pix.width || vi_strm->res.height != fmt.fmt.pix.height) {
        PRINT_ERROR("Set resolution failed!");
        goto error;
    }
    if (vi_strm->pix_fmt != fmt.fmt.pix.pixelformat) {
        PRINT_ERROR("Set pixel format failed!");
        goto error;
    }

    return OK;
error:
    return ERROR;
}

static int set_framerate(VI_STRM *vi_strm, int fd) {
    struct v4l2_streamparm streamparam;

    memset(&streamparam, 0x0, sizeof(streamparam));
    streamparam.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (0 > ioctl(fd, VIDIOC_G_PARM, &streamparam)) {
        PRINT_ERROR("ioctl VIDIOC_G_PARAM failed!");
        goto error;
    }
    // 判断是否支持帧率设置
    if (V4L2_CAP_TIMEPERFRAME & streamparam.parm.capture.capability) {
        streamparam.parm.capture.timeperframe.numerator = 1;
        streamparam.parm.capture.timeperframe.denominator = vi_strm->framerate;
        if (0 > ioctl(fd, VIDIOC_S_PARM, &streamparam)) {
            PRINT_ERROR("ioctl VIDIOC_S_PARM failed!");
            goto error;
        }
    } else {
        PRINT_ERROR("Change framerate is not supportted!");
    }

    return OK;
error:
    return ERROR;
}

static int set_buffer(VI_STRM *vi_strm, int fd) {
    struct v4l2_requestbuffers reqbuf;
    struct v4l2_buffer buf;

    vi_strm->v4l2_buf_ptr = malloc(sizeof(void*) * vi_strm->v4l2_buf_cnt);
    if (!vi_strm->v4l2_buf_ptr) {
        PRINT_ERROR("malloc failed!");
        goto error;
    }
    memset(vi_strm->v4l2_buf_ptr, 0x0, sizeof(void*) * vi_strm->v4l2_buf_cnt);

    memset(&reqbuf, 0x0, sizeof(reqbuf));
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.count = vi_strm->v4l2_buf_cnt;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    if (0 > ioctl(fd, VIDIOC_REQBUFS, &reqbuf)) {
        PRINT_ERROR("ioctl VIDIOC_REQBUFS failed!");
        goto error;
    }

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    for (buf.index = 0; buf.index < vi_strm->v4l2_buf_cnt; buf.index++) {
        if (0 > ioctl(fd, VIDIOC_QUERYBUF, &buf)) {
            PRINT_ERROR("ioctl VIDIOC_QUERYBUF failed!");
            goto error;
        }
        vi_strm->v4l2_buf_ptr[buf.index] = mmap(NULL, buf.length,
            PROT_READ | PROT_WRITE, MAP_SHARED, fd,
            buf.m.offset);
        if (MAP_FAILED == vi_strm->v4l2_buf_ptr[buf.index]) {
            PRINT_ERROR("mmap failed!");
            goto error;
        }
    }

    // 入队操作
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    for (buf.index = 0; buf.index < vi_strm->v4l2_buf_cnt; buf.index++) {
        if (0 > ioctl(fd, VIDIOC_QBUF, &buf)) {
            PRINT_ERROR("ioctl VIDIOC_QBUF failed!");
            goto error;
        }
    }

    return OK;
error:
    return ERROR;
}

int capture_init(VI_STRM *vi_strm) {
    int fd = -1;
    int ret = 0;

    // 1. 打开设备
    fd = open("/dev/video0", O_RDWR);
    if (0 > fd) {
        PRINT_ERROR("Open %s error: %s", "dev/video0", strerror(errno));
        goto error;
    }

    // 2. 设置分辨率，像素格式
    ret = set_format(vi_strm, fd);
    if (OK != ret) {
        PRINT_ERROR("set_format failed!");
        goto error_free;
    }

    // 3. 设置帧率
    ret = set_framerate(vi_strm, fd);
    if (OK != ret) {
        PRINT_ERROR("set_framerate failed!");
        goto error_free;
    }

    // 4. 设置缓存
    ret = set_buffer(vi_strm, fd);
    if (OK != ret) {
        PRINT_ERROR("set_buffer failed!");
        goto error_free;
    }

    // 开启视频采集
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (0 > ioctl(fd, VIDIOC_STREAMON, &type)) {
        PRINT_ERROR("ioctl VIDIOC_STREAMON failed!");
        goto error_free;
    }

    return OK;
error_free:
    close(fd);
error:
    return ERROR;
}




