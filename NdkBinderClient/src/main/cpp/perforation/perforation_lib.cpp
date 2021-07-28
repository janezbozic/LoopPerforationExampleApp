//
// Created by janez on 18. 07. 21.
//

#include <cstdlib>
#include "perforation_lib.h"
#include <jni.h>
#include <aidl/com/example/IMyService.h>
#include <android/binder_ibinder_jni.h>
#include <LogDefs.h>
#include <cstring>
#include <unordered_map>
#include "LoopInfo.h"

using aidl::com::example::IMyService;
using ndk::ScopedAStatus;
using namespace std;

std::shared_ptr<IMyService> g_spMyService;

std::unordered_map<int, LoopInfo> prevFactors;

int CLANG_LOOP_PERFORATION_FUNCTION(int loopId){

    auto itr = prevFactors.find(loopId);

    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    int curTimeVal = currentTime.tv_sec * 1000000 + currentTime.tv_usec;

    if (itr != prevFactors.end() && curTimeVal - itr->second.getTime() < 50000)
        return itr->second.getStoredFactor();

    int newFactor;
    ScopedAStatus getPerforationFactorResult = g_spMyService->getPerforationFactor(loopId, &newFactor);

    if (itr != prevFactors.end()) {
        itr->second.setStoredFactor(newFactor);
        itr->second.setTime(curTimeVal);
    }
    else {
        LoopInfo newInfo;
        newInfo.setLoopId(loopId);
        newInfo.setStoredFactor(newFactor);
        newInfo.setTime(curTimeVal);
        prevFactors.insert(make_pair(loopId, newInfo));
    }

    return newFactor;
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
