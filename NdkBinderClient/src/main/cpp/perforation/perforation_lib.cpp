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

//Service binder passed from Main Activity
std::shared_ptr<IMyService> g_spMyService;

//Hashmap for perforation factor caching
std::unordered_map<int, LoopInfo> prevFactors;

//Our main perforation function, for which the call is inserted in compilation phase for every
//  perforable loop
int CLANG_LOOP_PERFORATION_FUNCTION(int loopId, int upperValue){

    //Iterator for hashmap
    auto itr = prevFactors.find(loopId);

    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    //If factor is already cached and is not too old, we return it
    if (itr != prevFactors.end()) {
        u_long time = (currentTime.tv_sec - itr->second.getTimeSec())*1000000 + currentTime.tv_usec - itr->second.getTimeUSec();
        if (time < itr->second.getLifeSpan())
            return itr->second.getStoredFactor();
    }

    //Declaring new object instance for AIDL communicaiton
    LoopPerfFactor newFactorPackage;
    //Calling method from the service for retrieving perforation factors
    ScopedAStatus getPerforationFactorResult = g_spMyService->getPerforationFactor(loopId, upperValue, &newFactorPackage);

    //Getting value of perforation factor
    int newFactor;
    newFactorPackage.getPerfFactor(&newFactor);

    //Getting lifespan of perforation factor
    int lifeSpan;
    newFactorPackage.getLifeTime(&lifeSpan);

    //Changing already cached perforation factor details for the loop
    if (itr != prevFactors.end()) {
        itr->second.setStoredFactor(newFactor);
        itr->second.setTimeSec(currentTime.tv_sec);
        itr->second.setTimeUSec(currentTime.tv_usec);
        itr->second.setLifeSpan(lifeSpan);
    }
    //Inserting new perforation factor to cache
    else {
        LoopInfo newInfo;
        newInfo.setLoopId(loopId);
        newInfo.setStoredFactor(newFactor);
        newInfo.setTimeSec(currentTime.tv_sec);
        newInfo.setTimeUSec(currentTime.tv_usec);
        newInfo.setLifeSpan(lifeSpan);
        prevFactors.insert(make_pair(loopId, newInfo));
    }

    //Returning perforation factor
    return newFactor;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_ndkbinderclient_MainActivity_onServiceConnected(
        JNIEnv* env,
        jobject /* this */,
        jobject binder)
{
    //Saving binder to the perforation factor service passed from Main Activity
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
    //Removing (forgetting) binder to the perforation factor service
    g_spMyService = nullptr;

    LOGD("[App] [cpp] onServiceDisconnected");
}
