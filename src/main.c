#include "common.h"
#include "parse_config.h"

static void param_init(VI_PARAM *vi_param) {
    // init vi_param
    memset(vi_param, 0x0, sizeof(*vi_param));
    for (int i = 0; i < MAX_RESOLUTION_NUM; ++i) {
        vi_param->vi_cap.resolution[i].width = -1;
        vi_param->vi_cap.resolution[i].height = -1;
    }
    for (int i = 0; i < MAX_FRAMERATE_NUM; ++i) {
        vi_param->vi_cap.framerate[i] = -1;
    }
    for (int i = 0; i < MAX_PROFILE_NUM; ++i) {
        vi_param->vi_cap.profile[i][0] = '\0';
    }
    for (int i = 0; i < STRM_NUM; ++i) {
        vi_param->streams[i].profile[0] = '\0';
        vi_param->streams[i].resolution.width = -1;
        vi_param->streams[i].resolution.height = -1;
        vi_param->streams[i].framerate = -1;
        vi_param->streams[i].bitrate = -1;
    }
}

int main(void) {
    int ret = 0;
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
        goto error;
    }

    return OK;
error:
    return ERROR;
}
