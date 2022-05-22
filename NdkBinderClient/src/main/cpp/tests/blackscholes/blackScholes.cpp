//
// Created by janez on 27. 07. 21.
//

//This test is taken from the Parsec Benchmark Suite (https://parsec.cs.princeton.edu/)

#include "blackScholes.h"
#include "../../perforation/perforation_lib.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

using namespace std;

#define fptype float

#define NUM_RUNS 1000

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

void resetNewCount(){
    newcount = 0;
}

int getNewCount(){
    return newcount;
}

double bs_thread(void *tid_ptr, bool perf) {
    int i, j;
    fptype price;
    fptype priceDelta;
    int tid = *(int *)tid_ptr;
    int start = tid * (numOptions / nThreads);
    int end = start + (numOptions / nThreads);

    //Loop we are perforating
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
    //Same loop, but without perforation
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


    double sum_prices = 0;
    for (int i = 0; i<4; i++){
        sum_prices += prices[i];
    }

    return sum_prices;
}

//Test's main function
double startBalckScholes (bool perf)
{

    FILE *file;
    int i;
    int loopnum;
    fptype * buffer;
    int * buffer2;
    int rv;

    //LOGD("PARSEC Benchmark Suite\n");
    fflush(NULL);

    nThreads = 1;

    numOptions = 4;

    dataL = (OptionData*)malloc(numOptions*sizeof(OptionData));
    prices = (fptype*)malloc(numOptions*sizeof(fptype));

    //Input data
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

    //serial version
    int tid=0;
    double result = bs_thread(&tid, perf);

    free(dataL);
    free(prices);

    free(buffer);
    free(buffer2);

    return result;
}