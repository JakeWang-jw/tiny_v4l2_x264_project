CUR_DIR := $(shell pwd)

include $(CUR_DIR)/config.mk

###
### include路径
###
CFLAGS += -I$(CUR_DIR)/include

###
### 编译选项
###
# 用于启用几乎所有的警告信息，包括编译器认为值得注意的潜在问题
CFLAGS += -Wall
# 将所有警告信息转为错误信息
CFLAGS += -Werror
# 启用了一些额外的警告
CFLAGS += -Wextra
# 检测变量遮蔽问题，即内部变量与外部变量同名的情况
CFLAGS += -Wshadow
# 检测函数缺少原型声明
CFLAGS += -Wmissing-prototypes
# 禁止未使用函数的警告
CFLAGS += -Wno-unused-function
# 检测函数返回类型不匹配的问题
CFLAGS += -Wreturn-type
# 禁止有符号和无符号数比较的警告
CFLAGS += -Wno-sign-compare
# 使用c11标准
CFLAGS += -std=c11

###
### 链接选项
###
LDFLAGS += -L$(CUR_DIR)/lib
LDFLAGS += -lm -lpthread
LDFLAGS += -ljson-c
LDFLAGS += -lx264

export CFLAGS
export LDFLAGS

# 最终目标
COMPILE_TARGET := tiny_video_program

# 子文件夹路径
V4L2_CAPTURE_PATH := $(CUR_DIR)/v4l2-capture
X264_ENCODE_PATH := $(CUR_DIR)/x264-encode
UTILS_PATH := $(CUR_DIR)/utils

# 源文件及目标文件
SRCS := $(wildcard $(CUR_DIR)/*.c)
SRCS += $(wildcard $(V4L2_CAPTURE_PATH)/*.c)
SRCS += $(wildcard $(X264_ENCODE_PATH)/*.c)
SRCS += $(wildcard $(UTILS_PATH)/*.c)

OBJS := $(SRCS:.c=.o)

.phony: install all V4L2_CAPTURE X264_ENCODE UTILS
all: UTILS V4L2_CAPTURE X264_ENCODE $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(COMPILE_TARGET)

V4L2_CAPTURE:
	$(MAKE) -C $(V4L2_CAPTURE_PATH) all

X264_ENCODE:
	$(MAKE) -C $(X264_ENCODE_PATH) all

UTILS:
	$(MAKE) -C $(UTILS_PATH) all

install:
	$(INSTALL) $(CUR_DIR)/configs/*.json $(VIDEO_CONFIG_PATH)
	$(INSTALL) $(COMPILE_TARGET) $(PROGRAM_INSTALL_PATH)

.phony: clean
clean:
	$(MAKE) -C $(V4L2_CAPTURE_PATH) clean
	$(MAKE) -C $(X264_ENCODE_PATH) clean
	$(MAKE) -C $(UTILS_PATH) clean
	$(RM) *.o
	$(RM) $(COMPILE_TARGET)
