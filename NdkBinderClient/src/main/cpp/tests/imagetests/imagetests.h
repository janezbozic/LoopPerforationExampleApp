//
// Created by janez on 27. 07. 21.
//

#ifndef ANDROIDNDKBINDEREXAMPLES_IMAGETESTS_H
#define ANDROIDNDKBINDEREXAMPLES_IMAGETESTS_H

#include <android/bitmap.h>

void brightness(AndroidBitmapInfo* info, void* pixels, float brightnessValue,
                bool perf, int ***pixelsBase);
void edgeDetection(AndroidBitmapInfo* infoBase, void* pixelsBase, AndroidBitmapInfo* infoToChange,
                   void* pixelsToChange, bool perf);

#endif //ANDROIDNDKBINDEREXAMPLES_IMAGETESTS_H
