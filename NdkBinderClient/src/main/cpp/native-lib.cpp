#include <jni.h>
#include <string>

#include "tests/imagetests/imagetests.h"

using namespace std;

extern "C" JNIEXPORT jdouble JNICALL
Java_com_example_ndkbinderclient_MainActivity_brightness(JNIEnv * env, jobject  obj, jobject bitmap, jfloat brightnessValue, jboolean perf)
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

    //Getting start time
    auto start = chrono::steady_clock::now();
    //Running the test (true/false for perforation)
    brightness(&info,pixels, brightnessValue, perf);
    //Getting end time
    auto end = chrono::steady_clock::now();

    //Unlocking picture from memory
    AndroidBitmap_unlockPixels(env, bitmap);

    //Calculating time
    double time = ((double)chrono::duration_cast<chrono::microseconds>(end - start).count())/1000000;

    //Returning time needed for the test run
    return time;

}