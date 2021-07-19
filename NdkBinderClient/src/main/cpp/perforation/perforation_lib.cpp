//
// Created by janez on 18. 07. 21.
//

#include <cstdlib>
#include "perforation_lib.h"
#include <jni.h>
#include <aidl/com/example/IMyService.h>
#include <android/binder_ibinder_jni.h>
#include <LogDefs.h>

using aidl::com::example::IMyService;
using ndk::ScopedAStatus;
using namespace std;

std::shared_ptr<IMyService> g_spMyService;

int CLANG_LOOP_PERFORATION_FUNCTION(int n){
    int c;

    ScopedAStatus getPerforationFactorResult = g_spMyService->getPerforationFactor(n, &c);

    if(getPerforationFactorResult.isOk())
    {
        LOGD("[App] [cpp] IMyService.basicTypes - Succeeded");
    }
    else
    {
        LOGE("[App] [cpp] IMyService.basicTypes - Failed");
    }
    return c;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_ndkbinderclient_MainActivity_onServiceConnected(
        JNIEnv* env,
        jobject /* this */,
        jobject binder)
{
    AIBinder* pBinder = AIBinder_fromJavaBinder(env, binder);

    const ::ndk::SpAIBinder spBinder(pBinder);
    g_spMyService = IMyService::fromBinder(spBinder);

    LOGD("[App] [cpp] onServiceConnected");
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_ndkbinderclient_MainActivity_onServiceDisconnected(
        JNIEnv* env,
        jobject /* this */)
{
    g_spMyService = nullptr;

    LOGD("[App] [cpp] onServiceDisconnected");
}
