#include "common.h"
#include "dump_info.h"
#include "parse_config.h"

void dump_video_device_capability_stats(VI_DEV_CAP *vi_dev_cap) {
    PRINT_DEBUG("VI_DEV_CAP:");
    for (int i = 0; i < MAX_PIX_FMT_NUM; ++i) {
        if (0 == vi_dev_cap->pix_cap[i].pix_fmt) {
            continue;
        }
        PRINT_DEBUG("  pix_fmt: %s", pix_fmt_u32_to_json_str(vi_dev_cap->pix_cap[i].pix_fmt));
        for (int j = 0; j < MAX_RESOLUTION_NUM; ++j) {
            if (0 == vi_dev_cap->pix_cap[i].res_cap[j].res.width
                || 0 == vi_dev_cap->pix_cap[i].res_cap[j].res.height) {
                continue;
            }
            PRINT_DEBUG("    resolution: %u, %u", vi_dev_cap->pix_cap[i].res_cap[j].res.width,
                vi_dev_cap->pix_cap[i].res_cap[j].res.height);
            PRINT_DEBUG("      framerate: ");
            for (int k = 0; k < MAX_FRAMERATE_NUM; ++k) {
                if (0 == vi_dev_cap->pix_cap[i].res_cap[j].framerates[k]) {
                    continue;
                }
                PRINT_DEBUG("        %u", vi_dev_cap->pix_cap[i].res_cap[j].framerates[k]);
            }
        }
    }
}

void dump_stream_capability_stats(VI_STRM *vi_strm) {
    PRINT_DEBUG("VI_STRM:");
    PRINT_DEBUG("  profile: %s", vi_strm->profile);
    PRINT_DEBUG("  pix_fmt: %u", vi_strm->pix_fmt);
    PRINT_DEBUG("  resolution: %u*%u", vi_strm->res.width, vi_strm->res.height);
    PRINT_DEBUG("  framerate: %u", vi_strm->framerate);
    PRINT_DEBUG("  bitrate: %u", vi_strm->bitrate);
    PRINT_DEBUG("  v4l2-buf-cnt: %u", vi_strm->v4l2_buf_cnt);
}
