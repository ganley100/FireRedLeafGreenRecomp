// android_device.cpp — see android_device.h.
#include "android_device.h"

#if defined(__ANDROID__)
#include <jni.h>
#include <SDL.h>
#include <SDL_system.h>
#endif

namespace frlg {

#if defined(__ANDROID__)
namespace {

// PackageManager.hasSystemFeature(feature) on the SDL activity. Returns
// `fallback` if any JNI step fails (the check must never crash the launch).
bool has_system_feature(JNIEnv* env, jobject activity, const char* feature,
                        bool fallback) {
    bool result = fallback;
    jclass activity_class = env->GetObjectClass(activity);
    jmethodID get_pm = env->GetMethodID(activity_class, "getPackageManager",
                                        "()Landroid/content/pm/PackageManager;");
    jobject pm = get_pm ? env->CallObjectMethod(activity, get_pm) : nullptr;
    if (pm && !env->ExceptionCheck()) {
        jclass pm_class = env->GetObjectClass(pm);
        jmethodID has = env->GetMethodID(pm_class, "hasSystemFeature",
                                         "(Ljava/lang/String;)Z");
        jstring name = env->NewStringUTF(feature);
        if (has && name) {
            const jboolean value = env->CallBooleanMethod(pm, has, name);
            if (!env->ExceptionCheck()) result = value == JNI_TRUE;
        }
        if (name) env->DeleteLocalRef(name);
        env->DeleteLocalRef(pm_class);
    }
    if (env->ExceptionCheck()) env->ExceptionClear();
    if (pm) env->DeleteLocalRef(pm);
    env->DeleteLocalRef(activity_class);
    return result;
}

}  // namespace

bool android_is_tv_device() {
    JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    jobject activity = static_cast<jobject>(SDL_AndroidGetActivity());
    if (!env || !activity) return false;
    const bool leanback =
        has_system_feature(env, activity, "android.software.leanback", false);
    const bool touch =
        has_system_feature(env, activity, "android.hardware.touchscreen", true);
    env->DeleteLocalRef(activity);
    return leanback || !touch;
}
#else
bool android_is_tv_device() { return false; }
#endif

}  // namespace frlg
