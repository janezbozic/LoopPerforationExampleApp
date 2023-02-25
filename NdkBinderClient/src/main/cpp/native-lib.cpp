#include <jni.h>
#include <string>
#include <LogDefs.h>

#include "tests/blackscholes/blackScholes.h"
#include "tests/montecarlo/montecarlo.h"
#include "tests/imagetests/imagetests.h"
#include <unistd.h>

#include <android/asset_manager.h>

using namespace std;

extern "C" JNIEXPORT jobject JNICALL
Java_com_example_ndkbinderclient_MainActivity_talkToService(
        JNIEnv* env,
        jobject /* this */,
        jint testid,
        jboolean perf)
{

    double perfTime = 0;
    int allRunsPerf = 0;

    struct timeval stop, start;

    double normTime = 0;
    int allRunsNorm = 0;

    std::string returnResult;

    //Running test Black-Scholes
    if (testid == 1) {

        int NUM_REP = 100;

        double sum_res = 0;

        for (int i = 0; i < NUM_REP; i++) {

            //Getting start time
            auto start = chrono::steady_clock::now();
            //Running perforated test
            sum_res += startBalckScholes(perf);
            //Getting end time
            auto end = chrono::steady_clock::now();
            //Adding execution time to sum of perforated runs
            perfTime += ((double)chrono::duration_cast<chrono::microseconds>(end - start).count())/1000000;
            //Number of test's main loop execution
            allRunsPerf += getNewCount();
            //Set counter of execution to 0
            resetNewCount();

        }

        perfTime /= NUM_REP;
        sum_res /= NUM_REP;
        allRunsPerf /= NUM_REP;

        jclass cls = (*env).FindClass("com/diploma/loopperforationlibrary/ResultInfo");
        jmethodID midConstructor = (*env).GetMethodID(cls, "<init>", "(DD)V");
        jobject resultInfoObj = (*env).NewObject(cls, midConstructor, perfTime, sum_res);
        return resultInfoObj;
    }
    //Running test Monte-Carlo
    else if (testid == 2){

        double perforatedRuns = 0;

        int NUM_REP = 100;

        for (int i = 0; i < NUM_REP; i++) {

            //Getting start time
            auto start = chrono::steady_clock::now();
            //Running perforated test
            perforatedRuns += runMonteCarlo(perf);
            //Getting end time
            auto end = chrono::steady_clock::now();
            //Adding execution time to sum of non-perforated runs
            perfTime += ((double)chrono::duration_cast<chrono::microseconds>(end - start).count())/1000000;

        }

        perfTime /= NUM_REP;
        perforatedRuns /= NUM_REP;

        jclass cls = (*env).FindClass("com/diploma/loopperforationlibrary/ResultInfo");
        jmethodID midConstructor = (*env).GetMethodID(cls, "<init>", "(DD)V");
        jobject resultInfoObj = (*env).NewObject(cls, midConstructor, perfTime, perforatedRuns);
        return resultInfoObj;
    }

    //Returning result
    return env->NewStringUTF(returnResult.c_str());
}

int ***base;

extern "C" JNIEXPORT jdouble JNICALL
Java_com_example_ndkbinderclient_MainActivity_brightness(JNIEnv * env, jobject  obj, jobject bitmap, jfloat brightnessValue, jboolean perf, jboolean first)
{

    //Bitmap setup (setting pixel map, so we can iterate and change)
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
    //End of bitmap setup

    //Saving increment for every pixel of the picture to a table for later
    //  This is done only the first time
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

    //Getting start time
    auto start = chrono::steady_clock::now();
    //Running the test (true/false for perforation)
    brightness(&info,pixels, brightnessValue, perf, base);
    //Getting end time
    auto end = chrono::steady_clock::now();

    //Unlocking picture from memory
    AndroidBitmap_unlockPixels(env, bitmap);

    //Calculating time
    double time = ((double)chrono::duration_cast<chrono::microseconds>(end - start).count())/1000000;

    //Returning time needed for the test run
    return time;

}

extern "C" JNIEXPORT jdouble JNICALL
Java_com_example_ndkbinderclient_MainActivity_edgeDetection(JNIEnv * env,
                                                            jobject  obj,
                                                            jobject bitmapBase,
                                                            jobject bitmapToChange,
                                                            jboolean perf) {

    //Bitmap setup (setting pixel map, so we can iterate and change)
    // We need two, because one is changing (result), the other one is input
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
    //End of bitmap setup

    //Getting start time
    auto start = chrono::steady_clock::now();
    //Running the test
    edgeDetection(&infoBase, pixelsBase, &infoToChange, pixelsToChange, perf);
    //Getting end time
    auto end = chrono::steady_clock::now();

    //Unlocking pictures (bitmaps) from memory
    AndroidBitmap_unlockPixels(env, bitmapBase);
    AndroidBitmap_unlockPixels(env, bitmapToChange);

    //Calculating time
    double time = ((double)chrono::duration_cast<chrono::microseconds>(end - start).count())/1000000;

    //Returning time needed for test run
    return time;

}

extern "C" JNIEXPORT jdouble JNICALL
Java_com_example_ndkbinderclient_MainActivity_blur(JNIEnv * env, jobject  obj, jobject bitmap, jint radious, jboolean perf)
{
    //Bitmap setup (setting pixel map, so we can iterate and change)
    AndroidBitmapInfo  infoBase;
    int retBase;
    void* pixelsBase;

    if ((retBase = AndroidBitmap_getInfo(env, bitmap, &infoBase)) < 0) {
        std::string s = "AndroidBitmap_getInfo() failed ! error=" + std::to_string(retBase);
        return 0;
    }
    if (infoBase.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        std::string s = "Bitmap format is not RGBA_8888 !";
        return 0;
    }

    if ((retBase = AndroidBitmap_lockPixels(env, bitmap, &pixelsBase)) < 0) {
        std::string s = "AndroidBitmap_lockPixels() failed ! error=" + std::to_string(retBase);
    }
    //End of bitmap setup

    //Getting start time
    auto start = chrono::steady_clock::now();
    //Running the test
    if (perf){
        stackblurJobPerforated((unsigned char*)pixelsBase, infoBase.width, infoBase.height, radious);
    }
    else
        stackblurJob((unsigned char*)pixelsBase, infoBase.width, infoBase.height, radious);
    //Getting end time
    auto end = chrono::steady_clock::now();

    //Unlocking pictures (bitmaps) from memory
    AndroidBitmap_unlockPixels(env, bitmap);

    //Calculating time
    double time = ((double)chrono::duration_cast<chrono::microseconds>(end - start).count())/1000000;

    //Returning time needed for test run
    return time;
}

extern "C" JNIEXPORT jdouble JNICALL
Java_com_example_ndkbinderclient_MainActivity_removeNoise(JNIEnv * env, jobject  obj, jobject bitmap, jboolean perf)
{
    //Bitmap setup (setting pixel map, so we can iterate and change)
    AndroidBitmapInfo  infoBase;
    int retBase;
    void* pixelsBase;

    if ((retBase = AndroidBitmap_getInfo(env, bitmap, &infoBase)) < 0) {
        std::string s = "AndroidBitmap_getInfo() failed ! error=" + std::to_string(retBase);
        return 0;
    }
    if (infoBase.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        std::string s = "Bitmap format is not RGBA_8888 !";
        return 0;
    }

    if ((retBase = AndroidBitmap_lockPixels(env, bitmap, &pixelsBase)) < 0) {
        std::string s = "AndroidBitmap_lockPixels() failed ! error=" + std::to_string(retBase);
    }
    //End of bitmap setup

    //Getting start time
    auto start = chrono::steady_clock::now();
    //Running the test
    edgeDetection(&infoBase, pixelsBase, &infoBase, pixelsBase, perf);
    //Getting end time
    auto end = chrono::steady_clock::now();

    //Unlocking pictures (bitmaps) from memory
    AndroidBitmap_unlockPixels(env, bitmap);

    //Calculating time
    double time = ((double)chrono::duration_cast<chrono::microseconds>(end - start).count())/1000000;

    //Returning time needed for test run
    return time;
}