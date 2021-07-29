//
// Created by janez on 27. 07. 21.
//

#include "imagetests.h"

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

int redFun(u_int32_t val){
    return (int) ((val & 0x00FF0000) >> 16);
}

int greenFun(u_int32_t val){
    return (int) ((val & 0x0000FF00) >> 8);
}

int blueFun(u_int32_t val){
    return (int) (val & 0x00000FF);
}

void brightness(AndroidBitmapInfo* info, void* pixels, float brightnessValue, bool perf, int ***pixelsBase){
    int xx, yy, red, green, blue;
    uint32_t* line;

    if (perf) {
        #pragma clang loop perforate (enable)
        for (yy = 0; yy < info->height; yy++) {
            line = (uint32_t *) ((char*)pixels+(yy*info->stride));
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
        }
    }
    else {
        for (yy = 0; yy < info->height; yy++) {
            line = (uint32_t *) ((char*)pixels+(yy*info->stride));
            for (xx = 0; xx < info->width; xx++) {

                //extract the RGB values from the pixel
                red = redFun(line[xx]);
                green = greenFun(line[xx]);
                blue = blueFun(line[xx]);

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
        }
    }
}

void edgeDetection(AndroidBitmapInfo* infoBase, void* pixelsBase, AndroidBitmapInfo* infoToChange, void* pixelsToChange, bool perf){

    int kx[3][3] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    int ky[3][3] = {1, 2, 1, 0, 0, 0, -1, -2, -1};

    int xx, yy;
    uint32_t* lineBase;
    uint32_t* lineToChange;

    if (perf) {
        #pragma clang loop perforate (enable)
        for (yy = 0; yy < infoBase->height; yy++) {
            if (yy == 0 || yy == infoBase->height - 1)
                continue;
            lineBase = (uint32_t *) ((char *) pixelsBase + (yy * infoBase->stride));
            lineToChange = (uint32_t *) ((char *) pixelsToChange + (yy * infoBase->stride));
            uint32_t *prevLineBase = (uint32_t *) ((char *) pixelsBase +
                                                   ((yy - 1) * infoBase->stride));
            uint32_t *nextLineBase = (uint32_t *) ((char *) pixelsBase +
                                                   ((yy + 1) * infoBase->stride));
            #pragma clang loop perforate (enable)
            for (xx = 0; xx < infoBase->width; xx++) {
                if (xx == 0 || xx == infoBase->height - 1)
                    continue;
                int a = (redFun(prevLineBase[xx - 1]) + blueFun(prevLineBase[xx - 1])
                         + greenFun(prevLineBase[xx - 1])) / 3;
                int b = (redFun(prevLineBase[xx]) + blueFun(prevLineBase[xx])
                         + greenFun(prevLineBase[xx])) / 3;
                int c = (redFun(prevLineBase[xx + 1]) + blueFun(prevLineBase[xx + 1])
                         + greenFun(prevLineBase[xx + 1])) / 3;

                int d = (blueFun(lineBase[xx - 1]) + greenFun(lineBase[xx - 1])
                         + redFun(lineBase[xx - 1])) / 3;
                int e = (blueFun(lineBase[xx]) + greenFun(lineBase[xx])
                         + redFun(lineBase[xx])) / 3;
                int f = (blueFun(lineBase[xx + 1]) + greenFun(lineBase[xx + 1])
                         + redFun(lineBase[xx + 1])) / 3;

                int g = (greenFun(nextLineBase[xx - 1]) + redFun(nextLineBase[xx - 1])
                         + blueFun(nextLineBase[xx - 1])) / 3;
                int h = (greenFun(nextLineBase[xx]) + redFun(nextLineBase[xx])
                         + blueFun(nextLineBase[xx])) / 3;
                int i = (greenFun(nextLineBase[xx + 1]) + redFun(nextLineBase[xx + 1])
                         + blueFun(nextLineBase[xx + 1])) / 3;

                int matrix[3][3] = {a, b, c, d, e, f, g, h, i};

                int sumx = 0;
                int sumy = 0;

                for (int s = 0; s < 3; s++) {
                    for (int t = 0; t < 3; t++) {
                        sumx = sumx + (matrix[s][t] * kx[s][t]);
                        sumy = sumy + (matrix[s][t] * ky[s][t]); /* use ky, not kx */
                    }
                }

                int newValue = rgb_clamp(sqrt(pow(sumx, 2) + pow(sumy, 2)));

                // set the new pixel back in
                lineToChange[xx] = (lineBase[xx] & 0xFF000000) |
                                   ((newValue << 16) & 0x00FF0000) |
                                   ((newValue << 8) & 0x0000FF00) |
                                   (newValue & 0x000000FF);
            }
        }
    }
    else {
        for (yy = 0; yy < infoBase->height; yy++) {
            if (yy == 0 || yy == infoBase->height - 1)
                continue;
            lineBase = (uint32_t *) ((char *) pixelsBase + (yy * infoBase->stride));
            lineToChange = (uint32_t *) ((char *) pixelsToChange + (yy * infoBase->stride));
            uint32_t *prevLineBase = (uint32_t *) ((char *) pixelsBase +
                                                   ((yy - 1) * infoBase->stride));
            uint32_t *nextLineBase = (uint32_t *) ((char *) pixelsBase +
                                                   ((yy + 1) * infoBase->stride));
            for (xx = 0; xx < infoBase->width; xx++) {
                if (xx == 0 || xx == infoBase->height - 1)
                    continue;
                int a = (redFun(prevLineBase[xx - 1]) + blueFun(prevLineBase[xx - 1])
                         + greenFun(prevLineBase[xx - 1])) / 3;
                int b = (redFun(prevLineBase[xx]) + blueFun(prevLineBase[xx])
                         + greenFun(prevLineBase[xx])) / 3;
                int c = (redFun(prevLineBase[xx + 1]) + blueFun(prevLineBase[xx + 1])
                         + greenFun(prevLineBase[xx + 1])) / 3;

                int d = (blueFun(lineBase[xx - 1]) + greenFun(lineBase[xx - 1])
                         + redFun(lineBase[xx - 1])) / 3;
                int e = (blueFun(lineBase[xx]) + greenFun(lineBase[xx])
                         + redFun(lineBase[xx])) / 3;
                int f = (blueFun(lineBase[xx + 1]) + greenFun(lineBase[xx + 1])
                         + redFun(lineBase[xx + 1])) / 3;

                int g = (greenFun(nextLineBase[xx - 1]) + redFun(nextLineBase[xx - 1])
                         + blueFun(nextLineBase[xx - 1])) / 3;
                int h = (greenFun(nextLineBase[xx]) + redFun(nextLineBase[xx])
                         + blueFun(nextLineBase[xx])) / 3;
                int i = (greenFun(nextLineBase[xx + 1]) + redFun(nextLineBase[xx + 1])
                         + blueFun(nextLineBase[xx + 1])) / 3;

                int matrix[3][3] = {a, b, c, d, e, f, g, h, i};

                int sumx = 0;
                int sumy = 0;

                for (int s = 0; s < 3; s++) {
                    for (int t = 0; t < 3; t++) {
                        sumx = sumx + (matrix[s][t] * kx[s][t]);
                        sumy = sumy + (matrix[s][t] * ky[s][t]); /* use ky, not kx */
                    }
                }

                int newValue = rgb_clamp(sqrt(pow(sumx, 2) + pow(sumy, 2)));

                // set the new pixel back in
                lineToChange[xx] = (lineBase[xx] & 0xFF000000) |
                                   ((newValue << 16) & 0x00FF0000) |
                                   ((newValue << 8) & 0x0000FF00) |
                                   (newValue & 0x000000FF);
            }
        }
    }
}