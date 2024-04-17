#ifndef _ENCODE_H_
#define _ENCODE_H_
#include "common.h"

int x264_encode_init(VI_STRM *vi_strm);
void x264_encode_deinit(void);
int x264_encode_one_frame(unsigned char* yuv420_buf);

#endif
