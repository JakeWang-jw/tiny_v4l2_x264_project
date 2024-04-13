#include "common.h"
#include "dump_configs.h"

void dump_video_capability_stats(VI_CAP *vi_cap) {
    PRINT_DEBUG("VI_CAP:");
    PRINT_DEBUG("  resolution:");
    for (int i = 0; i < MAX_RESOLUTION_NUM; ++i) {
        if (vi_cap->resolution[i].width != -1 && vi_cap->resolution[i].height != -1) {
            PRINT_DEBUG("    %d*%d\n", vi_cap->resolution[i].width, vi_cap->resolution[i].height);
        }
    }
    PRINT_DEBUG("  framerate:");
    for (int i = 0; i < MAX_RESOLUTION_NUM; ++i) {
        if (vi_cap->framerate[i] != -1) {
            PRINT_DEBUG("    %d\n", vi_cap->framerate[i]);
        }
    }
    PRINT_DEBUG("  profile:");
    for (int i = 0; i < MAX_RESOLUTION_NUM; ++i) {
        if (vi_cap->profile[i][0] != '\0') {
            PRINT_DEBUG("    %s\n", vi_cap->profile[i]);
        }
    }
}

void dump_stream_capability_stats(VI_STRM *vi_strm) {
    PRINT_DEBUG("VI_STRM:");
    PRINT_DEBUG("  profile: %s", vi_strm->profile);
    PRINT_DEBUG("  resolution: %d*%d", vi_strm->resolution.width,
        vi_strm->resolution.height);
    PRINT_DEBUG("  framerate: %d", vi_strm->framerate);
    PRINT_DEBUG("  bitrate: %d", vi_strm->bitrate);
}
