LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE:= pigeon

LOCAL_MODULE_FILENAME := libpigeon

# 定义 all-cpp-files 返回当前路径和 Classes 路径想的所有 cpp 文件，注意：这里只考虑 cpp 而没有 c，如果需要自行添加
define all-cpp-files
$(patsubst jni/%,%, $(shell find $(LOCAL_PATH)/../../../Classes -iname "*.cpp" ! -iname "client.cpp" ! -iname "server.cpp" ! -iname "testOpenSSL.cpp"  ))  
endef
define all-c-files
$(patsubst jni/%,%, $(shell find $(LOCAL_PATH)/../../../external/snappy/android -iname "*.cc"  ))  
endef
# 这里使用新的方式替换换来的方式，以自动添加源文件
LOCAL_SRC_FILES := $(call all-cpp-files)
LOCAL_SRC_FILES += $(call all-c-files)
define all-c-files
$(patsubst jni/%,%, $(shell find $(LOCAL_PATH)/../../../Classes -iname "*.c"   ))  
endef
LOCAL_SRC_FILES += $(call all-c-files)

LOCAL_CFLAGS := -D_7ZIP_ST \
				-DANDROID \


LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../Classes \
					$(LOCAL_PATH)/../../../../external/openssl/include/android \
					$(LOCAL_PATH)/../../../external/snappy/android/ \
                    $(LOCAL_PATH)/../../../../cocos2d-x/external \
                    $(LOCAL_PATH)/../../../../cocos2d-x/external/lua/tolua \
                    $(LOCAL_PATH)/../../../../cocos2d-x/external/lua/luajit/include \
					$(LOCAL_PATH)/../../../../cocos2d-x/cocos 
LOCAL_STATIC_LIBRARIES := pc_crypto_static \
						pc_ssl_static \

include $(BUILD_STATIC_LIBRARY)


$(call import-add-path,$(LOCAL_PATH))
$(call import-module,external/openssl/prebuilt/android)
$(call import-module,.)

