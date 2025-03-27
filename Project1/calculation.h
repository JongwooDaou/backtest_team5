#pragma once
#ifndef BACKTESTING_H
#define BACKTESTING_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAX_STOCKS 20

typedef struct {
    int id;
    int stock_count;
    int stocks[MAX_STOCKS];
    double weights[MAX_STOCKS];
    int frequency;
    int amount;
    struct tm start_date;
    struct tm end_date;
} Portfolio;

int compare_dates(struct tm* date1, struct tm* date2);
double calculateReturn(Portfolio* portfolio);
int validateWeights(Portfolio* p);

#endif