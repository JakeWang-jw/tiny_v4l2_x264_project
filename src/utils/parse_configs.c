#include "common.h"
#include <json-c/json.h>
#include "parse_config.h"
#include "video_config.h"
#include "dump_configs.h"

static int parse_video_capability(json_object *top, VI_CAP *vi_cap) {
    json_object *cur = NULL;
    json_object *video_capability = NULL;
    int num = 0;

    video_capability = json_object_object_get(top, "video_capability");
    if (!video_capability) {
        PRINT_ERROR("Can't parse video_capability config!");
        goto error;
    }

    cur = json_object_object_get(video_capability, "resolution");
    if (!cur) {
        PRINT_ERROR("Can't parse resolution config!");
        goto error;
    }
    num = json_object_array_length(cur);
    for (int i = 0; i < num; ++i) {
        json_object *tmp = json_object_array_get_idx(cur, i);
        const char *s = json_object_get_string(tmp);
        vi_cap->resolution[i].width = atoi(s);
        vi_cap->resolution[i].height = atoi(strchr(s, '*') + 1);
    }

    cur = json_object_object_get(video_capability, "framerate");
    if (!cur) {
        PRINT_ERROR("Can't parse framerate config!");
        goto error;
    }
    num = json_object_array_length(cur);
    for (int i = 0; i < num; ++i) {
        json_object *tmp = json_object_array_get_idx(cur, i);
        vi_cap->framerate[i] = json_object_get_int(tmp);
    }

    cur = json_object_object_get(video_capability, "profile");
    if (!cur) {
        PRINT_ERROR("Can't parse profile config!");
        goto error;
    }
    num = json_object_array_length(cur);
    for (int i = 0; i < num; ++i) {
        json_object *tmp = json_object_array_get_idx(cur, i);
        const char *s = json_object_get_string(tmp);
        strncpy(vi_cap->profile[i], s, MIN(strlen(s), MAX_PROFILE_STR_LEN));
    }

    dump_video_capability_stats(vi_cap);

    return OK;
error:
    return ERROR;
}

static int parse_stream_capability(json_object *strm_obj, VI_STRM *cur_strm) {
    const char *s = NULL;
    json_object *cur = NULL;

    cur = json_object_object_get(strm_obj, "profile");
    if (!cur) {
        PRINT_ERROR("Can't parse stream profile config!");
        goto error;
    }
    s = json_object_get_string(cur);
    strncpy(cur_strm->profile, s, MIN(strlen(s), MAX_PROFILE_STR_LEN));

    cur = json_object_object_get(strm_obj, "resolution");
    if (!cur) {
        PRINT_ERROR("Can't parse stream resolution config!");
        goto error;
    }
    s = json_object_get_string(cur);
    cur_strm->resolution.width = atoi(s);
    cur_strm->resolution.height = atoi(strchr(s, '*') + 1);

    cur = json_object_object_get(strm_obj, "framerate");
    if (!cur) {
        PRINT_ERROR("Can't parse framerate config!");
        goto error;
    }
    cur_strm->framerate = json_object_get_int(cur);

    cur = json_object_object_get(strm_obj, "bitrate");
    if (!cur) {
        PRINT_ERROR("Can't parse bitrate config!");
        goto error;
    }
    cur_strm->bitrate = json_object_get_int(cur);

    dump_stream_capability_stats(cur_strm);

    return OK;
error:
    return ERROR;
}

static int parse_streams_capability(json_object *top, VI_STRM *vi_strms) {
    int ret = 0;
    json_object *strm_cap = NULL;

    strm_cap = json_object_object_get(top, MAIN_STRM_JSON_STR);
    if (!strm_cap) {
        PRINT_ERROR("Can't prase main stream config!");
        goto error;
    }
    ret = parse_stream_capability(strm_cap, &vi_strms[STRM_ID_MAIN]);
    if (OK != ret) {
        PRINT_ERROR("Can't parse main stream config!");
        goto error;
    }

    strm_cap = json_object_object_get(top, MINOR_STRM_JSON_STR);
    if (!strm_cap) {
        PRINT_ERROR("Can't prase minor stream config!");
        goto error;
    }
    ret = parse_stream_capability(strm_cap, &vi_strms[STRM_ID_MINOR]);
    if (OK != ret) {
        PRINT_ERROR("Can't parse minor stream config!");
        goto error;
    }

    return OK;
error:
    return ERROR;
}

static int parse_video_config(VI_PARAM *vi_param) {
    int ret = 0;
    json_object *top = NULL;
    char *video_config_path = VIDOE_CONFIG_PATH;

    top = json_object_from_file(video_config_path);
    if (!top) {
        PRINT_ERROR("Can't parse video config files!");
        goto error;
    }

    ret = parse_video_capability(top, &vi_param->vi_cap);
    if (OK != ret) {
        PRINT_ERROR("parse_video_capability failed!");
        goto error;
    }

    ret = parse_streams_capability(top, vi_param->streams);
    if (OK != ret) {
        PRINT_ERROR("parse_streams_capability failed!");
        goto error;
    }

    json_object_put(top);
    return OK;
error:
    return ERROR;
}

int parse_config(VI_PARAM *vi_param) {
    int ret = parse_video_config(vi_param);
    if (OK != ret) {
        PRINT_ERROR("parse_video_config failed!");
        goto error;
    }

    return OK;
error:
    return ERROR;
}
