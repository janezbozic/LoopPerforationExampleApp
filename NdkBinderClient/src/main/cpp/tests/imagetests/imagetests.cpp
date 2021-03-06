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
        }
    }
    else {
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
}

void edgeDetection(AndroidBitmapInfo* infoBase, void* pixelsBase, AndroidBitmapInfo* infoToChange, void* pixelsToChange, bool perf){

    //Sobel matrix
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
            //Previous and next line
            lineBase = (uint32_t *) ((char *) pixelsBase + (yy * infoBase->stride));
            lineToChange = (uint32_t *) ((char *) pixelsToChange + (yy * infoBase->stride));
            uint32_t *prevLineBase = (uint32_t *) ((char *) pixelsBase +
                                                   ((yy - 1) * infoBase->stride));
            uint32_t *nextLineBase = (uint32_t *) ((char *) pixelsBase +
                                                   ((yy + 1) * infoBase->stride));
            for (xx = 0; xx < infoBase->width; xx++) {
                if (xx == 0 || xx == infoBase->height - 1)
                    continue;
                //Sobel operator calculation
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



#define clamp(a,min,max) \
    ({__typeof__ (a) _a__ = (a); \
      __typeof__ (min) _min__ = (min); \
      __typeof__ (max) _max__ = (max); \
      _a__ < _min__ ? _min__ : _a__ > _max__ ? _max__ : _a__; })

// Based heavily on https://github.com/kikoso/android-stackblur

static unsigned short const stackblur_mul[255] =
        {
                512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512,
                454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
                482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456,
                437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
                497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328,
                320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
                446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335,
                329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
                505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405,
                399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
                324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271,
                268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
                451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388,
                385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
                332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292,
                289,287,285,282,280,278,275,273,271,269,267,265,263,261,259
        };

static unsigned char const stackblur_shr[255] =
        {
                9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17,
                17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19,
                19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
                20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
                21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
                21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22,
                22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
                22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23,
                23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
                23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
                23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
                23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
                24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
                24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
                24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
                24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
        };

/// Stackblur algorithm body
void stackblurJob(unsigned char* src,                ///< input image data
                  unsigned int w,                    ///< image width
                  unsigned int h,                    ///< image height
                  unsigned int radius               ///< blur intensity (should be in 2..254 range)
)
{
    unsigned int x, y, xp, yp, i;
    unsigned int sp;
    unsigned int stack_start;
    unsigned char* stack_ptr;

    unsigned char* src_ptr;
    unsigned char* dst_ptr;

    unsigned long sum_r;
    unsigned long sum_g;
    unsigned long sum_b;
    unsigned long sum_in_r;
    unsigned long sum_in_g;
    unsigned long sum_in_b;
    unsigned long sum_out_r;
    unsigned long sum_out_g;
    unsigned long sum_out_b;

    unsigned int wm = w - 1;
    unsigned int hm = h - 1;
    unsigned int w4 = w * 4;
    unsigned int div = (radius * 2) + 1;
    unsigned int mul_sum = stackblur_mul[radius];
    unsigned char shr_sum = stackblur_shr[radius];
    unsigned char stack[div * 3];

    //step 1
    for(y = 0; y < h; y++)
    {
        sum_r = sum_g = sum_b =
        sum_in_r = sum_in_g = sum_in_b =
        sum_out_r = sum_out_g = sum_out_b = 0;

        src_ptr = src + w4 * y; // start of line (0,y)

        for(i = 0; i <= radius; i++)
        {
            stack_ptr    = &stack[ 3 * i ];
            stack_ptr[0] = src_ptr[0];
            stack_ptr[1] = src_ptr[1];
            stack_ptr[2] = src_ptr[2];
            sum_r += src_ptr[0] * (i + 1);
            sum_g += src_ptr[1] * (i + 1);
            sum_b += src_ptr[2] * (i + 1);
            sum_out_r += src_ptr[0];
            sum_out_g += src_ptr[1];
            sum_out_b += src_ptr[2];
        }


        for(i = 1; i <= radius; i++)
        {
            if (i <= wm) src_ptr += 4;
            stack_ptr = &stack[ 3 * (i + radius) ];
            stack_ptr[0] = src_ptr[0];
            stack_ptr[1] = src_ptr[1];
            stack_ptr[2] = src_ptr[2];
            sum_r += src_ptr[0] * (radius + 1 - i);
            sum_g += src_ptr[1] * (radius + 1 - i);
            sum_b += src_ptr[2] * (radius + 1 - i);
            sum_in_r += src_ptr[0];
            sum_in_g += src_ptr[1];
            sum_in_b += src_ptr[2];
        }


        sp = radius;
        xp = radius;
        if (xp > wm) xp = wm;
        src_ptr = src + 4 * (xp + y * w); //   img.pix_ptr(xp, y);
        dst_ptr = src + y * w4; // img.pix_ptr(0, y);
        for(x = 0; x < w; x++)
        {
            int alpha = dst_ptr[3];
            dst_ptr[0] = clamp((sum_r * mul_sum) >> shr_sum, 0, alpha);
            dst_ptr[1] = clamp((sum_g * mul_sum) >> shr_sum, 0, alpha);
            dst_ptr[2] = clamp((sum_b * mul_sum) >> shr_sum, 0, alpha);
            dst_ptr += 4;

            sum_r -= sum_out_r;
            sum_g -= sum_out_g;
            sum_b -= sum_out_b;

            stack_start = sp + div - radius;
            if (stack_start >= div) stack_start -= div;
            stack_ptr = &stack[3 * stack_start];

            sum_out_r -= stack_ptr[0];
            sum_out_g -= stack_ptr[1];
            sum_out_b -= stack_ptr[2];

            if(xp < wm)
            {
                src_ptr += 4;
                ++xp;
            }

            stack_ptr[0] = src_ptr[0];
            stack_ptr[1] = src_ptr[1];
            stack_ptr[2] = src_ptr[2];

            sum_in_r += src_ptr[0];
            sum_in_g += src_ptr[1];
            sum_in_b += src_ptr[2];
            sum_r    += sum_in_r;
            sum_g    += sum_in_g;
            sum_b    += sum_in_b;

            ++sp;
            if (sp >= div) sp = 0;
            stack_ptr = &stack[sp*3];

            sum_out_r += stack_ptr[0];
            sum_out_g += stack_ptr[1];
            sum_out_b += stack_ptr[2];
            sum_in_r  -= stack_ptr[0];
            sum_in_g  -= stack_ptr[1];
            sum_in_b  -= stack_ptr[2];
        }

    }

    //Step 2


    for(x = 0; x < w; x++)
    {
        sum_r =    sum_g =    sum_b =
        sum_in_r = sum_in_g = sum_in_b =
        sum_out_r = sum_out_g = sum_out_b = 0;

        src_ptr = src + 4 * x; // x,0
        for(i = 0; i <= radius; i++)
        {
            stack_ptr    = &stack[i * 3];
            stack_ptr[0] = src_ptr[0];
            stack_ptr[1] = src_ptr[1];
            stack_ptr[2] = src_ptr[2];
            sum_r           += src_ptr[0] * (i + 1);
            sum_g           += src_ptr[1] * (i + 1);
            sum_b           += src_ptr[2] * (i + 1);
            sum_out_r       += src_ptr[0];
            sum_out_g       += src_ptr[1];
            sum_out_b       += src_ptr[2];
        }
        for(i = 1; i <= radius; i++)
        {
            if(i <= hm) src_ptr += w4; // +stride

            stack_ptr = &stack[3 * (i + radius)];
            stack_ptr[0] = src_ptr[0];
            stack_ptr[1] = src_ptr[1];
            stack_ptr[2] = src_ptr[2];
            sum_r += src_ptr[0] * (radius + 1 - i);
            sum_g += src_ptr[1] * (radius + 1 - i);
            sum_b += src_ptr[2] * (radius + 1 - i);
            sum_in_r += src_ptr[0];
            sum_in_g += src_ptr[1];
            sum_in_b += src_ptr[2];
        }

        sp = radius;
        yp = radius;
        if (yp > hm) yp = hm;
        src_ptr = src + 4 * (x + yp * w); // img.pix_ptr(x, yp);
        dst_ptr = src + 4 * x;               // img.pix_ptr(x, 0);
        for(y = 0; y < h; y++)
        {
            int alpha = dst_ptr[3];
            dst_ptr[0] = clamp((sum_r * mul_sum) >> shr_sum, 0, alpha);
            dst_ptr[1] = clamp((sum_g * mul_sum) >> shr_sum, 0, alpha);
            dst_ptr[2] = clamp((sum_b * mul_sum) >> shr_sum, 0, alpha);
            dst_ptr += w4;

            sum_r -= sum_out_r;
            sum_g -= sum_out_g;
            sum_b -= sum_out_b;

            stack_start = sp + div - radius;
            if(stack_start >= div) stack_start -= div;
            stack_ptr = &stack[3 * stack_start];

            sum_out_r -= stack_ptr[0];
            sum_out_g -= stack_ptr[1];
            sum_out_b -= stack_ptr[2];

            if(yp < hm)
            {
                src_ptr += w4; // stride
                ++yp;
            }

            stack_ptr[0] = src_ptr[0];
            stack_ptr[1] = src_ptr[1];
            stack_ptr[2] = src_ptr[2];

            sum_in_r += src_ptr[0];
            sum_in_g += src_ptr[1];
            sum_in_b += src_ptr[2];
            sum_r    += sum_in_r;
            sum_g    += sum_in_g;
            sum_b    += sum_in_b;

            ++sp;
            if (sp >= div) sp = 0;
            stack_ptr = &stack[sp*3];

            sum_out_r += stack_ptr[0];
            sum_out_g += stack_ptr[1];
            sum_out_b += stack_ptr[2];
            sum_in_r  -= stack_ptr[0];
            sum_in_g  -= stack_ptr[1];
            sum_in_b  -= stack_ptr[2];
        }
    }
}

void stackblurJobPerforated(unsigned char* src,                ///< input image data
                  unsigned int w,                    ///< image width
                  unsigned int h,                    ///< image height
                  unsigned int radius               ///< blur intensity (should be in 2..254 range)
)
{
    unsigned int x, y, xp, yp, i;
    unsigned int sp;
    unsigned int stack_start;
    unsigned char* stack_ptr;

    unsigned char* src_ptr;
    unsigned char* dst_ptr;

    unsigned long sum_r;
    unsigned long sum_g;
    unsigned long sum_b;
    unsigned long sum_in_r;
    unsigned long sum_in_g;
    unsigned long sum_in_b;
    unsigned long sum_out_r;
    unsigned long sum_out_g;
    unsigned long sum_out_b;

    unsigned int wm = w - 1;
    unsigned int hm = h - 1;
    unsigned int w4 = w * 4;
    unsigned int div = (radius * 2) + 1;
    unsigned int mul_sum = stackblur_mul[radius];
    unsigned char shr_sum = stackblur_shr[radius];
    unsigned char stack[div * 3];

#pragma clang loop perforate (enable)
    for(y = 0; y < h; y++)
    {
        sum_r = sum_g = sum_b =
        sum_in_r = sum_in_g = sum_in_b =
        sum_out_r = sum_out_g = sum_out_b = 0;

        src_ptr = src + w4 * y; // start of line (0,y)

        for(i = 0; i <= radius; i++)
        {
            stack_ptr    = &stack[ 3 * i ];
            stack_ptr[0] = src_ptr[0];
            stack_ptr[1] = src_ptr[1];
            stack_ptr[2] = src_ptr[2];
            sum_r += src_ptr[0] * (i + 1);
            sum_g += src_ptr[1] * (i + 1);
            sum_b += src_ptr[2] * (i + 1);
            sum_out_r += src_ptr[0];
            sum_out_g += src_ptr[1];
            sum_out_b += src_ptr[2];
        }


        for(i = 1; i <= radius; i++)
        {
            if (i <= wm) src_ptr += 4;
            stack_ptr = &stack[ 3 * (i + radius) ];
            stack_ptr[0] = src_ptr[0];
            stack_ptr[1] = src_ptr[1];
            stack_ptr[2] = src_ptr[2];
            sum_r += src_ptr[0] * (radius + 1 - i);
            sum_g += src_ptr[1] * (radius + 1 - i);
            sum_b += src_ptr[2] * (radius + 1 - i);
            sum_in_r += src_ptr[0];
            sum_in_g += src_ptr[1];
            sum_in_b += src_ptr[2];
        }


        sp = radius;
        xp = radius;
        if (xp > wm) xp = wm;
        src_ptr = src + 4 * (xp + y * w); //   img.pix_ptr(xp, y);
        dst_ptr = src + y * w4; // img.pix_ptr(0, y);
        for(x = 0; x < w; x++)
        {
            int alpha = dst_ptr[3];
            dst_ptr[0] = clamp((sum_r * mul_sum) >> shr_sum, 0, alpha);
            dst_ptr[1] = clamp((sum_g * mul_sum) >> shr_sum, 0, alpha);
            dst_ptr[2] = clamp((sum_b * mul_sum) >> shr_sum, 0, alpha);
            dst_ptr += 4;

            sum_r -= sum_out_r;
            sum_g -= sum_out_g;
            sum_b -= sum_out_b;

            stack_start = sp + div - radius;
            if (stack_start >= div) stack_start -= div;
            stack_ptr = &stack[3 * stack_start];

            sum_out_r -= stack_ptr[0];
            sum_out_g -= stack_ptr[1];
            sum_out_b -= stack_ptr[2];

            if(xp < wm)
            {
                src_ptr += 4;
                ++xp;
            }

            stack_ptr[0] = src_ptr[0];
            stack_ptr[1] = src_ptr[1];
            stack_ptr[2] = src_ptr[2];

            sum_in_r += src_ptr[0];
            sum_in_g += src_ptr[1];
            sum_in_b += src_ptr[2];
            sum_r    += sum_in_r;
            sum_g    += sum_in_g;
            sum_b    += sum_in_b;

            ++sp;
            if (sp >= div) sp = 0;
            stack_ptr = &stack[sp*3];

            sum_out_r += stack_ptr[0];
            sum_out_g += stack_ptr[1];
            sum_out_b += stack_ptr[2];
            sum_in_r  -= stack_ptr[0];
            sum_in_g  -= stack_ptr[1];
            sum_in_b  -= stack_ptr[2];
        }

    }

    //Step 2

#pragma clang loop perforate (enable)
    for(x = 0; x < w; x++)
    {
        sum_r =    sum_g =    sum_b =
        sum_in_r = sum_in_g = sum_in_b =
        sum_out_r = sum_out_g = sum_out_b = 0;

        src_ptr = src + 4 * x; // x,0
        for(i = 0; i <= radius; i++)
        {
            stack_ptr    = &stack[i * 3];
            stack_ptr[0] = src_ptr[0];
            stack_ptr[1] = src_ptr[1];
            stack_ptr[2] = src_ptr[2];
            sum_r           += src_ptr[0] * (i + 1);
            sum_g           += src_ptr[1] * (i + 1);
            sum_b           += src_ptr[2] * (i + 1);
            sum_out_r       += src_ptr[0];
            sum_out_g       += src_ptr[1];
            sum_out_b       += src_ptr[2];
        }
        for(i = 1; i <= radius; i++)
        {
            if(i <= hm) src_ptr += w4; // +stride

            stack_ptr = &stack[3 * (i + radius)];
            stack_ptr[0] = src_ptr[0];
            stack_ptr[1] = src_ptr[1];
            stack_ptr[2] = src_ptr[2];
            sum_r += src_ptr[0] * (radius + 1 - i);
            sum_g += src_ptr[1] * (radius + 1 - i);
            sum_b += src_ptr[2] * (radius + 1 - i);
            sum_in_r += src_ptr[0];
            sum_in_g += src_ptr[1];
            sum_in_b += src_ptr[2];
        }

        sp = radius;
        yp = radius;
        if (yp > hm) yp = hm;
        src_ptr = src + 4 * (x + yp * w); // img.pix_ptr(x, yp);
        dst_ptr = src + 4 * x;               // img.pix_ptr(x, 0);
        for(y = 0; y < h; y++)
        {
            int alpha = dst_ptr[3];
            dst_ptr[0] = clamp((sum_r * mul_sum) >> shr_sum, 0, alpha);
            dst_ptr[1] = clamp((sum_g * mul_sum) >> shr_sum, 0, alpha);
            dst_ptr[2] = clamp((sum_b * mul_sum) >> shr_sum, 0, alpha);
            dst_ptr += w4;

            sum_r -= sum_out_r;
            sum_g -= sum_out_g;
            sum_b -= sum_out_b;

            stack_start = sp + div - radius;
            if(stack_start >= div) stack_start -= div;
            stack_ptr = &stack[3 * stack_start];

            sum_out_r -= stack_ptr[0];
            sum_out_g -= stack_ptr[1];
            sum_out_b -= stack_ptr[2];

            if(yp < hm)
            {
                src_ptr += w4; // stride
                ++yp;
            }

            stack_ptr[0] = src_ptr[0];
            stack_ptr[1] = src_ptr[1];
            stack_ptr[2] = src_ptr[2];

            sum_in_r += src_ptr[0];
            sum_in_g += src_ptr[1];
            sum_in_b += src_ptr[2];
            sum_r    += sum_in_r;
            sum_g    += sum_in_g;
            sum_b    += sum_in_b;

            ++sp;
            if (sp >= div) sp = 0;
            stack_ptr = &stack[sp*3];

            sum_out_r += stack_ptr[0];
            sum_out_g += stack_ptr[1];
            sum_out_b += stack_ptr[2];
            sum_in_r  -= stack_ptr[0];
            sum_in_g  -= stack_ptr[1];
            sum_in_b  -= stack_ptr[2];
        }
    }
}