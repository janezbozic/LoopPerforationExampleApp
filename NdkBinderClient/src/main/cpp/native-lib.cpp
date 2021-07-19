#include <jni.h>
#include <string>
#include <LogDefs.h>

#include "LoopPerf.h"

using namespace std;

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_ndkbinderclient_MainActivity_talkToService(
        JNIEnv* env,
        jobject /* this */)
{
    std::string hello = std::to_string(PERF_fun());
    return env->NewStringUTF(hello.c_str());
}
