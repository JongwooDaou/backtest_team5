#pragma once
#ifndef BACKTESTING_H
#define BACKTESTING_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAX_STOCKS 20

typedef enum {
    DAILY, WEEKLY, MONTHLY
} Frequency;

typedef struct {
    int id;
    int stock_count;
    int stocks[MAX_STOCKS];
    double weights[MAX_STOCKS];
    Frequency frequency;
    int amount;
    struct tm start_date;
    struct tm end_date;
} Portfolio;

typedef struct {
    struct tm date;
    double closing_price;  // ����
} StockPrice;

typedef struct {
    int stock_id;
    StockPrice* price_data; // �־��� ������ ���� ������
    int price_data_size; // ���� �������� ����
} Stock;

int compare_dates(struct tm* date1, struct tm* date2);
double get_closing_price(Stock* stock, struct tm* date);
double calculateReturn(Portfolio* portfolio, Stock* stocks);
int validateWeights(Portfolio* p);

#endif