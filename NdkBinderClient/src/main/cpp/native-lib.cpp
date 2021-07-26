#include <jni.h>
#include <string>
#include <LogDefs.h>

#include "LoopPerf.h"

#include <sys/time.h>

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include <android/asset_manager.h>

using namespace std;

#define fptype float

#define NUM_RUNS 100

typedef struct OptionData_ {
    fptype s;          // spot price
    fptype strike;     // strike price
    fptype r;          // risk-free interest rate
    fptype divq;       // dividend rate
    fptype v;          // volatility
    fptype t;          // time to maturity or option expiration in years
    //     (1yr = 1.0, 6mos = 0.5, 3mos = 0.25, ..., etc)
    char OptionType;   // Option type.  "P"=PUT, "C"=CALL
    fptype divs;       // dividend vals (not used in this test)
    fptype DGrefval;   // DerivaGem Reference Value
} OptionData;

OptionData *dataL;
fptype *prices;
int numOptions;

int    * otype;
fptype * sptprice;
fptype * strike;
fptype * rate;
fptype * volatility;
fptype * otime;
int numError = 0;
int nThreads;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Cumulative Normal Distribution Function
// See Hull, Section 11.8, P.243-244
#define inv_sqrt_2xPI 0.39894228040143270286

fptype CNDF ( fptype InputX )
{
    int sign;

    fptype OutputX;
    fptype xInput;
    fptype xNPrimeofX;
    fptype expValues;
    fptype xK2;
    fptype xK2_2, xK2_3;
    fptype xK2_4, xK2_5;
    fptype xLocal, xLocal_1;
    fptype xLocal_2, xLocal_3;

    // Check for negative value of InputX
    if (InputX < 0.0) {
        InputX = -InputX;
        sign = 1;
    } else
        sign = 0;

    xInput = InputX;

    // Compute NPrimeX term common to both four & six decimal accuracy calcs
    expValues = exp(-0.5f * InputX * InputX);
    xNPrimeofX = expValues;
    xNPrimeofX = xNPrimeofX * inv_sqrt_2xPI;

    xK2 = 0.2316419 * xInput;
    xK2 = 1.0 + xK2;
    xK2 = 1.0 / xK2;
    xK2_2 = xK2 * xK2;
    xK2_3 = xK2_2 * xK2;
    xK2_4 = xK2_3 * xK2;
    xK2_5 = xK2_4 * xK2;

    xLocal_1 = xK2 * 0.319381530;
    xLocal_2 = xK2_2 * (-0.356563782);
    xLocal_3 = xK2_3 * 1.781477937;
    xLocal_2 = xLocal_2 + xLocal_3;
    xLocal_3 = xK2_4 * (-1.821255978);
    xLocal_2 = xLocal_2 + xLocal_3;
    xLocal_3 = xK2_5 * 1.330274429;
    xLocal_2 = xLocal_2 + xLocal_3;

    xLocal_1 = xLocal_2 + xLocal_1;
    xLocal   = xLocal_1 * xNPrimeofX;
    xLocal   = 1.0 - xLocal;

    OutputX  = xLocal;

    if (sign) {
        OutputX = 1.0 - OutputX;
    }

    return OutputX;
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
fptype BlkSchlsEqEuroNoDiv( fptype sptprice,
                            fptype strike, fptype rate, fptype volatility,
                            fptype time, int otype, float timet )
{
    fptype OptionPrice;

    // local private working variables for the calculation
    fptype xStockPrice;
    fptype xStrikePrice;
    fptype xRiskFreeRate;
    fptype xVolatility;
    fptype xTime;
    fptype xSqrtTime;

    fptype logValues;
    fptype xLogTerm;
    fptype xD1;
    fptype xD2;
    fptype xPowerTerm;
    fptype xDen;
    fptype d1;
    fptype d2;
    fptype FutureValueX;
    fptype NofXd1;
    fptype NofXd2;
    fptype NegNofXd1;
    fptype NegNofXd2;

    xStockPrice = sptprice;
    xStrikePrice = strike;
    xRiskFreeRate = rate;
    xVolatility = volatility;

    xTime = time;
    xSqrtTime = sqrt(xTime);

    logValues = log( sptprice / strike );

    xLogTerm = logValues;


    xPowerTerm = xVolatility * xVolatility;
    xPowerTerm = xPowerTerm * 0.5;

    xD1 = xRiskFreeRate + xPowerTerm;
    xD1 = xD1 * xTime;
    xD1 = xD1 + xLogTerm;

    xDen = xVolatility * xSqrtTime;
    xD1 = xD1 / xDen;
    xD2 = xD1 -  xDen;

    d1 = xD1;
    d2 = xD2;

    NofXd1 = CNDF( d1 );
    NofXd2 = CNDF( d2 );

    FutureValueX = strike * ( exp( -(rate)*(time) ) );
    if (otype == 0) {
        OptionPrice = (sptprice * NofXd1) - (FutureValueX * NofXd2);
    } else {
        NegNofXd1 = (1.0 - NofXd1);
        NegNofXd2 = (1.0 - NofXd2);
        OptionPrice = (FutureValueX * NegNofXd2) - (sptprice * NegNofXd1);
    }

    return OptionPrice;
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

int newcount = 0;
void countAdd (){
    newcount++;
}

int bs_thread(void *tid_ptr, bool perf) {
    int i, j;
    fptype price;
    fptype priceDelta;
    int tid = *(int *)tid_ptr;
    int start = tid * (numOptions / nThreads);
    int end = start + (numOptions / nThreads);

    if (perf) {
        #pragma clang loop perforate (enable)
        for (j = 0; j < NUM_RUNS; j++) {
            countAdd();
            for (i = start; i < end; i++) {
                /* Calling main function to calculate option value based on
                 * Black & Scholes's equation.
                 */
                price = BlkSchlsEqEuroNoDiv(sptprice[i], strike[i],
                                            rate[i], volatility[i], otime[i],
                                            otype[i], 0);
                prices[i] = price;

            }
        }
    }
    else {
        for (j = 0; j < NUM_RUNS; j++) {
            countAdd();
            for (i = start; i < end; i++) {
                /* Calling main function to calculate option value based on
                 * Black & Scholes's equation.
                 */
                price = BlkSchlsEqEuroNoDiv(sptprice[i], strike[i],
                                            rate[i], volatility[i], otime[i],
                                            otype[i], 0);
                prices[i] = price;

            }
        }
    }

    return 0;
}

int startBalckScholes (bool perf)
{

    struct timeval stop, start;
    gettimeofday(&start, NULL);

    FILE *file;
    int i;
    int loopnum;
    fptype * buffer;
    int * buffer2;
    int rv;

    //LOGD("PARSEC Benchmark Suite\n");
    fflush(NULL);

    nThreads = 1;
    char *inputFile = "";
    char *outputFile = "";

    //Read input data from file
    /*file = fopen(inputFile, "r");
    if(file == NULL) {
        LOGD("ERROR: Unable to open file `%s'.\n", inputFile);
        exit(1);
    }
    rv = fscanf(file, "%i", &numOptions);
    if(rv != 1) {
        LOGD("ERROR: Unable to read from file `%s'.\n", inputFile);
        fclose(file);
        exit(1);
    }
    if(nThreads > numOptions) {
        LOGD("WARNING: Not enough work, reducing number of threads to match number of options.\n");
        nThreads = numOptions;
    }

    // alloc spaces for the option data
    dataL = (OptionData*)malloc(numOptions*sizeof(OptionData));
    prices = (fptype*)malloc(numOptions*sizeof(fptype));
    for ( loopnum = 0; loopnum < numOptions; ++ loopnum )
    {
        rv = fscanf(file, "%f %f %f %f %f %f %c %f %f", &dataL[loopnum].s, &dataL[loopnum].strike, &dataL[loopnum].r, &dataL[loopnum].divq, &dataL[loopnum].v, &dataL[loopnum].t, &dataL[loopnum].OptionType, &dataL[loopnum].divs, &dataL[loopnum].DGrefval);
        if(rv != 9) {
            LOGD("ERROR: Unable to read from file `%s'.\n", inputFile);
            fclose(file);
            exit(1);
        }
    }
    rv = fclose(file);
    if(rv != 0) {
        LOGD("ERROR: Unable to close file `%s'.\n", inputFile);
        exit(1);
    }*/

    numOptions = 4;

    dataL = (OptionData*)malloc(numOptions*sizeof(OptionData));
    prices = (fptype*)malloc(numOptions*sizeof(fptype));

    dataL[0].s = 42.00;
    dataL[0].s = 42.00;
    dataL[0].strike = 40.00;
    dataL[0].r = 0.1000;
    dataL[0].divq = 0.00;
    dataL[0].v = 0.20;
    dataL[0].t = 0.50;
    dataL[0].OptionType = 'C';
    dataL[0].divs = 0.00;
    dataL[0].DGrefval = 4.759423036851750055;

    dataL[1].s = 42.00;
    dataL[1].strike = 40.00;
    dataL[1].r = 0.1000;
    dataL[1].divq = 0.00;
    dataL[1].v = 0.20;
    dataL[1].t = 0.50;
    dataL[1].OptionType = 'P';
    dataL[1].divs = 0.00;
    dataL[1].DGrefval = 0.808600016880314021;

    dataL[2].s = 100.00;
    dataL[2].strike = 100.00;
    dataL[2].r = 0.0500;
    dataL[2].divq = 0.00;
    dataL[2].v = 0.15;
    dataL[2].t = 1.00;
    dataL[2].OptionType = 'P';
    dataL[2].divs = 0.00;
    dataL[2].DGrefval = 3.714602051381290071;

    dataL[3].s = 100.00;
    dataL[3].strike = 100.00;
    dataL[3].r = 0.0500;
    dataL[3].divq = 0.00;
    dataL[3].v = 0.15;
    dataL[3].t = 1.00;
    dataL[3].OptionType = 'C';
    dataL[3].divs = 0.00;
    dataL[3].DGrefval = 8.591659601309890704;

   // LOGD("Num of Options: %d\n", numOptions);
   // LOGD("Num of Runs: %d\n", NUM_RUNS);

#define PAD 256
#define LINESIZE 64

    buffer = (fptype *) malloc(5 * numOptions * sizeof(fptype) + PAD);
    sptprice = (fptype *) (((unsigned long long)buffer + PAD) & ~(LINESIZE - 1));
    strike = sptprice + numOptions;
    rate = strike + numOptions;
    volatility = rate + numOptions;
    otime = volatility + numOptions;

    buffer2 = (int *) malloc(numOptions * sizeof(fptype) + PAD);
    otype = (int *) (((unsigned long long)buffer2 + PAD) & ~(LINESIZE - 1));

    for (i=0; i<numOptions; i++) {
        otype[i]      = (dataL[i].OptionType == 'P') ? 1 : 0;
        sptprice[i]   = dataL[i].s;
        strike[i]     = dataL[i].strike;
        rate[i]       = dataL[i].r;
        volatility[i] = dataL[i].v;
        otime[i]      = dataL[i].t;
    }

    //LOGD("Size of dataL: %d\n", numOptions * (sizeof(OptionData) + sizeof(int)));

    //serial version
    int tid=0;
    bs_thread(&tid, perf);

   // LOGD("Number of perforated runs: %d\n", newcount);


    //Write prices to output file
   /* file = fopen(outputFile, "w");
    LOGD("1");
    if(file == NULL) {
        LOGD("ERROR: Unable to open file `%s'.\n", outputFile);
        exit(1);
    }
    rv = fprintf(file, "%i\n", numOptions);
    LOGD("2");
    if(rv < 0) {
        LOGD("ERROR: Unable to write to file `%s'.\n", outputFile);
        fclose(file);
        exit(1);
    }
    LOGD("3");
    for(i=0; i<numOptions; i++) {
        rv = fprintf(file, "%.18f\n", prices[i]);
        if(rv < 0) {
            LOGD("ERROR: Unable to write to file `%s'.\n", outputFile);
            fclose(file);
            exit(1);
        }
    }
    rv = fclose(file);
    if(rv != 0) {
        LOGD("ERROR: Unable to close file `%s'.\n", outputFile);
        exit(1);
    }*/

    /*for(i=0; i<numOptions; i++) {
        LOGD("%.18f\n", prices[i]);
    }*/

    free(dataL);
    free(prices);

    free(buffer);
    free(buffer2);

    gettimeofday(&stop, NULL);

    return ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_ndkbinderclient_MainActivity_talkToService(
        JNIEnv* env,
        jobject /* this */,
        jint numberOfRuns)
{

    int perfTime = 0;
    int allRunsPerf = 0;

    for (int i = 0; i<numberOfRuns; i++) {
        perfTime += startBalckScholes(true);
        allRunsPerf += newcount;
        newcount = 0;
    }

    LOGD("end of perf");

    int normTime = 0;
    int allRunsNorm = 0;

    for (int i = 0; i<numberOfRuns; i++) {
        normTime += startBalckScholes(false);
        allRunsNorm += newcount;
        newcount = 0;
    }

    std::string hello = "\nNormal: " + std::to_string(normTime) + " All runs: " + std::to_string(allRunsNorm) + "\nPerforated: " + std::to_string(perfTime) + " All runs: " + std::to_string(allRunsPerf);

    return env->NewStringUTF(hello.c_str());
}
