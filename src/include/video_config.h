#ifndef VIDEO_CONFIG_H_
#define VIDEO_CONFIG_H_

#include "common.h"

#ifdef USE_OV5640
#define SENSOR_NAME "ov5640"
#elif defined(USE_SC2336)
#define SENSOR_NAME "sc2336"
#endif

#define VIDEO_CONFIG_PATH "/home/root/config/video.json"

#define MAX_PIX_FMT_NUM 5
#define MAX_RESOLUTION_NUM 10
#define MAX_FRAMERATE_NUM 10
#define MAX_PROFILE_NUM 10
#define MAX_PROFILE_STR_LEN 30

typedef struct _RESOLUTION {
    U32 width;
    U32 height;
} RESOLUTION;

typedef struct _RESOLUTION_CAP {
    RESOLUTION res;
    U32 framerates[MAX_FRAMERATE_NUM];
} RESOLUTION_CAP;

typedef struct _PIX_CAP {
    U32 pix_fmt;
    RESOLUTION_CAP res_cap[MAX_RESOLUTION_NUM];
} PIX_CAP;

typedef struct _VI_DEV_CAP {
    PIX_CAP pix_cap[MAX_PIX_FMT_NUM];
} VI_DEV_CAP;

typedef struct _VI_STRM {
    U32 pix_fmt;
    U32 framerate;
    U32 bitrate;
    U32 v4l2_buf_cnt;
    int v4l2_buf_len;
    int v4l2_fd;
    void **v4l2_buf_ptr;
    RESOLUTION res;
    char profile[MAX_PROFILE_STR_LEN];
} VI_STRM;

typedef struct _VI_PARAM {
    VI_DEV_CAP vi_dev_cap;
    VI_STRM strm;
} VI_PARAM;

const char* pix_fmt_u32_to_json_str(U32 pix_fmt);
U32 pix_fmt_json_str_to_u32(const char *json_str);
int check_config(VI_PARAM *vi_param);

#endif
