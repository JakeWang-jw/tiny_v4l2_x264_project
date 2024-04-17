#include "common.h"
#include "capture.h"
#include "parse_config.h"

static void param_init(VI_PARAM *vi_param) {
    // init vi_param
    memset(vi_param, 0x0, sizeof(*vi_param));
    vi_param->strm.profile[0] = '\0';
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
        goto error_free;
    }

    ret = check_config(vi_param);
    if (OK != ret) {
        PRINT_ERROR("stream config is illegal!\n");
        goto error_free;
    }

    ret = enum_video_device_capability();
    if (OK != ret) {
        PRINT_ERROR("enum_video_device_capability failed!");
        goto error_free;
    }

    free(vi_param);
    return OK;
error_free:
    free(vi_param);
error:
    return ERROR;
}
