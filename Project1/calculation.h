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
    double mdd;  // �ִ� ���� (MDD)
    double total_return;  // �� ���ڱݾ�
    double* monthly_returns;  // ���� ���ͷ� �迭
    double* cum_monthly_returns;  // ���� ���� ���ͷ� �迭
} ReturnResult;

int compare_dates(struct tm* date1, struct tm* date2);
ReturnResult calculateReturn(Portfolio* portfolio);

#endif