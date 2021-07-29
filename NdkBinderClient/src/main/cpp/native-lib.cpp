#include <jni.h>
#include <string>
#include <LogDefs.h>

#include "LoopPerf.h"
#include "tests/blackscholes/blackScholes.h"
#include "tests/montecarlo/montecarlo.h"
#include "tests/imagetests/imagetests.h"

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

        hello = "\nNormal: " + std::to_string(normTime) + "s All runs: " +
                            std::to_string(allRunsNorm) + "\nPerforated: " +
                            std::to_string(perfTime) + "s All runs: " + std::to_string(allRunsPerf);
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

int ***base;

extern "C" JNIEXPORT jdouble JNICALL
Java_com_example_ndkbinderclient_MainActivity_brightness(JNIEnv * env, jobject  obj, jobject bitmap, jfloat brightnessValue, jboolean perf, jboolean first)
{

    struct timeval stop, start;

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

    gettimeofday(&start, NULL);
    brightness(&info,pixels, brightnessValue, perf, base);
    gettimeofday(&stop, NULL);

    AndroidBitmap_unlockPixels(env, bitmap);

    double time = ((stop.tv_sec - start.tv_sec)  + (double) (stop.tv_usec - start.tv_usec)/1000000);

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

    struct timeval stop, start;
    gettimeofday(&start, NULL);
    edgeDetection(&infoBase, pixelsBase, &infoToChange, pixelsToChange, perf);
    gettimeofday(&stop, NULL);


    AndroidBitmap_unlockPixels(env, bitmapBase);
    AndroidBitmap_unlockPixels(env, bitmapToChange);

    double time = ((stop.tv_sec - start.tv_sec)  + (double)(stop.tv_usec - start.tv_usec)/1000000);

    return time;

}