LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := unzip
LOCAL_SRC_FILES := unzip.cpp zip.cpp
LOCAL_C_INCLUDES := $(SHARED_PATH_INC)

include $(BUILD_STATIC_LIBRARY)
