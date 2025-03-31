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

typedef struct {
    int closing_price;
    struct tm closing_date;
} StockPrice;

typedef struct {
    double mdd;  // 최대 낙폭 (MDD)
    double total_return;  // 총 투자금액
    double* monthly_returns;  // 월별 수익률 배열
    double* cum_monthly_returns;  // 월별 누적 수익률 배열
} ReturnResult;

int compare_dates(struct tm* date1, struct tm* date2);
ReturnResult calculateReturn(Portfolio* portfolio);

#endif