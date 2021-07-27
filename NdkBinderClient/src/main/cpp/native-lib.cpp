#include <jni.h>
#include <string>
#include <LogDefs.h>

#include "LoopPerf.h"
#include "tests/blackscholes/blackScholes.h"
#include "tests/montecarlo/montecarlo.h"

#include <sys/time.h>

#include <android/asset_manager.h>

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_ndkbinderclient_MainActivity_talkToService(
        JNIEnv* env,
        jobject /* this */,
        jint testid)
{

    double perfTime = 0;
    int allRunsPerf = 0;

    struct timeval stop, start;

    double normTime = 0;
    int allRunsNorm = 0;

    std::string hello;

    if (testid == 1) {

        for (int i = 0; i < 10000; i++) {

            gettimeofday(&start, NULL);
            startBalckScholes(false);
            gettimeofday(&stop, NULL);
            normTime += ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);
            allRunsNorm += getNewCount();
            resetNewCount();

            gettimeofday(&start, NULL);
            startBalckScholes(true);
            gettimeofday(&stop, NULL);
            perfTime += ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);
            allRunsPerf += getNewCount();
            resetNewCount();
        }

        normTime /= 1000000;
        perfTime /= 1000000;

        hello = "\nNormal: " + std::to_string(normTime) + " All runs: " +
                            std::to_string(allRunsNorm) + "\nPerforated: " +
                            std::to_string(perfTime) + " All runs: " + std::to_string(allRunsPerf);
    }
    else if (testid == 2){

        double perforatedRuns = 0;
        double normalRuns = 0;

        for (int i = 0; i < 1000; i++) {

            gettimeofday(&start, NULL);
            normalRuns += runMonteCarlo(false);
            gettimeofday(&stop, NULL);
            normTime += ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);

            gettimeofday(&start, NULL);
            perforatedRuns += runMonteCarlo(true);
            gettimeofday(&stop, NULL);
            perfTime += ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);
        }

        double diffPercent = (abs(normalRuns - perforatedRuns)/normalRuns)*100;

        normTime /= 1000000;
        perfTime /= 1000000;

        hello = "\nNormal: " + std::to_string(normTime) + "s\nPerforated: " +
                std::to_string(perfTime) + "s\nAvg. Difference: " + std::to_string(diffPercent) + "%" +
                "\nDifference value: " + std::to_string(abs(normalRuns - perforatedRuns)/1000);
    }
    return env->NewStringUTF(hello.c_str());
}
