#ifndef CPATURE_H_
#define CAPUTRE_H_

int enum_video_device_capability(void);
int capture_init(VI_STRM *vi_strm);
void capture_deinit(VI_STRM *vi_strm);

#endif
