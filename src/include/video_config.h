#ifndef VIDEO_CONFIG_H_
#define VIDEO_CONFIG_H_

#define VIDOE_CONFIG_PATH "/config/video.json"

#define MAX_RESOLUTION_NUM 10
#define MAX_FRAMERATE_NUM 10
#define MAX_PROFILE_NUM 10
#define MAX_PROFILE_STR_LEN 30

#define MAIN_STRM_JSON_STR "main"
#define MINOR_STRM_JSON_STR "minor"
#define STRM_ID_MAIN 0
#define STRM_ID_MINOR 1
#define STRM_NUM 2

typedef struct _RESOLUTION {
    int width;
    int height;
} RESOLUTION;

typedef struct _VI_CAP {
    RESOLUTION resolution[MAX_RESOLUTION_NUM];
    int framerate[MAX_FRAMERATE_NUM];
    char profile[MAX_PROFILE_NUM][MAX_PROFILE_STR_LEN];
} VI_CAP;

typedef struct _VI_STRM {
    char profile[MAX_PROFILE_STR_LEN];
    RESOLUTION resolution;
    int framerate;
    int bitrate;
} VI_STRM;

typedef struct _VI_PARAM {
    VI_CAP vi_cap;
    VI_STRM streams[STRM_NUM];
} VI_PARAM;

#endif
