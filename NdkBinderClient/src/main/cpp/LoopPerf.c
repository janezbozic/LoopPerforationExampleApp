//
// Created by janez on 2.4.2021.
//
#include <android/log.h>
#include "perforation/perforation_lib.h"

#define  LOG_TAG    "c-debug"

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

int PERF_fun(){
    int c;
    #pragma clang loop perforate (enable)
    for (c = 0; c<40; c++){
        #pragma clang loop perforate (enable)
        for (int j = 0; j<40; j++){
            LOGD("%d\n", j);
        }
        LOGD("main loop: %d\n", c);
    }
    LOGD("EXIT: %d\n", c);
    return c;
}