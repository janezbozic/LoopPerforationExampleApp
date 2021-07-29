//
// Created by janez on 18. 07. 21.
//

#include <cstdlib>
#include "perforation_lib.h"
#include <jni.h>
#include <aidl/com/example/IMyService.h>
#include <LoopPerfFactor.h>
#include <android/binder_ibinder_jni.h>
#include <LogDefs.h>
#include <cstring>
#include <unordered_map>
#include "LoopInfo.h"

using aidl::com::example::IMyService;
using aidl::com::example::LoopPerfFactor;
using ndk::ScopedAStatus;
using namespace std;

std::shared_ptr<IMyService> g_spMyService;

std::unordered_map<int, LoopInfo> prevFactors;

int CLANG_LOOP_PERFORATION_FUNCTION(int loopId){

        auto itr = prevFactors.find(loopId);

    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    if (itr != prevFactors.end()) {

        u_long time = (currentTime.tv_sec - itr->second.getTimeSec())*1000000 + currentTime.tv_usec - itr->second.getTimeUSec();
        if (time < itr->second.getLifeSpan())
            return itr->second.getStoredFactor();
    }

    LoopPerfFactor newFactorPackage;
    ScopedAStatus getPerforationFactorResult = g_spMyService->getPerforationFactor(loopId, &newFactorPackage);

    int newFactor;
    newFactorPackage.getPerfFactor(&newFactor);

    int lifeSpan;
    newFactorPackage.getLifeTime(&lifeSpan);

    if (itr != prevFactors.end()) {
        itr->second.setStoredFactor(newFactor);
        itr->second.setTimeSec(currentTime.tv_sec);
        itr->second.setTimeUSec(currentTime.tv_usec);
        itr->second.setLifeSpan(lifeSpan);
    }
    else {
        LoopInfo newInfo;
        newInfo.setLoopId(loopId);
        newInfo.setStoredFactor(newFactor);
        newInfo.setTimeSec(currentTime.tv_sec);
        newInfo.setTimeUSec(currentTime.tv_usec);
        newInfo.setLifeSpan(lifeSpan);
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
