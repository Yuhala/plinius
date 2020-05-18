/*
 * Created on Fri Mar 06 2020
 *
 * Copyright (c) 2020 xxx xxxx, xxxx
 */

#ifndef BENCHTOOLS_H
#define BENCHTOOLS_H

/* includes */
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#include <numeric>

/* For benchmarking */

#define NUM_ITERATIONS (10 + 1) //Default:100 added 1 so as to get middle number for calculating median
#define NUM_POINTS 64           //Default:100 32000 points approx= 64MB...
#define WARM_UP 10              //Default:10
#define SHIFT 1 << 19           //1<<16 //Equals 256KB for 4byte elts eg ints
#define MEDIAN_INDEX (NUM_ITERATIONS / 2)
#define Q1_INDEX (MEDIAN_INDEX / 2)                //starting index for 1st quartile
#define Q3_INDEX (NUM_ITERATIONS - (Q1_INDEX + 1)) //starting index for 3rd quartile


typedef enum {
    MILLI,
    MICRO,
    NANO,
    SEC
} granularity;

void quicksort(std::vector<double> &vect, int left, int right);
double get_median(std::vector<double> &vect);
double get_mean(std::vector<double> &vect);
void swap(double *a, double *b);
double get_SD(std::vector<double> &vect);
double time_diff(timespec *start, timespec *stop, granularity gran);

#endif /* BENCHTOOLS_H */
