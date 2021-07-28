//
// Created by janez on 27. 07. 21.
//

#include "adjustBrightness.h"

#include "../../perforation/perforation_lib.h"

#include <ctime>
#include <android/log.h>

#include <cstdio>
#include <cstdlib>
#include <cmath>

int rgb_clamp(int value) {
    if(value > 255) {
        return 255;
    }
    if(value < 0) {
        return 0;
    }
    return value;
}

void brightness(AndroidBitmapInfo* info, void* pixels, float brightnessValue, bool perf, int ***pixelsBase){
    int xx, yy, red, green, blue;
    uint32_t* line;

    if (perf) {
        #pragma clang loop perforate (enable)
        for (yy = 0; yy < info->height; yy++) {
            line = (uint32_t *) pixels;
            #pragma clang loop perforate (enable)
            for (xx = 0; xx < info->width; xx++) {

                //extract the RGB values from the pixel
                red = (int) ((line[xx] & 0x00FF0000) >> 16);
                green = (int) ((line[xx] & 0x0000FF00) >> 8);
                blue = (int) (line[xx] & 0x00000FF);

                //manipulate each value
                red = rgb_clamp((int) (red + pixelsBase[yy][xx][0]));
                green = rgb_clamp((int) (green + pixelsBase[yy][xx][1]));
                blue = rgb_clamp((int) (blue + pixelsBase[yy][xx][2]));

                // set the new pixel back in
                line[xx] = (line[xx] & 0xFF000000) |
                           ((red << 16) & 0x00FF0000) |
                           ((green << 8) & 0x0000FF00) |
                           (blue & 0x000000FF);
            }

            pixels = (char *) pixels + info->stride;
        }
    }
    else {
        for (yy = 0; yy < info->height; yy++) {
            line = (uint32_t *) pixels;
            for (xx = 0; xx < info->width; xx++) {

                //extract the RGB values from the pixel
                red = (int) ((line[xx] & 0x00FF0000) >> 16);
                green = (int) ((line[xx] & 0x0000FF00) >> 8);
                blue = (int) (line[xx] & 0x00000FF);

                //manipulate each value
                red = rgb_clamp((int) (red + pixelsBase[yy][xx][0]));
                green = rgb_clamp((int) (green + pixelsBase[yy][xx][1]));
                blue = rgb_clamp((int) (blue + pixelsBase[yy][xx][2]));

                // set the new pixel back in
                line[xx] = (line[xx] & 0xFF000000) |
                           ((red << 16) & 0x00FF0000) |
                           ((green << 8) & 0x0000FF00) |
                           (blue & 0x000000FF);
            }

            pixels = (char *) pixels + info->stride;
        }
    }
}