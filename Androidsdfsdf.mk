LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := thomas_command_line
LOCAL_SRC_FILES := \
    main.cpp
include $(BUILD_EXECUTABLE)