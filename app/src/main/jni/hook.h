
#include <stdio.h>
#include <android/log.h>
#include <unistd.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "zzHook", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "zzHook", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "zzHook", __VA_ARGS__))

uint32_t get_module_base(pid_t pid, const char *module_path);