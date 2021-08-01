#include <jni.h>
#include <string>
#include <LogDefs.h>

#include "LoopPerf.h"
#include "tests/blackscholes/blackScholes.h"
#include "tests/montecarlo/montecarlo.h"
#include "tests/imagetests/imagetests.h"
#include <unistd.h>

#include <android/asset_manager.h>

using namespace std;

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

        int NUM_REP = 100;

        for (int i = 0; i < NUM_REP; i++) {

            auto start = chrono::steady_clock::now();
            startBalckScholes(false);
            auto end = chrono::steady_clock::now();
            normTime += ((double)chrono::duration_cast<chrono::microseconds>(end - start).count())/1000000;
            allRunsNorm += getNewCount();
            resetNewCount();

            start = chrono::steady_clock::now();
            startBalckScholes(true);
            end = chrono::steady_clock::now();
            perfTime += ((double)chrono::duration_cast<chrono::microseconds>(end - start).count())/1000000;
            allRunsPerf += getNewCount();
            resetNewCount();

            sleep(1);
        }

        normTime /= NUM_REP;
        perfTime /= NUM_REP;

        allRunsPerf /= NUM_REP;
        allRunsNorm /= NUM_REP;

        hello = "Average of " + std::to_string(NUM_REP) + " tests of 1000 runs of test's main loop:" +
                            "\n\tNormal: " + std::to_string(normTime) + "s All runs: " +
                            std::to_string(allRunsNorm) + "\n\tPerforated: " +
                            std::to_string(perfTime) + "s All runs: " + std::to_string(allRunsPerf);
    }
    else if (testid == 2){

        double perforatedRuns = 0;
        double normalRuns = 0;

        int NUM_REP = 100;

        for (int i = 0; i < NUM_REP; i++) {

            auto start = chrono::steady_clock::now();
            normalRuns += runMonteCarlo(false);
            auto end = chrono::steady_clock::now();
            normTime += ((double)chrono::duration_cast<chrono::microseconds>(end - start).count())/1000000;

            start = chrono::steady_clock::now();
            perforatedRuns += runMonteCarlo(true);
            end = chrono::steady_clock::now();
            perfTime += ((double)chrono::duration_cast<chrono::microseconds>(end - start).count())/1000000;

            sleep(1);
        }

        double diffPercent = (abs(normalRuns - perforatedRuns)/normalRuns)*100;

        normTime /= NUM_REP;
        perfTime /= NUM_REP;

        perforatedRuns /= NUM_REP;
        normalRuns /= NUM_REP;

        hello = "Average of " + std::to_string(NUM_REP) + " tests of 128*(4^5) runs of test's main loop:" +
                "\n\tNormal: " + std::to_string(normTime) + "s\n\tPerforated: " +
                std::to_string(perfTime) + "s\n\tAvg. Difference: " + std::to_string(diffPercent) + "%" +
                "\n\tDifference value: " + std::to_string(abs(normalRuns - perforatedRuns));
    }
    return env->NewStringUTF(hello.c_str());
}

int ***base;

extern "C" JNIEXPORT jdouble JNICALL
Java_com_example_ndkbinderclient_MainActivity_brightness(JNIEnv * env, jobject  obj, jobject bitmap, jfloat brightnessValue, jboolean perf, jboolean first)
{

    AndroidBitmapInfo  info;
    int ret;
    void* pixels;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        std::string s = "AndroidBitmap_getInfo() failed ! error=" + std::to_string(ret);
        return 0;
    }
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        std::string s = "Bitmap format is not RGBA_8888 !";
        return 0;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        std::string s = "AndroidBitmap_lockPixels() failed ! error=" + std::to_string(ret);
    }

    if (first) {
        void *pixelBase = pixels;
        base = new int**[info.height];
        uint32_t* line;
        int red, green, blue;
        for (int i = 0; i < info.height; i++) {
            base[i] = new int*[info.width];
            line = (uint32_t *) pixelBase;
            for (int j = 0; j < info.width; j++) {
                red = (int) ((line[j] & 0x00FF0000) >> 16);
                green = (int) ((line[j] & 0x0000FF00) >> 8);
                blue = (int) (line[j] & 0x00000FF);

                base[i][j] = new int [3];

                base[i][j][0] = red * brightnessValue;
                base[i][j][1] = green * brightnessValue;
                base[i][j][2] = blue * brightnessValue;
            }
            pixelBase = (char *) pixelBase + info.stride;
        }
    }

    auto start = chrono::steady_clock::now();
    brightness(&info,pixels, brightnessValue, perf, base);
    auto end = chrono::steady_clock::now();

    AndroidBitmap_unlockPixels(env, bitmap);

    double time = ((double)chrono::duration_cast<chrono::microseconds>(end - start).count())/1000000;

    return time;

}

extern "C" JNIEXPORT jdouble JNICALL
Java_com_example_ndkbinderclient_MainActivity_edgeDetection(JNIEnv * env,
                                                            jobject  obj,
                                                            jobject bitmapBase,
                                                            jobject bitmapToChange,
                                                            jboolean perf) {

    AndroidBitmapInfo  infoBase;
    int retBase;
    void* pixelsBase;

    if ((retBase = AndroidBitmap_getInfo(env, bitmapBase, &infoBase)) < 0) {
        std::string s = "AndroidBitmap_getInfo() failed ! error=" + std::to_string(retBase);
        return 0;
    }
    if (infoBase.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        std::string s = "Bitmap format is not RGBA_8888 !";
        return 0;
    }

    if ((retBase = AndroidBitmap_lockPixels(env, bitmapBase, &pixelsBase)) < 0) {
        std::string s = "AndroidBitmap_lockPixels() failed ! error=" + std::to_string(retBase);
    }

    AndroidBitmapInfo  infoToChange;
    int retToChange;
    void* pixelsToChange;

    if ((retToChange = AndroidBitmap_getInfo(env, bitmapToChange, &infoToChange)) < 0) {
        std::string s = "AndroidBitmap_getInfo() failed ! error=" + std::to_string(retToChange);
        return 0;
    }
    if (infoToChange.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        std::string s = "Bitmap format is not RGBA_8888 !";
        return 0;
    }

    if ((retToChange = AndroidBitmap_lockPixels(env, bitmapToChange, &pixelsToChange)) < 0) {
        std::string s = "AndroidBitmap_lockPixels() failed ! error=" + std::to_string(retToChange);
    }

    auto start = chrono::steady_clock::now();
    edgeDetection(&infoBase, pixelsBase, &infoToChange, pixelsToChange, perf);
    auto end = chrono::steady_clock::now();


    AndroidBitmap_unlockPixels(env, bitmapBase);
    AndroidBitmap_unlockPixels(env, bitmapToChange);

    double time = ((double)chrono::duration_cast<chrono::microseconds>(end - start).count())/1000000;

    return time;

}