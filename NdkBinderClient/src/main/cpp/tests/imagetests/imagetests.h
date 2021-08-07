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

void stackblurJob(unsigned char* src,                ///< input image data
             unsigned int w,                    ///< image width
             unsigned int h,                    ///< image height
             unsigned int radius               ///< blur intensity (should be in 2..254 range)
);

void stackblurJobPerforated(unsigned char* src,                ///< input image data
                  unsigned int w,                    ///< image width
                  unsigned int h,                    ///< image height
                  unsigned int radius               ///< blur intensity (should be in 2..254 range)
);

#endif //ANDROIDNDKBINDEREXAMPLES_IMAGETESTS_H
