//
// Created by janez on 27. 07. 21.
//

#ifndef ANDROIDNDKBINDEREXAMPLES_IMAGETESTS_H
#define ANDROIDNDKBINDEREXAMPLES_IMAGETESTS_H

#include <android/bitmap.h>

void brightness(AndroidBitmapInfo* info, void* pixels, float brightnessValue,
                bool perf);

#endif //ANDROIDNDKBINDEREXAMPLES_IMAGETESTS_H
