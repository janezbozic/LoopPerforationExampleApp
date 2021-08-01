//
// Created by janez on 27. 07. 21.
//

#include "montecarlo.h"
#include "../../perforation/perforation_lib.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

double myFunction(double x);
void monteCarloEstimateSTD(double lowBound, double upBound, int iterations, double mcStats[], bool perf);

double runMonteCarlo(bool perf)
{

    double lowerBound, upperBound;
    int iterations;

    lowerBound = 0;
    upperBound = 20;

    double mcStats[2]; //position 0 holds the estimate, position 1 holds the STD

    iterations = 128*pow(4,5);

    monteCarloEstimateSTD(lowerBound, upperBound,iterations, mcStats, perf);

    return mcStats[1];
}


double myFunction(double x)
//Function to integrate
{
    return exp(-1*pow(x-6,4)) + exp(-1*pow(x-14,4));
}

void monteCarloEstimateSTD(double lowBound, double upBound, int iterations, double statsArray[], bool perf)
//Function to execute Monte Carlo integration on predefined function, calculates STD
{

    double totalSum = 0;
    double totalSumSquared = 0;

    int iter = 0;

    if (perf) {
        #pragma clang loop perforate(enable)
        while (iter < iterations - 1) {

            double randNum = lowBound + (float(rand()) / RAND_MAX) * (upBound - lowBound);

            double functionVal = myFunction(randNum);

            totalSum += functionVal;
            totalSumSquared += pow(functionVal, 2);

            iter++;
        }
    }
    else {
        while (iter<iterations-1)
        {

            double randNum = lowBound + (float(rand())/RAND_MAX) * (upBound-lowBound);

            double functionVal = myFunction(randNum);

            totalSum += functionVal;
            totalSumSquared+= pow(functionVal,2);

            iter++;
        }
    }

    double estimate = (upBound-lowBound)*totalSum/iterations; //For normal solve

    double expected = totalSum/iterations;

    double expectedSquare = totalSumSquared/iterations;

    double std = (upBound-lowBound)*pow( (expectedSquare-pow(expected,2))/(iterations-1) ,0.5);

    statsArray[0] = estimate;
    statsArray[1] = std;

}