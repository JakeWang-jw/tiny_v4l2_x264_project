#include <linux/videodev2.h>

#include "common.h"
#include "video_config.h"

static const struct {
    const char *str;
    U32 value;
} json_str_to_u32_map[] = {
    {"rgb565", V4L2_PIX_FMT_RGB565},
    {"yuv420", V4L2_PIX_FMT_YUV420},
    {NULL, 0}
};

const char* pix_fmt_u32_to_json_str(U32 pix_fmt) {
    for (int i = 0; json_str_to_u32_map[i].value != 0; i++) {
        if (pix_fmt == json_str_to_u32_map[i].value) {
            return json_str_to_u32_map[i].str;
        }
    }

    return NULL;
}

U32 pix_fmt_json_str_to_u32(const char *json_str) {
    for (int i = 0; json_str_to_u32_map[i].str != NULL; i++) {
        PRINT_ERROR("str1: %s, str2: %s", json_str_to_u32_map[i].str,
            json_str);
        if (strcmp(json_str_to_u32_map[i].str, json_str) == 0) {
            PRINT_ERROR("Val: %u", json_str_to_u32_map[i].value);
            return json_str_to_u32_map[i].value;
        }
    }

    return 0;
}

int check_config(VI_PARAM *vi_param) {
    int i = 0;
    for (i = 0; i < MAX_PIX_FMT_NUM; ++i) {
        if (0 == vi_param->vi_dev_cap.pix_cap[i].pix_fmt) {
            PRINT_ERROR("pix_fmt equal to zero!");
            goto error;
        }
        if (vi_param->strm.pix_fmt != vi_param->vi_dev_cap.pix_cap[i].pix_fmt) {
            continue;
        }
        for (int j = 0; j < MAX_RESOLUTION_NUM; ++j) {
            if (0 == vi_param->vi_dev_cap.pix_cap[i].res_cap[j].res.width ||
                0 == vi_param->vi_dev_cap.pix_cap[i].res_cap[j].res.height) {
                PRINT_ERROR("Illegal resolution! Cur index: %d", j);
                goto error;
            }
            if (vi_param->strm.res.width != vi_param->vi_dev_cap.pix_cap[i].res_cap[j].res.width ||
                vi_param->strm.res.height != vi_param->vi_dev_cap.pix_cap[i].res_cap[j].res.height) {
                continue;
            }
            for (int k = 0; k < MAX_FRAMERATE_NUM; ++k) {
                if (0 == vi_param->vi_dev_cap.pix_cap[i].res_cap[j].framerates[k]) {
                    PRINT_ERROR("Illegal framerate! Cur index: %d", k);
                    goto error;
                }
                if (vi_param->strm.framerate == vi_param->vi_dev_cap.pix_cap[i].res_cap[j].framerates[k]) {
                    goto ok;
                }
            }
        }
    }

    if (i == MAX_PIX_FMT_NUM) {
        PRINT_ERROR("Illegal pix_fmt!");
        goto error;
    }
ok:
    return OK;
error:
    return ERROR;
}
