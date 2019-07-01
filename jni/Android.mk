include $(CLEAR_VARS)
LOCAL_PATH := $(call my-dir)





TF_DIR := /home/alok/DATA-WS/tensorflow-prebuilt/tensorflow-android-static
LOCAL_CLAGS := -std=c++11  -llog -fno-rtti  -Wall -Wc++11-extensions# whatever g++ flags you like
LOCAL_CPPFLAGS := -std=c++11  -llog -fno-rtti  -Wall -Wc++11-extensions# whatever g++ flags you like
LOCAL_MODULE := tensorflow-core
LOCAL_SRC_FILES := $(TF_DIR)/tensorflow/contrib/makefile/gen/lib/android_arm64-v8a/libtensorflow-core.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE := nsync
LOCAL_SRC_FILES := $(TF_DIR)/tensorflow/contrib/makefile/downloads/nsync/builds/arm64-v8a.android.c++11/libnsync.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE := protobuf
LOCAL_SRC_FILES := $(TF_DIR)/tensorflow/contrib/makefile/gen/protobuf_android/arm64-v8a/lib/libprotobuf.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE := tacotron2
LOCAL_CFLAGS   += -DANDROID26
#APP_OPTIM := debug
LOCAL_PATH := $(call my-dir)
LOCAL_SRC_FILES :=  $(LOCAL_PATH)/src/main.cc  $(LOCAL_PATH)/src/tf_synthesizer.cc
LOCAL_WHOLE_STATIC_LIBRARIES := tensorflow-core
LOCAL_STATIC_LIBRARIES := nsync protobuf
LOCAL_C_INCLUDES := /data/alok/tensorflow-android/target/tensorflow \
/data/alok/tensorflow-android/target/tensorflow/tensorflow/contrib/makefile/gen/proto \
/data/alok/tensorflow-android/target/tensorflow/tensorflow/contrib/makefile/gen/protobuf_android/arm64-v8a/include \
$(TF_DIR)/target/tensorflow/tensorflow/contrib/makefile/downloads/eigen \
$(TF_DIR)/target/tensorflow/tensorflow/contrib/makefile/downloads/absl \
$(TF_DIR)/target/tensorflow/tensorflow/contrib/makefile/downloads/nsync/public
LOCAL_LDFLAGS := -rpath /data/local -llog
include $(BUILD_EXECUTABLE)



