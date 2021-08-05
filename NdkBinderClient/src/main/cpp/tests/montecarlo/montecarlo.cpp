//
// Created by janez on 27. 07. 21.
//

//This test is adopted from

#include "montecarlo.h"
#include "../../perforation/perforation_lib.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

double myFunction(double x);
double monteCarloEstimateSTD(double lowBound, double upBound, int iterations, bool perf);

//Test's main function
double runMonteCarlo(bool perf)
{

    double lowerBound, upperBound;
    int iterations;

    lowerBound = 0;
    upperBound = 20;

    double mcStats[2]; //position 0 holds the estimate, position 1 holds the STD

    iterations = 128*pow(4,5);

    return monteCarloEstimateSTD(lowerBound, upperBound,iterations, perf);
}


double myFunction(double x)
//Function to integrate
{
    return exp(-1*pow(x-6,4)) + exp(-1*pow(x-14,4));
}

double monteCarloEstimateSTD(double lowBound, double upBound, int iterations, bool perf)
//Function to execute Monte Carlo integration on predefined function, calculates STD
{

    double totalSum = 0;

    volatile int trueIterations = 0;

    int iter = 0;

    if (perf) {
        //Our perforated loop
        #pragma clang loop perforate(enable)
        while (iter < iterations - 1) {

            double randNum = lowBound + (float(rand()) / RAND_MAX) * (upBound - lowBound);

            double functionVal = myFunction(randNum);

            totalSum += functionVal;

            trueIterations++;
            iter++;
        }
    }
    else {
        //Same loop, but not perforated
        while (iter<iterations-1)
        {

            double randNum = lowBound + (float(rand())/RAND_MAX) * (upBound-lowBound);

            double functionVal = myFunction(randNum);

            totalSum += functionVal;

            trueIterations++;
            iter++;
        }
    }

    double estimate = (upBound-lowBound)*totalSum/(trueIterations+1); //For normal solve

    return estimate;
}