#Backing up previous LOCAL_PATH so it does not screw with the root Android.mk file
LOCAL_PATH_OLD := $(LOCAL_PATH)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE        := libcmft

LOCAL_LDLIBS        := -lm -llog -latomic

LOCAL_C_INCLUDES    := \
$(LOCAL_PATH)/include \
$(LOCAL_PATH)/src/cmft \
$(LOCAL_PATH)/src/cmft/common \
$(LOCAL_PATH)/dependency \
$(LOCAL_PATH)/dependency/stb \
$(LOCAL_PATH)/dependency/dm/include \
$(LOCAL_PATH)/dependency/CL \
$(LOCAL_PATH)/dependency/bx/include \
$(LOCAL_PATH)/dependency/bx/3rdparty

FILE_LIST := $(wildcard \
		$(LOCAL_PATH)/src/cmft/common/*.cpp \
		$(LOCAL_PATH)/src/cmft/*.cpp \
		)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

include $(BUILD_SHARED_LIBRARY)

#Putting previous LOCAL_PATH back here
LOCAL_PATH := $(LOCAL_PATH_OLD)