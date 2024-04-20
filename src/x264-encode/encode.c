#include <x264/x264.h>

#include "encode.h"

static x264_t *h = NULL;
static x264_param_t param;
static x264_picture_t pic;

#define H264_FILE_PATH "/home/root/video/output.h264"
static FILE *fp = NULL;

static int v4l2_buf_to_x264_pic(unsigned char *buf) {
	int i = 0;
    int j = 0;
    int uvOffset = param.i_width * 2 * sizeof(uint8_t);
	uint8_t *y = pic.img.plane[0];
	uint8_t *u = pic.img.plane[1];
	uint8_t *v = pic.img.plane[2];
    uint8_t *cur = (uint8_t *)buf;

    if (!buf) {
        PRINT_ERROR("Src YUYV buffer is empty!");
        goto error;
    }

    /*
     * \attention 直接取yuv422中的一个值会存在偏色问题，所以选择取平均值的方式
     */
    for(i = 0; i < param.i_height - 1; i++) {
        for(j = 0; j < param.i_width; j += 2) {
            uint16_t calc = 0;
            int evenRow = ((i&1) == 0);
            *y++ = *cur++;
            if (evenRow) {
                calc = *cur;
                calc += *(cur + uvOffset);
                calc /= 2;
                *u++ = (uint8_t)calc;
            }
            ++cur;
            *y++ = *cur++;
            if (evenRow)
            {
                calc = *cur;
                calc += *(cur + uvOffset);
                calc /= 2;
                *v++ = (uint8_t) calc;
            }
            ++cur;
        }
    }

    return OK;
error:
    return ERROR;
}

int x264_encode_init(VI_STRM *vi_strm) {
    int ret = 0;

    ret = x264_param_default_preset(&param, "ultrafast", NULL);
    if(ret < 0) {
        PRINT_ERROR("x264_param_default_preset failed!");
        goto error;
    }

    /*
     * 设置编码相关参数
     */
    param.i_bitdepth = 8;
    param.i_csp = X264_CSP_I420;               // 目前只支持I420
    param.i_width  = vi_strm->res.width;
    param.i_height = vi_strm->res.height;
    param.i_fps_num  = vi_strm->framerate;     // 设置帧率（分子）
    param.i_fps_den  = 1;                      // 设置帧率时间1s（分母）
    param.i_threads  = X264_SYNC_LOOKAHEAD_AUTO;
    param.i_log_level  = X264_LOG_ERROR;       // LOG参数
    param.i_frame_total = 0;                   // 编码的帧数，不知道用0
    param.rc.i_bitrate = vi_strm->bitrate;     // 设置码率,在ABR(平均码率)模式下才生效，且必须在设置ABR前先设置bitrate
    param.rc.i_rc_method = X264_RC_ABR;        // 码率控制方法，CQP(恒定质量)，CRF(恒定码率,缺省值23)，ABR(平均码率)
    param.rc.b_mb_tree = 0;                    // 这个不为0,将导致编码延时帧,在实时编码时,必须为0
    param.b_vfr_input = 0;
    param.b_repeat_headers = 1;                // 在每个关键帧I帧前添加sps和pps，实时视频传输需要enable
    /*
     * \attention 这个不改为1的话，ffprobe解析不出来
     */
    param.b_annexb = 1;                        // 如果设置了该项，则在每个NAL单元前加一个四字节的前缀符

    /*
     * 设置profile
     */
    ret = x264_param_apply_profile(&param, vi_strm->profile);
    if(ret < 0) {
        PRINT_ERROR("x264_param_apply_profile failed!");
        goto error;
    }

    /*
     * 分配原图像buffer
     */
    ret = x264_picture_alloc(&pic, param.i_csp, param.i_width, param.i_height);
    if(ret < 0) {
        PRINT_ERROR("x264_picture_alloc failed!");
        goto error;
    }

    /*
     * 开启编码器
     */
    h = x264_encoder_open(&param);
    if(!h) {
        PRINT_ERROR("x264_encoder_open failed!");
        goto error_free;
    }

    fp = fopen(H264_FILE_PATH, "w");
    if (!fp) {
        PRINT_ERROR("fopen H264_FILE failed!");
        goto error_free;
    }

    PRINT_DEBUG("x264_encode_init success");
    return OK;
error_free:
    x264_picture_clean(&pic);
error:
    return ERROR;
}

void x264_encode_deinit(void) {
    int i_nal = 0;
    int i_frame_size = 0;
    x264_nal_t *nal = NULL;
    x264_picture_t pic_out;

    /* Flush delayed frames */
    while( x264_encoder_delayed_frames( h ) )
    {
        i_frame_size = x264_encoder_encode(h, &nal, &i_nal, NULL, &pic_out);
        if(i_frame_size > 0)
        {
            fwrite(nal->p_payload, i_frame_size, 1, fp);
        }
    }
    x264_encoder_close(h);
    x264_picture_clean(&pic);
    fclose(fp);
}

int x264_encode_one_frame(unsigned char* yuyv422_buf) {
    int ret = 0;
    int i_nal = 0;
    int i_frame_size = 0;
    x264_nal_t *nal = NULL;
    x264_picture_t pic_out;

    ret = v4l2_buf_to_x264_pic(yuyv422_buf);
    if (OK != ret) {
        PRINT_ERROR("v4l2_buf_to_x264_pic failed!");
        goto error;
    }

    i_frame_size = x264_encoder_encode(h, &nal, &i_nal, &pic, &pic_out);
    if (i_frame_size < 0) {
        PRINT_ERROR("x264_encoder_encode failed!");
        goto error;
    }

    if (i_frame_size > 0) {
        fwrite(nal->p_payload, i_frame_size, 1, fp);
	}

    return OK;
error:
    x264_encoder_close(h);
    x264_picture_clean(&pic);
    return ERROR;
}
