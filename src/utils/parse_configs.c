#include <json-c/json.h>
#include <linux/videodev2.h>

#include "common.h"
#include "dump_info.h"
#include "parse_config.h"
#include "video_config.h"

static int parse_video_capability(json_object *top, VI_DEV_CAP *vi_dev_cap) {
    int i = 0;
    U32 num = 0;
    U32 cur_fmt = 0;
    U32 res_idx = 0;
    U32 cur_width = 0;
    U32 cur_height = 0;
    U32 pix_fmt_idx = 0;
    U32 cur_framerate = 0;
    json_object *tmp = NULL;
    json_object *vi_dev_js = NULL;
    json_object *sensor_js = NULL;

    vi_dev_js = json_object_object_get(top, "video_device_capability");
    if (!vi_dev_js) {
        PRINT_ERROR("Can't parse video_device_capability config!");
        goto error;
    }

    sensor_js = json_object_object_get(vi_dev_js, SENSOR_NAME);
    if (!sensor_js) {
        PRINT_ERROR("Can't parse sensor config!");
        goto error;
    }

    pix_fmt_idx = 0;
    json_object_object_foreach(sensor_js, pix_fmt_key, pix_fmt_val) {
        cur_fmt = pix_fmt_json_str_to_u32(pix_fmt_key);

        if (0 == cur_fmt) {
            PRINT_ERROR("Pixel Format is unsupported!");
            goto error;
        }
        vi_dev_cap->pix_cap[pix_fmt_idx].pix_fmt = cur_fmt;

        res_idx = 0;
        json_object_object_foreach(pix_fmt_val, res_key, res_val) {
            cur_width = atoi(res_key);
            cur_height = atoi(strchr(res_key, '*') + 1);

            vi_dev_cap->pix_cap[pix_fmt_idx].res_cap[res_idx].res.width = cur_width;
            vi_dev_cap->pix_cap[pix_fmt_idx].res_cap[res_idx].res.height = cur_height;

            num = json_object_array_length(res_val);
            for (i = 0; i < num; ++i) {
                tmp = json_object_array_get_idx(res_val, i);
                cur_framerate = json_object_get_int(tmp);
                vi_dev_cap->pix_cap[pix_fmt_idx].res_cap[res_idx].framerates[i] = cur_framerate;
            }
            res_idx++;
        }
        pix_fmt_idx++;
    }

    dump_video_device_capability_stats(vi_dev_cap);

    return OK;
error:
    return ERROR;
}

static int parse_stream_capability(json_object *top, VI_STRM *vi_strm) {
    const char *s = NULL;
    json_object *strm_json = NULL;
    json_object *cur = NULL;

    strm_json = json_object_object_get(top, "stream_config");
    if (!strm_json) {
        PRINT_ERROR("Can't prase main stream config!");
        goto error;
    }

    cur = json_object_object_get(strm_json, "profile");
    if (!cur) {
        PRINT_ERROR("Can't parse stream profile config!");
        goto error;
    }
    s = json_object_get_string(cur);
    strncpy(vi_strm->profile, s, MIN(strlen(s), MAX_PROFILE_STR_LEN));

    cur = json_object_object_get(strm_json, "pix_fmt");
    if (!cur) {
        PRINT_ERROR("Can't parse stream profile config!");
        goto error;
    }
    s = json_object_get_string(cur);
    PRINT_DEBUG("stream pix_fmt: %s", s);
    vi_strm->pix_fmt = pix_fmt_json_str_to_u32(s);

    cur = json_object_object_get(strm_json, "resolution");
    if (!cur) {
        PRINT_ERROR("Can't parse stream resolution config!");
        goto error;
    }
    s = json_object_get_string(cur);
    vi_strm->res.width = atoi(s);
    vi_strm->res.height = atoi(strchr(s, '*') + 1);

    cur = json_object_object_get(strm_json, "framerate");
    if (!cur) {
        PRINT_ERROR("Can't parse framerate config!");
        goto error;
    }
    vi_strm->framerate = json_object_get_int(cur);

    cur = json_object_object_get(strm_json, "bitrate");
    if (!cur) {
        PRINT_ERROR("Can't parse bitrate config!");
        goto error;
    }
    vi_strm->bitrate = json_object_get_int(cur);

    cur = json_object_object_get(strm_json, "v4l2-buf-cnt");
    if (!cur) {
        PRINT_ERROR("Can't parse v4l2-buf-cnt config!");
        goto error;
    }
    vi_strm->v4l2_buf_cnt = json_object_get_int(cur);

    dump_stream_capability_stats(vi_strm);

    return OK;
error:
    return ERROR;
}

static int parse_video_config(VI_PARAM *vi_param) {
    int ret = 0;
    json_object *top = NULL;
    char *video_config_path = VIDEO_CONFIG_PATH;

    top = json_object_from_file(video_config_path);
    if (!top) {
        PRINT_ERROR("Can't parse video config files!");
        goto error;
    }

    ret = parse_video_capability(top, &vi_param->vi_dev_cap);
    if (OK != ret) {
        PRINT_ERROR("parse_video_capability failed!");
        goto error_free;
    }

    ret = parse_stream_capability(top, &vi_param->strm);
    if (OK != ret) {
        PRINT_ERROR("parse_streams_capability failed!");
        goto error_free;
    }

    json_object_put(top);
    return OK;
error_free:
    json_object_put(top);
error:
    return ERROR;
}

int parse_config(VI_PARAM *vi_param) {
    int ret = parse_video_config(vi_param);
    if (OK != ret) {
        PRINT_ERROR("parse_video_config failed!");
        goto error;
    }

    /*
     * \todo 这里后续可以增加其它配置，视项目需求而定
     */

    return OK;
error:
    return ERROR;
}
