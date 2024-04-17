#include <x264/x264.h>

#include "encode.h"

static x264_t *h;
static x264_nal_t *nal;
static x264_param_t param;
static x264_picture_t pic;

int x264_encode_init(VI_STRM *vi_strm) {
    int ret = 0;

    ret = x264_param_default_preset(&param, "fast", NULL);
    if(ret < 0) {
        PRINT_ERROR("x264_param_default_preset failed!");
        goto error;
    }

    /*
     * 设置编码相关参数
     */
    param.i_bitdepth = 8;
    param.i_csp = X264_CSP_I420;               // 目前只支持I420，后续计划通过libyuv来扩展
    param.i_width  = vi_strm->res.width;
    param.i_height = vi_strm->res.height;
    param.i_fps_num  = vi_strm->framerate;     // 设置帧率（分子）
    param.i_fps_den  = 1;                      // 设置帧率时间1s（分母）
    param.i_threads  = X264_SYNC_LOOKAHEAD_AUTO;
    param.i_keyint_max = 10;                   // 在此间隔设置IDR关键帧
    param.i_log_level  = X264_LOG_DEBUG;       //LOG参数
    param.i_frame_total = 0;                   //编码的桢数，不知道用0
    param.rc.i_bitrate = vi_strm->bitrate;     // 设置码率,在ABR(平均码率)模式下才生效，且必须在设置ABR前先设置bitrate
    param.rc.i_rc_method = X264_RC_ABR;        // 码率控制方法，CQP(恒定质量)，CRF(恒定码率,缺省值23)，ABR(平均码率)
    param.rc.b_mb_tree = 0;                    //这个不为0,将导致编码延时帧,在实时编码时,必须为0
    param.b_vfr_input = 0;
    param.b_repeat_headers = 1;                //在每个关键帧I帧前添加sps和pps，实时视频传输需要enable
    param.b_annexb = 0;                        //如果设置了该项，则在每个NAL单元前加一个四字节的前缀符

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

    PRINT_DEBUG("x264_encode_init success");
    return OK;
error_free:
    x264_picture_clean(&pic);
error:
    return ERROR;
}

void x264_encode_deinit(void) {
    x264_encoder_close(h);
    x264_picture_clean(&pic);
}

int x264_encode_one_frame(unsigned char* yuv420_buf) {
    int i_nal;
    int ret = 0;
    int y_size = param.i_width * param.i_height;
    x264_picture_t pic_out;

    memcpy(pic.img.plane[0], yuv420_buf, y_size);                        // Y
    memcpy(pic.img.plane[1], &yuv420_buf[y_size], y_size >> 2);          // U
    memcpy(pic.img.plane[2], &yuv420_buf[(y_size*5)>>2], y_size >> 2);   // V
    ret = x264_encoder_encode(h, &nal, &i_nal, &pic, &pic_out);
    if (ret < 0) {
        PRINT_ERROR("x264_encoder_encode failed!");
        goto error;
    }

    // 这里把pic_out传给live555

    return OK;
error:
    x264_encoder_close(h);
    x264_picture_clean(&pic);
    return ERROR;
}
