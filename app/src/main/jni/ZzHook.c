#include <stdio.h>

#include <android/log.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/types.h>

#include "hook.h"
#include "hookzz.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "zzHook", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "zzHook", __VA_ARGS__))

#define INVALID_FUNC_ADDR NULL

void _init(char *args) {
    LOGI("hook is start");
}


uint32_t (*lua_loadx_ori)(void *, void *, void *, const char *, const char *) = NULL;

uint32_t lua_loadx_replace(void *L, void *reader, void *dt,
                           const char *chunkname, const char *mode) {
    LOGI("%s:%s", __FUNCTION__, chunkname);


    return lua_loadx_ori(L, reader, dt, chunkname, mode);
}

uint32_t (*dvmLoadNativeCode_ori)(const char *, uint32_t, uint32_t) = NULL;

uint32_t dvmLoadNativeCode_replace(const char *a1, uint32_t a2, uint32_t a3) {
    // LOGI("%s:%s", __FUNCTION__, a1);

    static int xx_isDone = 1;
    //判断是否是该package
    char *target_package = "com.bilibili.azurlane";
    if (strstr(a1, target_package) && xx_isDone) {
        LOGI("find the target_package:%s", target_package);
        char *libpath = "/data/data/com.bilibili.azurlane/lib/libtolua.so";

        void *handler = dlopen(libpath, RTLD_NOW);

        char *err = (char *) dlerror();
        if (err != NULL)
            LOGI("dlopen error is:%s\n", err);
        if (handler) {
            LOGI("hook is start");
            pid_t pid = getpid();
            LOGI("pid is %d", pid);
            uint32_t module_base = get_module_base(pid,
                                                   "/data/app-lib/com.bilibili.azurlane-1/libtolua.so");
            if (module_base) {
                LOGI("module_base:%x", module_base);
                uint32_t lua_loadx_target = module_base + 0x3BCB4;
                ZzHook((void *) lua_loadx_target, (void *) &lua_loadx_replace,
                       (void **) &lua_loadx_ori, NULL, NULL, false);
            }

            xx_isDone = 0;
        }
    }

    return dvmLoadNativeCode_ori(a1, a2, a3);
}


void so_entry(char *p) {
    char *module_path = "/system/lib/libdvm.so";

    void *handler1 = dlopen("/system/lib/libsubstrate.so", RTLD_NOW);
    char *err = (char *) dlerror();
    if (err != NULL)
        LOGI("dlopen error is:%s\n", err);

    void (*MSHookFunction)(void *symbol, void *replace, void **result) = INVALID_FUNC_ADDR;
    MSHookFunction = dlsym(handler1, "MSHookFunction");
    if (MSHookFunction == INVALID_FUNC_ADDR) {
        LOGD("can't find MSHookFunction");
    }

    pid_t pid = getpid();
    LOGI("the target pid is %d", pid);
    uint32_t module_base = get_module_base(pid, module_path);

    LOGI("the module_base is %x", module_base);
    if (module_base) {
        uint32_t dvmLoadNativeCode_target = module_base + 0x4FE30 + 1;//一定要注意是ARM指令还是Thumb指令
        // MSHookFunction((void*)dvmLoadNativeCode_target, (void *)&dvmLoadNativeCode_replace, (void **)&dvmLoadNativeCode_ori);


        ZzHook((void *) dvmLoadNativeCode_target, (void *) &dvmLoadNativeCode_replace,
               (void **) &dvmLoadNativeCode_ori, NULL, NULL, false);
        LOGI("hook done!");
    }

}
