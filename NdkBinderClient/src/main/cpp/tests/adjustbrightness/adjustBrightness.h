//
// Created by janez on 27. 07. 21.
//

#ifndef ANDROIDNDKBINDEREXAMPLES_ADJUSTBRIGHTNESS_H
#define ANDROIDNDKBINDEREXAMPLES_ADJUSTBRIGHTNESS_H

#include <android/bitmap.h>

void brightness(AndroidBitmapInfo* info, void* pixels, float brightnessValue, bool perf, int ***pixelsBase);


#endif //ANDROIDNDKBINDEREXAMPLES_ADJUSTBRIGHTNESS_H
