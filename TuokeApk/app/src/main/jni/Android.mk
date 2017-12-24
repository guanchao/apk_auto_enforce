LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := hackcodejiagu
LOCAL_SRC_FILES := hackcodejiagu.c
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
