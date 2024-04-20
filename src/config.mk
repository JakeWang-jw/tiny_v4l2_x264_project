CUR_DIR = $(shell pwd)
include $(CUR_DIR)/configs/compile.config

ifneq ($(CONFIG_LOG_LEVEL),)
CFLAGS += -D$(CONFIG_LOG_LEVEL)
else
CFLAGS += -DLOG_LEVEL_INFO
endif

ifneq ($(CONFIG_SENSOR_NAME),)
CFLAGS += -DUSE_OV5640
else
CFLAGS += -DUSE_SC2336
endif

ifneq ($(CONFIG_CONFIG_OUTPUT_PATH),)
VIDEO_CONFIG_PATH := $(CONFIG_CONFIG_OUTPUT_PATH)
else
VIDEO_CONFIG_PATH := /home/alientek/linux/nfs/rootfs/config
endif

ifneq ($(CONFIG_PROGRAM_INSTALL_PATH),)
PROGRAM_INSTALL_PATH := $(CONFIG_PROGRAM_INSTALL_PATH)
else
PROGRAM_INSTALL_PATH:=/home/alientek/linux/nfs/rootfs/bin
endif

ifneq ($(CONFIG_CROSS_COMPILE),)
CROSS_COMPILE := $(CONFIG_CROSS_COMPILE)
else
CROSS_COMPILE := arm-linux-gnueabihf
endif

export CC := $(CROSS_COMPILE)-gcc
export AR := $(CROSS_COMPILE)-ar

export INSTALL := install -m 755

ifneq ($(CONFIG_H264_FRAME_NUM),)
export CFLAGS += -DH264_FRAME_NUM=$(CONFIG_H264_FRAME_NUM)
endif
