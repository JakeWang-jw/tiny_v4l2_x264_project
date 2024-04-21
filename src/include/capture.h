#ifndef CPATURE_H_
#define CAPUTRE_H_

int enum_video_device_capability(void);
int v4l2_capture_init(VI_STRM *vi_strm);
void v4l2_capture_deinit(VI_STRM *vi_strm);

#endif
